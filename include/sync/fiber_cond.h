//
// Created by xzmjx on 22-11-21.
//

#ifndef XZMJX_FIBER_COND_H
#define XZMJX_FIBER_COND_H
#include "fiber_mutex.h"

namespace xzmjx {
class FiberCondvar {
public:
    using MutexType = Spinlock;

    void wait(FiberMutex& mutex);
    void notify();
    void notifyAll();

private:
    MutexType m_mutex;
    std::queue<std::pair<Scheduler*,Fiber::ptr>> m_wait_queue;
};
}

#endif //XZMJX_FIBER_COND_H
