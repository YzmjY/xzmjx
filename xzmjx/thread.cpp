//
// Created by 20132 on 2022/3/11.
//
#include "thread.h"
#include "log.h"

namespace xzmjx {
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
static thread_local std::string t_threadName = "UNKNOWN";
static thread_local Thread* t_thread = nullptr;

Thread::Thread(std::function<void()> cb, std::string name) : m_cb(move(cb)), m_name(move(name)), m_tid(0), m_thread(0) {
    if (m_name.empty()) {
        m_name = "UNKOWN";
    }
    int ret = pthread_create(&m_thread, NULL, Thread::run, this);
    if (ret != 0) {
        XZMJX_LOG_ERROR(g_logger) << "pthread_create failed,error = " << ret << " thread name = " << m_name;
        throw std::logic_error("pthread_create error");
    }
    m_sem.wait();
}
Thread::~Thread() {
    /// 默认析构函数不出错，不抛异常
    if (m_thread) {
        pthread_detach(m_thread);
    }
}

void Thread::join() {
    if (m_thread) {
        int ret = pthread_join(m_thread, nullptr);
        if (ret != 0) {
            XZMJX_LOG_ERROR(g_logger) << "pthread_join failed,error = " << ret << " thread name = " << m_name;
            throw std::logic_error("pthread_join error");
        }
        m_thread = 0;
    }
}

void Thread::SetName(const std::string& name) {
    if (name.empty()) {
        return;
    }
    t_threadName = name;
    if (t_thread) {
        t_thread->m_name = name;
    }
}

const std::string& Thread::GetName() { return t_threadName; }

Thread* Thread::GetThis() { return t_thread; }

void* Thread::run(void* arg) {
    Thread* thread = static_cast<Thread*>(arg);
    t_threadName = thread->m_name;
    t_thread = thread;
    thread->m_tid = GetThreadID();
    pthread_setname_np(pthread_self(), GetName().substr(0, 15).c_str());

    /// 假设m_cb这个函数对象拥有一个智能指针，则该智能指针的对象的生命周期不会短于
    /// m_cb（m_cb拥有其一份拷贝）,如此swap之后，m_cb不再拥有其原资源的计数，而
    /// run函数结束后，cb的生命周期也结束，其可能拥有的资源技术也会对应减一
    std::function<void()> cb;
    cb.swap(thread->m_cb); /// 这里为什么要swap，说是便于释放m_cb的资源

    thread->m_sem.notify();
    cb();
    return nullptr;
}
} // namespace xzmjx
