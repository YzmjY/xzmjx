//
// Created by xzmjx on 22-11-21.
//
#include "sync/fiber_mutex.h"
#include "scheduler.h"
#include "marco.h"

namespace xzmjx {
FiberMutex::FiberMutex() : m_fiber_id(-1) {}

FiberMutex::~FiberMutex() { XZMJX_ASSERT_2(m_wait_queue.empty(), "wait_queue is not empty"); }

bool FiberMutex::tryLock() { return m_lock.tryLock(); }

void FiberMutex::lock() {
    uint64_t id = Fiber::GetFiberId();
    if (id == m_fiber_id) {
        return;
    }

    while (!tryLock()) {
        // 获取锁失败，加入等待队列，让出执行权
        bool getLock = false;
        for (size_t i = 0; i < 4; ++i) {
            if (tryLock()) {
                m_fiber_id = id;
                getLock = true;
                break;
            }
        }
        if (getLock) {
            return;
        }

        m_queue_lock.lock();
        m_wait_queue.push(std::make_pair(Scheduler::Self(), Fiber::Self()));
        m_queue_lock.unlock();
        Fiber::YieldToHold();
    }
}

void FiberMutex::unlock() {
    m_queue_lock.lock();
    if (m_wait_queue.empty()) {
        m_queue_lock.unlock();
        return;
    }
    auto next_fiber = m_wait_queue.front();
    m_wait_queue.pop();
    m_queue_lock.unlock();
    m_lock.unlock();
    next_fiber.first->submit(next_fiber.second);
}
} // namespace xzmjx
