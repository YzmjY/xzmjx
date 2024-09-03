//
// Created by 20132 on 2022/3/21.
//

#ifndef XZMJX_SCHEDULER_H
#define XZMJX_SCHEDULER_H
#include <memory>
#include <vector>
#include <list>
#include "fiber.h"
#include "thread.h"
#include <atomic>

namespace xzmjx {
/**
 * @brief
 * 简化了sylar的调度器，不使用创建线程进行调度，只使用调度器内部的线程池，方便编码和理解
 */
class Scheduler : std::enable_shared_from_this<Scheduler> {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    /**
     * @brief 构造函数，确定线程池大小，
     * @param thrNum
     * @param name
     */
    explicit Scheduler(size_t thrNum = 4, const std::string& name = "Scheduler");
    virtual ~Scheduler();

    /**
     * @brief 启动调度器，内部创建调度线程，并绑定调度器
     */
    void start();

    /**
     * @brief 停止调度，当无任务可调度时才会成功停止
     */
    void stop();

    /**
     * @brief 通知调度器有任务需要调度
     */
    virtual void notify();

    static Scheduler* Self();

    const std::string& getName() const { return m_name; }

    /**
     * @brief 提交一个任务到调度器
     * @tparam FiberOrCb
     * @param fc
     * @param thrNum
     */
    template <class FiberOrCb>
    void submit(FiberOrCb&& fc, int64_t thrNum = -1) {
        bool needNotify = false;
        {
            MutexType::Lock lock(m_mutex);
            needNotify = submitNoLock(std::forward<FiberOrCb>(fc), thrNum);
        }
        if (needNotify) {
            notify();
        }
    }

    /**
     * @brief 提交一组任务掉调度器
     * @tparam InputIterator
     * @param begin
     * @param end
     */
    template <class InputIterator>
    void submit(InputIterator begin, InputIterator end) {
        bool needNotify = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                needNotify = needNotify || submitNoLock(std::move(*begin), -1);
                begin++;
            }
        }
        if (needNotify) {
            notify();
        }
    }

private:
    /**
     * @brief 协程任务结构，协程任务可以定义为一个封装好的协程或者一个执行体。
     */
    struct Task {
        Fiber::ptr fiber;
        std::function<void()> taskCb;
        int64_t threadId;
        /**
         * @brief 普通构造函数，传入协程构造
         * @param task
         * @param thread
         */
        Task(Fiber::ptr& task, int64_t thread) : threadId(thread) { fiber = task; }

        /**
         * @brief
         * 移动构造，外部传入的task引用计数转移到Task的fiber中，传入协程构造
         * @param task
         * @param thread
         */
        Task(Fiber::ptr&& task, int64_t thread) : fiber(std::move(task)), threadId(thread) {}

        /**
         * @brief 同上，传入function构造
         * @param task
         * @param thread
         */
        Task(std::function<void()>& task, int64_t thread) : threadId(thread) { taskCb = task; }

        /**
         * @brief 同上，传入function构造
         * @param task
         * @param thread
         */
        Task(std::function<void()>&& task, int64_t thread) : taskCb(std::move(task)), threadId(thread) {}

        Task() : threadId(-1) {}

        void reset() {
            fiber = nullptr;
            taskCb = nullptr;
            threadId = -1;
        }
        explicit operator bool() { return fiber || taskCb; }
    };

protected:
    /**
     * @brief 无锁提交任务
     * @tparam FiberOrCb
     * @param fc
     * @param thrNum
     * @return
     */
    template <class FiberOrCb>
    bool submitNoLock(FiberOrCb&& fc, int64_t thrNum) {
        bool needNotify = m_task.empty();
        Task task(std::forward<FiberOrCb>(fc), thrNum);
        if (task) {
            m_task.push_back(task);
        }
        return needNotify;
    }

    /**
     * @brief 调度主函数
     */
    void run();

    /**
     * @brief 登记此线程上的调度器
     */
    void registerSelf();

    /**
     * @brief 当调度器没有可执行的任务时，执行此协程，等待调度任务的到来
     * @details 此基类调度器中只是简单的忙等
     */
    virtual void wait();

    /**
     * @brief 判断当前是否可以停止调度
     * @return
     */
    virtual bool canStop();

    bool hasIdleThreads() { return m_idle_thread_num > 0; }

private:
    MutexType m_mutex;                      /// 互斥锁
    std::list<Task> m_task;                 /// 待处理的任务，可以是协程，可以是function
    std::vector<Thread::ptr> m_thread_pool; /// 调度线程池，每个线程上运行一个调度器，调度其上的任务
    std::string m_name;                     /// 协程调度器的名称
    std::vector<uint64_t> m_thread_ids;
    size_t m_thr_num = 0;                       /// 线程总数
    std::atomic<size_t> m_active_thread_num{0}; /// 活跃线程数
    std::atomic<size_t> m_idle_thread_num{0};   /// 空闲线程数
    bool m_stopping = true;
};
} // namespace xzmjx

#endif // XZMJX_SCHEDULER_H
