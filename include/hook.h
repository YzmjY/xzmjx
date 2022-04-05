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
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t seconds);
extern usleep_fun usleep_f;

}

#endif //XZMJX_HOOK_H
