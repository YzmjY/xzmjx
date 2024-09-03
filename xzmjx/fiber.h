//
// Created by 20132 on 2022/3/15.
//

#ifndef XZMJX_FIBER_H
#define XZMJX_FIBER_H
#include <memory>
#include <ucontext.h>
#include <functional>
#include <queue>

/**
 * 非对称协程。切换规则：
 * Thread-------->main_fiber<------>sub_fiber
 *                  ^
 *                  |
 *                  |
 *                  v
 *                  sub_fiber
 * fiber之间不进行切换，需要先切换会main_fiber
 * 当前协程和主协程通过一个线程局部变量保存
 */
namespace xzmjx {
/**
 * @brief 协程类，封装了ucontext相关接口，实现了yield和resume语义的操作
 * @details
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;
    typedef std::weak_ptr<Fiber> wptr;

    /**
     * @brief 协程状态枚举
     */
    enum FiberState {
        FIBER_INIT,    /// 初始化中
        FIBER_RUNNING, /// 执行中
        FIBER_HOLD,    /// 被挂起
        FIBER_READY,   /// 就绪
        FIBER_TERM,    /// 终止
        FIBER_EXCEPT   /// 异常
    };

public:
    /**
     * @brief 构造一个协程，指定协程执行体和协程栈大小
     * @details
     * 这里只是调用ucontext的接口创建了一个协程对象，但此时协程并没有跑起来，需要resume才行
     * @param cb：函数对象
     * @param stackSize：指定的栈大小
     * @param run_on_scheduler：是否使用调度器调度
     */
    explicit Fiber(std::function<void()> cb, uint32_t stackSize = 0);

    /**
     * @brief 析构函数，释放当前协程栈
     */
    ~Fiber();

    /**
     * @brief 切换当前协程至RUNNING态
     * @details 此函数一定是在主协程内调用
     */
    void resume();

    /**
     * @brief 将当前协程切换到后台
     */
    void yield();

    /**
     * @brief 重置当前协程的执行体，只有当当前协程的状态为TERM或INIT才能调用
     */
    void reset(std::function<void()>);

    /**
     * @brief 返回当前协程id
     * @return
     */
    uint64_t getId() const { return m_fiberId; }

    FiberState getState() { return m_state; }

    /**
     * @brief 切换当前协程到后台，并且设置为HOLD状态
     * @details 此函数一定是在子协程内调用
     */
    static void YieldToHold();

    /**
     * @brief 切换当前协程到后台，并设置为READY
     */
    static void YieldToReady();

    /**
     * @brief 初始化主协程
     * @details 主协程相当于时当前的线程，这个协程不是由我们的Fiber创建的，
     *          通过这个函数将Fiber的一些属性初始化
     */
    static Fiber::ptr EnableFiber();

    /**
     * @brief 获取当前Fiber的指针
     * @return
     */
    static Fiber::ptr Self();

    /**
     * @brief 设置当前的协程
     * @details 通过修改保存当前协程的线程局部变量实现
     * @param p
     */
    static void SetThis(Fiber::ptr p);

    /**
     * @brief 静态方法，获取当前协程id
     * @return
     */
    static uint64_t GetFiberId();

    /**
     * @brief 返回协程总数
     * @return
     */
    static uint64_t TotalCount();

private:
    /**
     * @brief 用来初始化主协程，只能内部Init的时候使用，其余协程必须提供入口函数
     */
    Fiber();

    /**
     * @brief 协程执行体
     */
    static void Coroutine();

private:
    ucontext_t m_context;       /// context上下文
    uint32_t m_stackSize;       /// 协程栈大小
    void* m_stack;              /// 协程栈地址
    std::function<void()> m_cb; /// 协程回调入口
    uint64_t m_fiberId;         /// 协程id号
    FiberState m_state;
};
} // namespace xzmjx

#endif // XZMJX_FIBER_H
