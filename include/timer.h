//
// Created by 20132 on 2022/3/27.
//

#ifndef XZMJX_TIMER_H
#define XZMJX_TIMER_H
#include <functional>
#include <memory>
#include "mutex.h"
#include <set>
#include <vector>

namespace xzmjx{
class TimerManager;
class Timer: public std::enable_shared_from_this<Timer>{
    friend class TimerManager;
public:
    typedef std::shared_ptr<Timer> ptr;

    ~Timer() = default;

    /**
     * @brief 取消一个定时器
     * @return true:取消成功 false：该定时器已被取消或者已经处理完毕
     */
    bool cancel();

    /**
     * @brief 刷新一个定时器，重置相对时间的起始点为当前时间
     * @return true:刷新成功 false:该定时器已被取消或者已经处理完毕
     */
    bool refresh();

    /**
     * @brief 重设定时器的执行周期
     * @param ms
     * @param from_now 是否从当前时间开始计算
     * @return
     */
    bool reset(uint64_t ms,bool from_now);

private:
    /**
     * @brief 构造
     * @param ms 执行时间周期
     * @param cb 到期后执行回调
     * @param recurring 是否为周期性定时时间
     * @param manager 所属定时管理器
     */
    Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* manager);

    /**
     * @brief 临时构造，查询到期时间时比较用
     * @param next
     */
    Timer(uint64_t next);

private:
    bool m_recurring = false;               ///是否循环该定时器
    uint64_t m_ms = 0;                      ///定时器相对间隔
    uint64_t m_next = 0;                    ///定时器的绝对过期时间
    std::function<void()> m_cb;             ///定时器回调函数
    TimerManager* m_manager = nullptr;      ///所属的定时器管理器

private:
    /**
     * @brief 基于过期时间比较两定时器的大小
     */
    struct Comparator{
        bool operator()(const Timer::ptr& lhs,const Timer::ptr& rhs) const;
    };
};

class TimerManager{
    friend class Timer;
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<TimerManager> ptr;

    TimerManager();
    virtual ~TimerManager();

    /**
     * @brief 添加一个定时器
     * @param ms
     * @param cb
     * @param recurring
     * @return
     */
    Timer::ptr addTimer(uint64_t ms,std::function<void()> cb,bool recurring = false);

    Timer::ptr addCondTimer(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> cond,bool recurring = false);

    /**
     * @brief 返回当前已到期的定时器，内部会一处过期且不周期执行的定时器，重新添加过期但需要周期执行的定时器
     * @param cbs ：返回值，返回待处理的定时任务
     */
    void listExpiredCb(std::vector<std::function<void()>>& cbs);

    /**
     * @brief 返回当前时间到最近一个定时时间的间隔，便于设置epoll_wait的超时时间
     * @return
     */
    uint64_t getNextTimerInterval();

    /**
     * @brief 设置当添加到定时器最前面时的处理回调
     * @param cb
     */
    void setInsertAtFrontCb(std::function<void()> cb);

private:
    void addTimer(Timer::ptr timer,RWMutexType::WriteLock& lock);
private:
    std::set<Timer::ptr,Timer::Comparator> m_timers;
    RWMutexType m_mutex;
    std::function<void()> m_onTimerInsertAtFrontCb;
};
}///namespace xzmjx



#endif //XZMJX_TIMER_H
