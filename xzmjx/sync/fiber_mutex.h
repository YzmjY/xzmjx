//
// Created by xzmjx on 22-11-20.
//

#ifndef XZMJX_FIBER_MUTEX_H
#define XZMJX_FIBER_MUTEX_H

#include <queue>
#include <map>
#include "fiber.h"
#include "mutex.h"


namespace xzmjx {
class Scheduler;
class FiberMutex{
public:
    using MutexType = Spinlock;
    using Lock = ScopedLockImpl<FiberMutex>;
    FiberMutex();
    ~FiberMutex();

    bool tryLock();
    void lock();
    void unlock();

private:
    std::queue<std::pair<Scheduler*,Fiber::ptr>> m_wait_queue;
    MutexType m_lock;
    MutexType m_queue_lock;
    uint64_t m_fiber_id;
};
}


#endif //XZMJX_FIBER_MUTEX_H
