//
// Created by 20132 on 2022/3/24.
//
#include "iomanager.h"
#include "log.h"
#include "marco.h"
#include <sys/epoll.h>
#include <cstring>
#include <fcntl.h>

namespace xzmjx{
    static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
    enum EpollCtlOp{} ;

    static std::ostream &operator <<(std::ostream& os,const EpollCtlOp& op){
        switch(op){
            case EPOLL_CTL_ADD:
                return os<<"EPOLL_CTL_ADD";
            case EPOLL_CTL_DEL:
                return os<<"EPOLL_CTL_DEL";
            case EPOLL_CTL_MOD:
                return os<<"EPOLL_CTL_MOD";
            default:
                return os<<static_cast<int>(op);
        }
    }

    IOManager::FdContext::EventContext &IOManager::FdContext::getContext(Event event) {
        XZMJX_ASSERT(event == Event_READ||event == Event_WRITE);
        if(event == Event_READ){
            return read;
        }else{
            return write;
        }
    }
    void IOManager::FdContext::resetContext(EventContext& ctx) {
        ctx.scheduler = nullptr;
        ctx.fiber.reset();
        ctx.cb = nullptr;
    }

    void IOManager::FdContext::triggerContext(Event event) {
        XZMJX_ASSERT(event&events);
        events = static_cast<Event>(events&(~event));
        EventContext ctx = getContext(event);
        if(ctx.fiber){
            ctx.scheduler->submit(std::move(ctx.fiber));
        }else{
            ctx.scheduler->submit(std::move(ctx.cb));
        }
        ctx.scheduler = nullptr;
    }

    IOManager::IOManager(size_t thrNum,const std::string& name)
                : Scheduler(thrNum,name){
        XZMJX_LOG_DEBUG(g_logger)<<"IOManager::IOManager()";
        m_time_manager.reset(new TimerManager());
        m_time_manager->setInsertAtFrontCb(std::bind(&IOManager::notify, this));
        m_epoll_fd = epoll_create(1);
        XZMJX_ASSERT(m_epoll_fd != 0);

        int rt = pipe(m_tickle_fds);
        XZMJX_ASSERT(rt == 0);

        epoll_event event;
        memset(&event,0,sizeof event);
        event.events = EPOLLET|EPOLLIN;
        event.data.fd = m_tickle_fds[0];

        rt = fcntl(m_tickle_fds[0], F_SETFL, O_NONBLOCK);
        XZMJX_ASSERT(rt == 0);

        rt =epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_tickle_fds[0], &event);
        XZMJX_ASSERT(rt == 0);

