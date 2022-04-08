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

    ///@details 用户未设置为非阻塞，但实际均为非阻塞，利用hook后加入epoll或定时器来实现非阻塞的效果。
    ssize_t n = fun(fd,std::forward<Args>(args)...);
    while(n == -1&&errno == EINTR){
        n = fun(fd,std::forward<Args>(args)...);
    }
    if(n == -1&&errno == EAGAIN){
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





}




