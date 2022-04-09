//
// Created by 20132 on 2022/4/1.
//
#include "hook.h"
#include "fiber.h"
#include "log.h"
#include "iomanager.h"
#include "fdmanager.h"
#include <dlfcn.h>

namespace {
    bool thread_local t_enable_hook = false;
    xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
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

struct _HookIniter{
    _HookIniter(){
        hook_init();
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
        errno = EBADF;
        return -1;
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
                t->cancelled - ETIMEDOUT;
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
            if(timer){
                ///@details 未超时，重置超时事件
                timer->cancel();
                if(t_info->cancelled ){
                    errno = t_info->cancelled;
                    return -1;
                }
                goto retry;
            }
        }
    }


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
    int timeout_ms = req->tv_sec*1000+req->tv_nsec/1000/1000;
    iom->addTimerEvent(timeout_ms,[&iom,&fiber](){
        iom->submit(fiber);
    });
    xzmjx::Fiber::YieldToHold();
    return 0;
}

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
    return do_io(sockfd, recvfrom,"recvfrom",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,buf,len,flags,src_addr,addrlen);
}

ssize_t recvmsg(int sockfd,struct msghdr *msg,int flags){
    return do_io(sockfd,recvmsg_f,"recvmsg",xzmjx::IOManager::Event_READ,SO_RCVTIMEO,msg,flags);
}

ssize_t write(int fd,const void *buf,size_t count){
    return do_io(fd,write_f,"write",xzmjx::IOManager::Event_WRITE,SO_RCVTIMEO,buf,count);
}

ssize_t writev(int fd, const struct iovec *iov,int iovcnt){
    return do_io(fd, readv_f,"readv",xzmjx::IOManager::Event_WRITE,SO_RCVTIMEO,iov,iovcnt);
}





}




