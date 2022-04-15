//
// Created by 20132 on 2022/3/24.
//

#ifndef XZMJX_IOMANAGER_H
#define XZMJX_IOMANAGER_H
#include <scheduler.h>
#include "timer.h"
namespace xzmjx{
class IOManager:public Scheduler{
public:
    typedef std::shared_ptr<IOManager> ptr;
    typedef RWMutex MutexType;

    enum Event{
        Event_NONE = 0x0,
        Event_READ = 0x1,
        Event_WRITE = 0x4
    };
private:
    /**
     * @brief 每个fd绑定一个FdContex来获取其上下文
     */
    struct FdContext{
        typedef Mutex MutexType;

        struct EventContext{
            Scheduler* scheduler = nullptr;
            Fiber::ptr fiber;
            std::function<void()> cb;
        };

        EventContext& getContext(Event event);
        void resetContext(EventContext& ctx);
        void triggerContext(Event event);

        EventContext read;
        EventContext write;
        int fd = 0;
        Event events;
        MutexType m_mutex;
    };

public:
    explicit IOManager(size_t thrNum = 4,const std::string& name = "Scheduler");
    ~IOManager() override;

    int addEvent(int fd,Event event,std::function<void()> cb = nullptr);
    bool delEvent(int fd,Event event);
    bool cancelEvent(int fd,Event event);
    bool cancelAll(int fd);

    Timer::ptr addTimerEvent(uint64_t ms,std::function<void()> cb,bool recurring = false);

    Timer::ptr addCondTimerEvent(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> cond,bool recurring = false);

    TimerManager::ptr timerManager() const{
        return m_time_manager;
    }
    static IOManager* Self();

protected:
    void notify() override;
    void wait() override;
    bool canStop() override;
    bool canStop(uint64_t& next_timer);

    void contextResize(size_t size);

private:
    std::vector<FdContext*> m_fd_contexts;      ///所有fd的上下文，使用fd作为下标寻址
    int m_epoll_fd = 0;                         ///epoll文件描述符
    int m_tickle_fds[2];                        ///管道的读写两端，用来notify等在epoll_wait的调度器线程，具体来说，wait协程会监视管道的读端
    std::atomic<size_t> m_pending_event_count = {0};///当前待执行的事件数
    RWMutex m_mutex;
    TimerManager::ptr m_time_manager;
};
}///namespace xzmjx

#endif //XZMJX_IOMANAGER_H
