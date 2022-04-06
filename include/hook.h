//
// Created by 20132 on 2022/4/1.
//

#ifndef XZMJX_HOOK_H
#define XZMJX_HOOK_H
#include <unistd.h>
namespace xzmjx{
bool IsHookEnable();
void SetHookEnable(bool hook_flag);
void EnableHook();
}///namespace xzmjx

extern "C"{
///sleep
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t seconds);
extern usleep_fun usleep_f;

typedef int (*nanosleep_fun)(const struct timespec *req, struct timespec *rem);
extern nanosleep_fun nanosleep_f;

///socket
typedef int (*socket_fun)(int domain,int type,int protocol);
extern socket_fun socket_f;

///read
typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
extern read_fun read_f;



///write


///others



}

#endif //XZMJX_HOOK_H
