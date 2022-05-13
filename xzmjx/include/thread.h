//
// Created by 20132 on 2022/3/11.
//

#ifndef XZMJX_THREAD_H
#define XZMJX_THREAD_H

#include <memory>
#include <functional>
#include <pthread.h>
#include "mutex.h"

namespace xzmjx{
class Thread:public Noncopyable{
public:
    typedef std::shared_ptr<Thread> ptr;

    /**
     * @brief 构造函数
     * @param cb ：仿函数，可以通过bind绑定参数，从而实现线程可以执行有任何参数的函数体
     * @param name ：线程名称
     */
    Thread(std::function<void()> cb,std::string name);

    /**
     * @brief 析构函数
     */
    ~Thread();

    /**
     * @brief 获取线程id
     * @return
     */
    uint64_t getThreadId() const{
        return m_tid;
    }

    /**
     * @brief 内部实现为使用pthread_join，确保子线程执行完毕
     */
    void join();

    /**
     * @brief 设置线程名称，为一些不是我们创建的线程设置t_thread_name的值。例如main线程
     * @param name
     */
    static void SetName(const std::string& name);

    /**
     * @brief 获取线程名称
     * @return
     */
    static const std::string& GetName();

    /**
     * @brief 获取当前线程对象
     * @return
     */
    static Thread* GetThis();

private:
    /**
     * @brief 线程主函数
     * @param arg
     * @brief 这里设计为静态函数的原因有两个:1.符合pthread_create接口设计，非静态的成员函数有一个隐式的this指针参数.
     *                                 2.static的成员函数可以访问类的私有成员及接口.
     * @return
     */
    static void* run(void* arg);

private:
    std::function<void()> m_cb;  ///线程执行函数
    std::string m_name;          ///线程名称，通过pthread_setname_np使得通过top查看的名称与日志中的名称一致
    uint64_t m_tid;              ///线程id
    pthread_t m_thread;          ///pthread线程结构
    Semaphore m_sem;             ///信号量，用来确保构造函数返回时，该线程一定处于run状态
};
}///namespace xzmjx


#endif //XZMJX_THREAD_H
