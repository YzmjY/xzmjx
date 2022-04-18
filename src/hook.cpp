//
// Created by 20132 on 2022/4/1.
//
#include "hook.h"
#include "fiber.h"
#include "log.h"
#include "iomanager.h"
#include "fdmanager.h"
#include "config.h"
#include <dlfcn.h>

namespace {
    bool thread_local t_enable_hook = false;
    xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
    xzmjx::ConfigVar<int>::ptr g_tcp_connect_timeout =
            xzmjx::Config::Lookup("tcp.connect.timeout",5000,"tcp connect timeout");
}
namespace xzmjx{
    bool IsHookEnable(){
        return t_enable_hook;
    }
    void SetHookEnable(bool hook_flag){
        t_enable_hook = hook_flag;
    }
    void EnableHook(){
        t_enable_hook = true;
    }
} ///xzmjx

#define HOOK_FUN(XX) \
    XX(sleep)        \
    XX(usleep)       \
    XX(nanosleep)    \
    XX(socket)       \
    XX(connect)      \
    XX(accept)       \
    XX(read)         \
    XX(readv)        \
    XX(recv)         \
    XX(recvfrom)     \
    XX(recvmsg)      \
    XX(write)        \
    XX(writev)       \
    XX(send)         \
    XX(sendto)       \
    XX(sendmsg)      \
    XX(close)        \
    XX(fcntl)        \
    XX(ioctl)        \
    XX(getsockopt)   \
    XX(setsockopt)


void hook_init(){
    static bool is_inited = false;
    if(is_inited){
        return;
    }
///XX(sleep)--->sleep_f = (sleep_fun)(dlsym(RTLD_NEXT),sleep)
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT,#name);
    HOOK_FUN(XX);
#undef XX
}
static uint64_t s_connect_timeout = -1;
struct _HookIniter{
    _HookIniter(){
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();
        g_tcp_connect_timeout->addListener([](const int& old_val,const int& new_val){
            XZMJX_LOG_INFO(g_logger)<<"tcp connect timeout change from "<<old_val<<" to "<<new_val;
            s_connect_timeout = new_val;
        });
    }
};

/**
 * @brief 利用在main函数之前执行静态变量的初始化特性，完成在进入main之前保存hook的原函数
 */
static _HookIniter s_hook_initer;

struct timer_info{
    int cancelled = 0;
};

