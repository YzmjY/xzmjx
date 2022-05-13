//
// Created by 20132 on 2022/3/15.
//

#ifndef XZMJX_NONCOPYABLE_H
#define XZMJX_NONCOPYABLE_H

namespace xzmjx{
class Noncopyable {
public:
    Noncopyable() = default;
    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};
}///namespace xzmjx

#endif //XZMJX_NONCOPYABLE_H
