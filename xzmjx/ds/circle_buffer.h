//
// Created by xzmjx on 22-11-21.
//

#ifndef XZMJX_DS_CIRCLE_BUFFER_H
#define XZMJX_DS_CIRCLE_BUFFER_H

#include <vector>
#include <memory>

namespace xzmjx {
namespace ds {
template <typename T>
class CircleBuffer {
public:
    using ptr = std::shared_ptr<CircleBuffer>;
    CircleBuffer():m_r_idx(0)
            ,m_w_idx(0)
            ,m_size(0)
            ,m_capacity(0){}
    ~CircleBuffer() = default;

    bool push(const T& v) {
        if(full()) {
            return false;
        }

        m_container[m_w_idx] = v;
        m_w_idx++;
        m_size++;
        return true;
    }

    bool pop(T& v) {
        if(empty()) {
            return false;
        }

        v = m_container[m_r_idx];
        m_r_idx++;
        m_size--;
        return true;
    }

    bool empty() const {
        return m_r_idx == m_w_idx;
    }

    bool full() const {
        return m_size == m_capacity;
    }

    std::size_t size() const {
        return m_size;
    }

    std::size_t capacity() const {
        return m_capacity;
    }

private:
    std::vector<T> m_container;
    std::size_t m_r_idx;
    std::size_t m_w_idx;
    std::size_t m_size;
    std::size_t m_capacity;
};
}
}



#endif //XZMJX_DS_CIRCLE_BUFFER_H