template<typename OriginFun,typename... Args>
static ssize_t do_io(int fd,OriginFun fun,const char* hook_fun_name,
                     uint32_t event,int timeout_so,Args&&... args){
    if(!xzmjx::IsHookEnable()){
        return fun(fd,std::forward<Args>(args)...);
    }
    xzmjx::FdCtx::ptr fd_ctx = xzmjx::FdMgr::GetInstance()->get(fd);
    if(!fd_ctx){
        return fun(fd,std::forward<Args>(args)...);
    }

    if(fd_ctx->isClose()){
        errno = EBADF;
        return -1;
    }

    if(!fd_ctx->isSocket()||fd_ctx->getUserNonblock()){
        ///@details 用户设置了非阻塞，则用户需要自己为可能发生的读不全结果负责，不hook
        return fun(fd,std::forward<Args>(args)...);
    }

    uint64_t to = fd_ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> t_info = std::make_shared<timer_info>();

retry:
    ///@details 用户未设置为非阻塞，但实际均为非阻塞，利用hook后加入epoll或定时器来实现非阻塞的效果。
    ssize_t n = fun(fd,std::forward<Args>(args)...);
    while(n == -1&&errno == EINTR){
        n = fun(fd,std::forward<Args>(args)...);
    }

    if(n == -1&&errno == EAGAIN){
        ///非阻塞下，由于无数据可读或可写返回，加入观察事件，等待相应事件发生。有超时机制要加入定时器。
        xzmjx::IOManager* iom = xzmjx::IOManager::Self();
        std::weak_ptr<timer_info> weak_cond(t_info);
        xzmjx::Timer::ptr timer = nullptr;
        if(to != (uint64_t)-1){
            ///@details 设置有超时时间
            timer = iom->addCondTimerEvent(to,[weak_cond,fd,iom,event](){
                auto t = weak_cond.lock();
                if(!t||t->cancelled==ETIMEDOUT){
                    return;
                }
                t->cancelled = ETIMEDOUT;
                iom->cancelEvent(fd,(xzmjx::IOManager::Event)event);
            },weak_cond);
        }
        ///@details 挂起当前协程，放在epoll中监听事件
        int rt = iom->addEvent(fd,(xzmjx::IOManager::Event)event);
        if(rt){
            XZMJX_LOG_ERROR(g_logger)<<hook_fun_name<<" addEvent("<<fd<<" ,"<<event<<")";
            if(timer){
                timer->cancel();
            }
            return -1;
        }else{
            xzmjx::Fiber::YieldToHold();
            if(timer) {
                ///@details 未超时，重置超时事件
                timer->cancel();
            }
            if(t_info->cancelled ){
                errno = t_info->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}

extern "C"{
///初始化XX(sleep)--->sleep_f = nullptr
#define XX(name) name ## _fun name ## _f = nullptr;
    HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds){
    if(!xzmjx::IsHookEnable()){
        return sleep_f(seconds);
    }
    xzmjx::Fiber::ptr fiber = xzmjx::Fiber::Self();
    xzmjx::IOManager* iom = xzmjx::IOManager::Self();
    iom->addTimerEvent(seconds*1000,[&iom,&fiber](){
       iom->submit(fiber);
    });
    xzmjx::Fiber::YieldToHold();
    return 0;
}

int usleep(useconds_t seconds){
    if(!xzmjx::IsHookEnable()){
        return usleep_f(seconds);
    }
    xzmjx::Fiber::ptr fiber = xzmjx::Fiber::Self();
    xzmjx::IOManager* iom = xzmjx::IOManager::Self();
    iom->addTimerEvent(seconds*1000,[&iom,&fiber](){
        iom->submit(fiber);
    });
    xzmjx::Fiber::YieldToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem){
    if(!xzmjx::IsHookEnable()){
        return nanosleep_f(req,rem);
    }
    xzmjx::Fiber::ptr fiber = xzmjx::Fiber::Self();
    xzmjx::IOManager* iom = xzmjx::IOManager::Self();
    uint64_t timeout_ms = req->tv_sec*1000+req->tv_nsec/1000/1000;
    iom->addTimerEvent(timeout_ms,[&iom,&fiber](){
        iom->submit(fiber);
    });
    xzmjx::Fiber::YieldToHold();
    return 0;
}

///@details read
ssize_t read(int fd,void* buf,size_t count){
    return do_io(fd,read_f,"read",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,buf,count);
}

ssize_t  readv(int fd,const struct iovec *iov,int iovcnt){
    return do_io(fd, readv_f,"readv",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,iov,iovcnt);
}

ssize_t recv(int sockfd,void *buf,size_t len,int flags){
    return do_io(sockfd, recv_f,"recv",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,buf,len,flags);
}

ssize_t recvfrom(int sockfd,void *buf,size_t len,int flags,struct sockaddr *src_addr,socklen_t *addrlen){
    return do_io(sockfd, recvfrom_f,"recvfrom",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,buf,len,flags,src_addr,addrlen);
}

ssize_t recvmsg(int sockfd,struct msghdr *msg,int flags){
    return do_io(sockfd,recvmsg_f,"recvmsg",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,msg,flags);
}

///@details write
ssize_t write(int fd,const void *buf,size_t count){
    return do_io(fd,write_f,"write",xzmjx::IOManager::Event_WRITE,SO_SNDTIMEO,buf,count);
}

ssize_t writev(int fd, const struct iovec *iov,int iovcnt){
    return do_io(fd, readv_f,"readv",xzmjx::IOManager::Event_WRITE,SO_SNDTIMEO,iov,iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags){
    return do_io(s,send_f,"send",xzmjx::IOManager::Event_WRITE,SO_SNDTIMEO,msg,len,flags);

}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen){
    return do_io(s,sendto_f,"sendto",xzmjx::IOManager::Event_WRITE,SO_SNDTIMEO,msg,len,flags,to,tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags){
    return do_io(s,sendmsg_f,"sendmsg",xzmjx::IOManager::Event_WRITE,SO_SNDTIMEO,msg,flags);
}

int close(int fd){
    if(!xzmjx::IsHookEnable()){
        return close_f(fd);
    }
    xzmjx::FdCtx::ptr ctx = xzmjx::FdMgr::GetInstance()->get(fd);
    if(ctx){
        auto iom = xzmjx::IOManager::Self();
        if(iom){
            iom->cancelAll(fd);
        }
        xzmjx::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}

///@details others
int fcntl(int fd, int cmd, ... /* arg */ ){
    va_list va;
    va_start(va, cmd);
    switch(cmd) {
        case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            xzmjx::FdCtx::ptr ctx = xzmjx::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                return fcntl_f(fd, cmd, arg);
            }
            ctx->setUserNonblock(arg & O_NONBLOCK);
            if(ctx->getSysNonblock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
            break;
        case F_GETFL:
        {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            xzmjx::FdCtx::ptr ctx = xzmjx::FdMgr::GetInstance()->get(fd);
            if(!ctx || ctx->isClose() || !ctx->isSocket()) {
                return arg;
            }
            if(ctx->getUserNonblock()) {
                return arg | O_NONBLOCK;
            } else {
                return arg & ~O_NONBLOCK;
            }
        }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
            case F_SETPIPE_SZ:
#endif
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
            case F_GETPIPE_SZ:
#endif
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
        {
            struct flock* arg = va_arg(va, struct flock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX:
        {
            struct f_owner_exlock* arg = va_arg(va, struct f_owner_exlock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...){
    va_list va;
    va_start(va,request);
    void* arg = va_arg(va,void*);
    va_end(va);

    if(request == FIONBIO){
        bool user_nonblock = !!(*(int*)arg);
        xzmjx::FdCtx::ptr ctx = xzmjx::FdMgr::GetInstance()->get(d);
        if(!ctx||ctx->isClose()||!ctx->isSocket()){
            return ioctl_f(d,request,arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d,request,arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen){
    return getsockopt_f(sockfd,level,optname,optval,optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen){
    if(!xzmjx::IsHookEnable()){
        return setsockopt_f(sockfd,level,optname,optval,optlen);
    }
    if(level == SOL_SOCKET){
        if(optname == SO_RCVTIMEO||optname == SO_SNDTIMEO){
            xzmjx::FdCtx::ptr ctx = xzmjx::FdMgr::GetInstance()->get(sockfd);
            if(ctx){
                const timeval* v = (const timeval*)optval;
                ctx->setTimeout(optname,v->tv_sec*1000+v->tv_usec/1000);
            }
        }
    }
    return setsockopt_f(sockfd,level,optname,optval,optlen);
}
///@details socket
int connect_with_timeout(int fd, const struct sockaddr* addr, socklen_t addrlen, uint64_t timeout_ms){
    if(!xzmjx::IsHookEnable()){
        return connect_f(fd,addr,addrlen);
    }
    xzmjx::FdCtx::ptr ctx = xzmjx::FdMgr::GetInstance()->get(fd);
    if(!ctx||ctx->isClose()){
        errno = EBADF;
        return -1;
    }
    if(!ctx->isSocket()){
        return connect_f(fd,addr,addrlen);
    }

    if(ctx->getUserNonblock()){
        return connect_f(fd,addr,addrlen);
    }

    int rt = connect_f(fd,addr,addrlen);
    if(rt == 0){
        return 0;
    }else if(rt != -1||errno != EINPROGRESS){
        return rt;
    }

    ///@details 设置超时定时器，yield该协程
    xzmjx::IOManager* iom = xzmjx::IOManager::Self();
    xzmjx::Timer::ptr timer;
    std::shared_ptr<timer_info> t_info = std::make_shared<timer_info>();
    std::weak_ptr<timer_info> winfo(t_info);

    if(timeout_ms != (uint64_t)-1){
        timer = iom->addCondTimerEvent(timeout_ms,[winfo,fd,iom](){
            auto ptr = winfo.lock();
            if(!ptr||ptr->cancelled){
                return;
            }
            ptr->cancelled = ETIMEDOUT;

            ///@details 如果超时，这里会触发一次事件，结果是该协程从yield点后返回
            iom->cancelEvent(fd,xzmjx::IOManager::Event_WRITE);
        },winfo);
    }

    ///@details 添加监听事件，等待超时或者connect成功，超时会由于cancelEvent触发写事件将fiber加入调度
    ///Connect返回会由于epoll触发可写，epoll_wait返回，fiber加入调度，在yield点后返回
    rt = iom->addEvent(fd,xzmjx::IOManager::Event_WRITE);
    if(rt == 0){
        xzmjx::Fiber::YieldToHold();
        if(timer){
            timer->cancel();
        }
        if(t_info->cancelled){
            ///@details 由于超时返回
            errno = t_info->cancelled;
            return -1;
        }
    }else{
        if(timer){
            timer->cancel();
        }
        XZMJX_LOG_ERROR(g_logger)<<"connect addEvent("<<fd<<",WRITE) error";
    }

    int error = 0;
    socklen_t len = sizeof(int);
    if(-1 == getsockopt(fd,SOL_SOCKET,SO_ERROR,&error,&len)){
        return -1;
    }
    if(!error){
        return 0;
    }else{
        errno = error;
        return -1;
    }
}

int socket(int domain,int type,int protocol){
    if(!xzmjx::IsHookEnable()){
        return socket_f(domain,type,protocol);
    }
    int fd = socket_f(domain,type, protocol);
    if(fd == -1){
        return fd;
    }
    xzmjx::FdMgr::GetInstance()->get(fd,true);
    return fd;
}

int connect(int sockfd,const struct sockaddr *addr,socklen_t addrlen){
    return connect_with_timeout(sockfd,addr,addrlen,s_connect_timeout);
}

int accept(int sockfd,struct sockaddr *addr,socklen_t *addrlen){
    int fd = do_io(sockfd, accept_f,"accept",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,addr,addrlen);
    if(fd>=0&&xzmjx::IsHookEnable()){
        xzmjx::FdMgr::GetInstance()->get(fd,true);
    }
    return fd;
}
}




