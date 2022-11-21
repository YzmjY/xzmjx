//
// Created by xzmjx on 22-11-21.
//
#include "sync/fiber_cond.h"
#include "scheduler.h"

namespace xzmjx {
void FiberCondvar::wait(FiberMutex& mutex) {
    m_mutex.lock();
    m_wait_queue.push(std::make_pair(Scheduler::Self(),Fiber::Self()));
    m_mutex.unlock();

    //解锁
    mutex.unlock();

    //让出执行权
    Fiber::YieldToHold();

    //再次被唤醒时，抢锁
    mutex.lock();
}

void FiberCondvar::notify() {
    MutexType::Lock lock(m_mutex);
    if(!m_wait_queue.empty()) {
        auto next_fiber = m_wait_queue.front();
        m_wait_queue.pop();
        next_fiber.first->submit(next_fiber.second);
    } else {
        return;
    }
}

void FiberCondvar::notifyAll() {
    MutexType::Lock lock(m_mutex);
    while(!m_wait_queue.empty()) {
        auto next_fiber = m_wait_queue.front();
        m_wait_queue.pop();
        next_fiber.first->submit(next_fiber.second);
    }
}

}
