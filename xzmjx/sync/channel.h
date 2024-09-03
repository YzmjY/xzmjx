//
// Created by xzmjx on 22-11-21.
//

#ifndef XZMJX_CHANNEL_H
#define XZMJX_CHANNEL_H
#include <memory>
#include "sync/fiber_mutex.h"
#include "sync/fiber_cond.h"
#include "ds/circle_buffer.h"

namespace xzmjx {
template <typename T>
class Channel {
    struct Rep {
        bool m_isClosed;
        uint64_t m_capacity;
        uint64_t m_size;
        ds::CircleBuffer<T> m_data;
        FiberMutex m_mutex;
        FiberCondvar m_empty_cond;
        FiberCondvar m_full_cond;

        using ptr = std::shared_ptr<Rep>;
        Rep() : m_capacity(0), m_size(0) {}
    };

public:
    using ptr = std::shared_ptr<Channel<T>>;

    Channel() = default;
    ~Channel() = default;

    Channel& operator<<(const T& v) {
        this->push(v);
        return *this;
    }

    Channel& operator>>(T& v) {
        this->pop(v);
        return *this;
    }

    bool isClosed() const { return this->m_rep->m_isClosed; }

    uint64_t size() const { return this->m_rep->m_size; }

    uint64_t capacity() const { return this->m_rep->m_capacity; }

    void close() {
        FiberMutex::Lock lock(m_rep->m_mutex);
        if (m_rep->m_isClosed) {
            return;
        }
        m_rep->m_isClosed = true;
        // 唤醒等待的协程
        m_rep->m_empty_cond.notify();
        m_rep->m_full_cond.notify();
    }

private:
    bool tryPush(const T& v) {
        FiberMutex::Lock lock(m_rep->m_mutex);
        if (isClosed()) {
            return false;
        }
        if (!m_rep->m_data.push(v)) {
            return false;
        }

        return true;
    }

    bool tryPop(T& v) {
        FiberMutex::Lock lock(m_rep->m_mutex);
        if (isClosed()) {
            return false;
        }
        if (!m_rep->m_data.pop(v)) {
            return false;
        }

        return true;
    }

    bool push(const T& v) {
        FiberMutex::Lock lock(m_rep->m_mutex);
        while (!m_rep->m_data.push(v)) {
            m_rep->m_empty_cond.wait(m_rep->m_mutex);
            if (isClosed()) {
                return false;
            }
        }

        m_rep->m_full_cond.notify();
        return true;
    }

    bool pop(T& v) {
        FiberMutex::Lock lock(m_rep->m_mutex);
        while (!m_rep->m_data.pop(v)) {
            m_rep->m_full_cond.wait(m_rep->m_mutex);
            if (isClosed()) {
                return false;
            }
        }

        m_rep->m_empty_cond.notify();
        return true;
    }

private:
    typename Channel<T>::Rep::ptr m_rep;
};
} // namespace xzmjx

#endif // XZMJX_CHANNEL_H
