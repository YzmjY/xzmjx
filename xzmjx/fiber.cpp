//
// Created by 20132 on 2022/3/15.
//
#include "fiber.h"
#include "log.h"
#include <atomic>
#include "marco.h"
namespace xzmjx {
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
static std::atomic<uint64_t> s_fiberId{0};    /// 当前以分配的最大FiberId，FiberId只增不减
static std::atomic<uint64_t> s_fiberCount{0}; /// 系统协程总个数

/**
 * @details
 * 这里t_fiber使用原生指针的目的在于:不管协程模块外面是如何的使用，都不能出现由于协程的切换而导致该协程的析构
 *          而使用智能指针，则可能出现切换协程时，引用计数减至0，导致该协程析构,但我觉得不会，所以没管
 *
 */
static thread_local Fiber::ptr t_fiber; /// 该线程上当前正在跑的协程，每次切换协程要改变这个值
static thread_local Fiber::ptr
    t_mainFiber; /// 该线程上的主协程，即线程对应的协程，每次切换协程都以该协程为出发点和目的地

/**
 * @brief 协程栈分配器，简单使用malloc/free，后续可能增加mmap/unmap
 * 故设计dealloc接口时有参数size
 */
class MallocStackAllocator {
public:
    /**
     * @brief 分配栈内存
     * @param size
     * @return
     */
    static void* Alloc(size_t size) {
        XZMJX_LOG_DEBUG(g_logger) << "stack Alloc " << size << " bytes";
        return malloc(size);
    }

    /**
     * @brief 释放栈内存
     * @param ptr
     * @param size
     */
    static void Dealloc(void* ptr, size_t size) {
        XZMJX_LOG_DEBUG(g_logger) << "stack Free " << size << " bytes";
        free(ptr);
    }
};
typedef MallocStackAllocator StackAllocator;

Fiber::Fiber(std::function<void()> cb, uint32_t stackSize) : m_stackSize(stackSize), m_cb(move(cb)) {
    m_fiberId = ++s_fiberId;
    ++s_fiberCount;

    m_stackSize = stackSize ? stackSize : 128 * 1024;
    /// 分配协程栈，实际为进程堆上的空间，用来储存协程局部变量，内核对此一无所知
    m_stack = StackAllocator::Alloc(m_stackSize);
    m_state = FiberState::FIBER_INIT;
    if (getcontext(&m_context)) {
        XZMJX_ASSERT_2(false, "getcontext()");
    }
    m_context.uc_link = nullptr;
    m_context.uc_stack.ss_sp = m_stack;
    m_context.uc_stack.ss_size = m_stackSize;

    makecontext(&m_context, &Fiber::Coroutine, 0);
}

Fiber::Fiber() {
    ++s_fiberCount;
    m_fiberId = 0;
    m_stackSize = 0;
    m_stack = nullptr;
    m_state = FiberState::FIBER_INIT;
    if (getcontext(&m_context)) {
        XZMJX_ASSERT_2(false, "systm error:getcontext()");
    }
}

Fiber::~Fiber() {
    XZMJX_LOG_DEBUG(g_logger) << "Fiber::~Fiber()";
    --s_fiberCount;
    if (m_stack) {
        XZMJX_ASSERT(m_state == Fiber::FIBER_INIT || m_state == Fiber::FIBER_EXCEPT || m_state == Fiber::FIBER_TERM);
        StackAllocator::Dealloc(m_stack, m_stackSize);
    } else {
        /// 未分配栈，为主协程，此时的t_fiber应该就是this，由于t_mainFiber是一个线程局部变量，只有当该线程结束的时候，才会触发
        /// main_fiber的析构
        XZMJX_ASSERT(m_cb == nullptr);
        if (t_mainFiber.get() == this) {
            SetThis(nullptr);
        }
    }
}

void Fiber::resume() {
    SetThis(shared_from_this());
    XZMJX_ASSERT_2(m_state != Fiber::FIBER_RUNNING, "Fiber id = " + std::to_string(m_fiberId));

    m_state = FiberState::FIBER_RUNNING;
    /// XZMJX_LOG_DEBUG(g_logger)<<"Fiber "<<m_fiberId<<" resume";
    if (swapcontext(&t_mainFiber->m_context, &m_context)) {
        XZMJX_ASSERT_2(false, "system error:swapcontext()");
    }
}

void Fiber::yield() {
    SetThis(t_mainFiber);
    if (swapcontext(&m_context, &t_mainFiber->m_context)) {
        XZMJX_ASSERT_2(false, "system error:swapcontext()");
    }
}

void Fiber::reset(std::function<void()> cb) {
    XZMJX_ASSERT(m_stackSize);
    XZMJX_ASSERT(m_state == Fiber::FIBER_INIT || m_state == Fiber::FIBER_EXCEPT || m_state == Fiber::FIBER_TERM);
    m_cb = cb;
    if (getcontext(&m_context)) {
        XZMJX_ASSERT_2(false, "systm error:getcontext()");
    }

    m_context.uc_link = nullptr;
    m_context.uc_stack.ss_size = m_stackSize;
    m_context.uc_stack.ss_sp = m_stack;

    makecontext(&m_context, &Fiber::Coroutine, 0);
    m_state = FiberState::FIBER_INIT;
}

void Fiber::YieldToHold() {
    Fiber::ptr cur = Self();
    cur->m_state = FiberState::FIBER_HOLD;
    cur->yield();
}

void Fiber::YieldToReady() {
    Fiber::ptr cur = Self();
    cur->m_state = FiberState::FIBER_READY;
    cur->yield();
}

Fiber::ptr Fiber::EnableFiber() {
    if (t_mainFiber) {
        return t_mainFiber;
    }
    /// 构造函数中将t_fiber设置为this;
    Fiber::ptr mainFiber(new Fiber);
    SetThis(mainFiber);
    XZMJX_ASSERT(t_fiber.get() == mainFiber.get());
    t_mainFiber = mainFiber;
    return t_mainFiber;
}

Fiber::ptr Fiber::Self() {
    if (t_fiber) {
        return t_fiber;
    } else {
        return nullptr;
    }
}

void Fiber::SetThis(Fiber::ptr p) { t_fiber = p; }

uint64_t Fiber::GetFiberId() {
    if (t_fiber) {
        return t_fiber->getId();
    }
    ///@brief
    /// 只可能出现在Fiber::Fiber()中，此时还没有InitialMianFiber，所以导致t_fiber未赋值
    return 0;
}

uint64_t Fiber::TotalCount() { return s_fiberCount; }

void Fiber::Coroutine() {
    Fiber::ptr cur = Self();
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = FiberState::FIBER_TERM;
    } catch (std::exception& ex) {
        cur->m_state = Fiber::FIBER_EXCEPT;
        XZMJX_LOG_ERROR(g_logger) << "Fiber Exception " << ex.what();
    } catch (...) {
        cur->m_state = Fiber::FIBER_EXCEPT;
        XZMJX_LOG_ERROR(g_logger) << "Fiber Exception";
    }

    /// 局部变量cur由shared_from_this而来，增加了一个引用计数，Coroutine最后会让出执行权，若不手动减少一个引用计数会导致无法析构
    XZMJX_LOG_DEBUG(g_logger) << "Fiber: " << cur->m_fiberId << "terminate";
    Fiber* ptr = cur.get();
    cur.reset();
    ptr->yield();
}
} // namespace xzmjx