        contextResize(32);
        start();
    }

    IOManager::~IOManager(){
        stop();
        close(m_epoll_fd);
        close(m_tickle_fds[0]);
        close(m_tickle_fds[1]);
        for(size_t i = 0; i < m_fd_contexts.size(); i++){
            if(m_fd_contexts[i]){
                delete m_fd_contexts[i];
            }
        }
    }

    int IOManager::addEvent(int fd,Event event,std::function<void()> cb){
        FdContext* fd_ctx = nullptr;
        MutexType::ReadLock lock(m_mutex);
        if(static_cast<int>(m_fd_contexts.size()) <= fd){
            ///需要扩容，按照1.5倍大小扩容
            lock.unlock();
            MutexType::WriteLock lock1(m_mutex);
            if(static_cast<int>(m_fd_contexts.size()) <= fd){
                contextResize(fd*1.5);
            }
            fd_ctx = m_fd_contexts[fd];
        }else{
            fd_ctx = m_fd_contexts[fd];
        }
        FdContext::MutexType::Lock lock1(fd_ctx->m_mutex);
        if(fd_ctx->events&event){
            XZMJX_LOG_ERROR(g_logger) << "addEvent assert fd=" << fd
                                      << " event=" << static_cast<EPOLL_EVENTS>(event)
                                      << " fd_ctx.event=" << static_cast<EPOLL_EVENTS>(fd_ctx->events);
            XZMJX_ASSERT(!(fd_ctx->events&event));
        }
        int op = fd_ctx->events?EPOLL_CTL_MOD:EPOLL_CTL_ADD;
        epoll_event epevent;
        epevent.events = EPOLLET|fd_ctx->events|event;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epoll_fd, op, fd, &epevent);
        if(rt){
            XZMJX_LOG_ERROR(g_logger) << "epoll_ctl" << "(" << m_epoll_fd << ","
                                      << static_cast<EpollCtlOp>(op) << ","
                                      << fd << static_cast<EPOLL_EVENTS>(epevent.events)
                                      << "):ret = " << rt << " (" << errno << ") ("
                                      << strerror(errno) << ") fd_ctx->events="
                                      << (EPOLL_EVENTS)fd_ctx->events;

            return -1;
        }

        ++m_pending_event_count;
        fd_ctx->events = static_cast<Event>(fd_ctx->events|event);
        FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
        XZMJX_ASSERT(!event_ctx.scheduler&&
                     !event_ctx.fiber&&
                     !event_ctx.cb);
        event_ctx.scheduler = Scheduler::Self();
        if(cb){
            event_ctx.cb = std::move(cb);
        }else{
            event_ctx.fiber = Fiber::Self();
        }
        return 0;
    }

    bool IOManager::delEvent(int fd,Event event){
        MutexType::ReadLock lock(m_mutex);
        if(fd>=static_cast<int>(m_fd_contexts.size())){
            return false;
        }
        FdContext* fd_ctx = m_fd_contexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fd_ctx->m_mutex);
        if(!(fd_ctx->events&event)){
            return false;
        }

        Event new_events = static_cast<Event>(fd_ctx->events&~event);
        int op = new_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET|new_events;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epoll_fd, op, fd, &epevent);
        if(rt){
            XZMJX_LOG_ERROR(g_logger) << "epoll_ctl" << "(" << m_epoll_fd << ","
                                      << static_cast<EpollCtlOp>(op) << ","
                                      << fd << static_cast<EPOLL_EVENTS>(epevent.events)
                                      << "):ret = " << rt << " (" << errno << ") ("
                                      << strerror(errno) << ") fd_ctx->events="
                                      << (EPOLL_EVENTS)fd_ctx->events;
            return false;
        }
        --m_pending_event_count;
        fd_ctx->events = new_events;
        FdContext::EventContext& event_ctx = fd_ctx->getContext(event);
        fd_ctx->resetContext(event_ctx);///重置对应fd的对应事件上下文
        return true;
    }

    bool IOManager::cancelEvent(int fd,Event event){
        MutexType::ReadLock lock(m_mutex);
        if(fd>=static_cast<int>(m_fd_contexts.size())){
            return false;
        }
        FdContext* fd_ctx = m_fd_contexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fd_ctx->m_mutex);
        if(!(fd_ctx->events&event)){
            return false;
        }

        Event new_events = static_cast<Event>(fd_ctx->events&~event);
        int op = new_events?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = EPOLLET|new_events;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epoll_fd, op, fd, &epevent);
        if(rt){
            XZMJX_LOG_ERROR(g_logger) << "epoll_ctl" << "(" << m_epoll_fd << ","
                                      << static_cast<EpollCtlOp>(op) << ","
                                      << fd << static_cast<EPOLL_EVENTS>(epevent.events)
                                      << "):ret = " << rt << " (" << errno << ") ("
                                      << strerror(errno) << ") fd_ctx->events="
                                      << (EPOLL_EVENTS)fd_ctx->events;
            return false;
        }
        fd_ctx->triggerContext(event);
        --m_pending_event_count;
        return true;
    }

    bool IOManager::cancelAll(int fd){
        MutexType::ReadLock lock(m_mutex);
        if(fd>=static_cast<int>(m_fd_contexts.size())){
            return false;
        }
        FdContext* fd_ctx = m_fd_contexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock1(fd_ctx->m_mutex);
        if(!(fd_ctx->events)){
            return false;
        }

        int op = EPOLL_CTL_DEL;
        epoll_event epevent;
        epevent.events = 0;
        epevent.data.ptr = fd_ctx;

        int rt = epoll_ctl(m_epoll_fd, op, fd, &epevent);
        if(rt){
            XZMJX_LOG_ERROR(g_logger) << "epoll_ctl" << "(" << m_epoll_fd << ","
                                      << static_cast<EpollCtlOp>(op) << ","
                                      << fd << static_cast<EPOLL_EVENTS>(epevent.events)
                                      << "):ret = " << rt << " (" << errno << ") ("
                                      << strerror(errno) << ") fd_ctx->events="
                                      << (EPOLL_EVENTS)fd_ctx->events;
            return false;
        }
        if(fd_ctx->events&Event_READ){
            fd_ctx->triggerContext(Event_READ);
        }else if(fd_ctx->events&Event_WRITE){
            fd_ctx->triggerContext(Event_WRITE);
        }
        --m_pending_event_count;
        return true;
    }

    Timer::ptr IOManager::addTimerEvent(uint64_t ms,std::function<void()> cb,bool recurring){
        return m_time_manager->addTimer(ms, std::move(cb), recurring);
    }
    Timer::ptr IOManager::addCondTimerEvent(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> cond,bool recurring){
        return m_time_manager->addCondTimer(ms, std::move(cb), cond, recurring);
    }


    IOManager* IOManager::Self(){
        return dynamic_cast<IOManager*>(Scheduler::Self());
    }

    void IOManager::notify() {
        if(!hasIdleThreads()){
            XZMJX_LOG_INFO(g_logger)<<"Scheduler has no idle threads";
            return;
        }
        XZMJX_LOG_INFO(g_logger)<<"notify";
        ///写端写入一个字节唤醒处于epoll_wait的调度线程
        int rt = write(m_tickle_fds[1], "T", 1);
        XZMJX_ASSERT(rt == 1);
    }

    void IOManager::wait(){
        XZMJX_LOG_DEBUG(g_logger)<<"wait";
        const uint64_t MAX_EVENTS = 256;
        epoll_event* events = new  epoll_event[MAX_EVENTS];
        ///自动管理生命周期
        std::shared_ptr<epoll_event> sharedEvents(events,[](epoll_event* ptr){
            delete[] ptr;});
        while(true){
            uint64_t next_timer = 0;
            ///wait协程退出时机是调度器stop时
            if(canStop(next_timer)){
                XZMJX_LOG_INFO(g_logger)<<"name = "<<getName()<<"wait stopping exit";
                break;
            }
            int rt = 0;
            do{
                static const uint64_t MAX_TIME_SLOT = 3000;///3s为一个最大时间间隔
                if(next_timer != ~0ull){
                    next_timer = std::min(MAX_TIME_SLOT,next_timer);
                }else{
                    next_timer = MAX_TIME_SLOT;
                }

                rt =  epoll_wait(m_epoll_fd, events, MAX_EVENTS, next_timer);
                if(rt<0&&errno == EINTR) {
                    continue;
                }else {
                    break;
                }

            }while(true);
            std::vector<std::function<void()>> timer_cbs;
            m_time_manager->listExpiredCb(timer_cbs);
            XZMJX_LOG_DEBUG(g_logger)<<"Expire Timer Count = "<<timer_cbs.size();
            submit(timer_cbs.begin(),timer_cbs.end());

            for(int i = 0; i<rt;++i){
                epoll_event& event = events[i];
                if(event.data.fd == m_tickle_fds[0]){
                    ///任务队列有任务待取,消息只起到notify作用，无实际意义
                    char dummy[256];
                    while(read(m_tickle_fds[0], dummy, sizeof dummy) > 0);
                    XZMJX_LOG_INFO(g_logger)<<"notify receive";
                    continue;
                }
                FdContext* fd_ctx = static_cast<FdContext*>(event.data.ptr);
                FdContext::MutexType::Lock lock(fd_ctx->m_mutex);

                ///对epoll的事件进行归类，对于错误和
                if(event.events&(EPOLLERR|EPOLLHUP)){
                    event.events|=(EPOLLIN|EPOLLOUT)&fd_ctx->events;
                }

                ///重置关注的event,此处采用移除已唤醒的事件，
                /// 如果后续还需要关注这个事件，则需要处理完成后再次添加对应事件
                int real_event = Event_NONE;
                if(event.events&EPOLLIN){
                    real_event|=Event_READ;
                }
                if(event.events&EPOLLOUT){
                    real_event|=Event_WRITE;
                }
                if((fd_ctx->events&real_event) == Event_NONE){
                    continue;
                }

                ///剩下的event
                int left_event = fd_ctx->events&~real_event;
                int op = left_event?EPOLL_CTL_MOD:EPOLL_CTL_DEL;
                event.events = left_event|EPOLLET;
                int rt2 = epoll_ctl(m_epoll_fd, op, fd_ctx->fd, &event);
                if(rt2){
                    XZMJX_LOG_ERROR(g_logger) << "epoll_ctl" << "(" << m_epoll_fd << ","
                                              << static_cast<EpollCtlOp>(op) << ","
                                              << fd_ctx->fd << "," << static_cast<EPOLL_EVENTS>(event.events)
                                              << "):ret = " << rt << " (" << errno << ") ("
                                              << strerror(errno) << ") fd_ctx->events="
                                              << (EPOLL_EVENTS)fd_ctx->events;
                    continue;
                }

                if(real_event&Event_READ){
                    fd_ctx->triggerContext(Event_READ);
                    --m_pending_event_count;
                }

                if(real_event&Event_WRITE){
                    fd_ctx->triggerContext(Event_WRITE);
                    --m_pending_event_count;
                }
            }
            Fiber::YieldToHold();
        }
    }
    bool IOManager::canStop() {
        uint64_t next_timer = 0;
        return canStop(next_timer);
    }
    bool IOManager::canStop(uint64_t& next_timer){
        next_timer = m_time_manager->getNextTimerInterval();
        return next_timer == ~0ull &&
               m_pending_event_count == 0 &&
               Scheduler::canStop();
    }

    void IOManager::contextResize(size_t size){
        m_fd_contexts.resize(size);
        for(size_t  i = 0; i<size;i++){
            if(m_fd_contexts[i] == nullptr){
                m_fd_contexts[i] = new FdContext();
                m_fd_contexts[i]->fd = i;
            }
        }
    }
}
