//
// Created by 20132 on 2022/3/11.
//

#ifndef XZMJX_SINGLETON_H
#define XZMJX_SINGLETON_H
#include <memory>

namespace xzmjx {
template <typename T>
class SingletonPtr {
public:
    static typename T::ptr GetInstance() {
        static typename T::ptr instance(new T);
        return instance;
    }
};
} // namespace xzmjx

#endif // XZMJX_SINGLETON_H
