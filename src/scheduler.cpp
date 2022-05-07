//
// Created by 20132 on 2022/3/21.
//
#include "scheduler.h"
#include "log.h"
#include "marco.h"
#include "hook.h"

namespace xzmjx{
    static Logger::ptr g_logger = XZMJX_LOG_NAME("system");
    static thread_local Scheduler* t_scheduler;

    Scheduler::Scheduler(size_t thrNum,const std::string& name)
        : m_name(name), m_thr_num(thrNum){
        XZMJX_LOG_DEBUG(g_logger)<<"Scheduler::Scheduler()";
        t_scheduler = this;
    }
    Scheduler::~Scheduler(){
        XZMJX_ASSERT(m_stopping);
        XZMJX_LOG_DEBUG(g_logger)<<"Scheduler::~Scheduler()";
        if(Self() == this){
            t_scheduler = nullptr;
        }
    }

    void Scheduler::start(){
        ///@TODO 这里为什么要加锁
        MutexType::Lock lock(m_mutex);
        if(!m_stopping){
            return;
        }
        m_stopping = false;
        XZMJX_ASSERT(m_thread_pool.empty());

        ///@details 调度线程初始化
        m_thread_pool.resize(m_thr_num);
        m_thread_ids.resize(m_thr_num);
        for(size_t i = 0; i < m_thr_num; ++i){
            m_thread_pool[i].reset(new Thread([this] { this->run(); }
                                             ,m_name+"_"+std::to_string(i)));
            m_thread_ids[i] = m_thread_pool[i]->getThreadId();
        }
    }

    void Scheduler::stop(){
        m_stopping = true;
        for(size_t i = 0; i < m_thr_num; i++){
            notify();
        }

        std::vector<Thread::ptr> vec;
        vec.swap(m_thread_pool);
        for(auto&& thr:vec){
            thr->join();
        }
        XZMJX_LOG_INFO(g_logger)<<"Scheduler::stop";
    }

    void Scheduler::notify(){
        XZMJX_LOG_INFO(g_logger)<<"notify";
    }

    Scheduler* Scheduler::Self(){
        return t_scheduler;
    }

    void Scheduler::run(){
        ///启用hook，初始化协程
        ///@TODO 启用hook
        xzmjx::EnableHook();
        registerSelf();
        Fiber::EnableFiber();

        Fiber::ptr wait(new Fiber([this] { this->wait(); }));
        Fiber::ptr task_fb;
        Task task;
        while(true){
            bool notify_me = false;
            ///任务队列中取任务
            {
                MutexType::Lock lock(m_mutex);
                auto iter = m_task.begin();
                auto end = m_task.end();
                while(iter != end){
                    if(iter->threadId!=-1&&static_cast<uint64_t>(iter->threadId)!=GetThreadID()){
                        iter++;
                        notify_me = true;
                        continue;
                    }
                    XZMJX_ASSERT(iter->fiber||iter->taskCb);
                    /// 这里由于fiber的yield和加入调度队列这两不操作不是原子的，在一个fiber让出执行权时会做以下事情：
                    /// 1,加入调度队列
                    /// 2,yield让出执行权
                    /// 在1,2之间可能该fiber被执行到，这就导致fiber处于FIBER_RUNNING，这是不合理的
                    if(iter->fiber && iter->fiber->getState() == Fiber::FiberState::FIBER_RUNNING){
                        ++iter;
                        notify_me = true;
                        continue;
                    }
                    task = *iter;
                    m_task.erase(iter);
                    break;
                }
                if(!m_task.empty()){
                    notify_me = true;
                }
            }
            if(notify_me){
                ///任务队列尚有任务或者遇到一个指定在其他线程处理的任务，需要滴滴别人。
                notify();
            }
            if(task.fiber){
                ++m_active_thread_num;
                task.fiber->resume();
                --m_active_thread_num;

                ///如果协程仍处于READY，则继续将其添加至任务队列，等待下次调度。
                if(task.fiber->getState() == Fiber::FIBER_READY){
                    submit(task.fiber,task.threadId);
                }
                task.reset();
            }else if(task.taskCb){
                ++m_active_thread_num;
                if(task_fb){
                    task_fb->reset(task.taskCb);
                }else{
                    task_fb.reset(new Fiber(task.taskCb));
                }
                task.reset();
                task_fb->resume();
                --m_active_thread_num;
                if(task_fb->getState() == Fiber::FIBER_READY){
                    submit(task_fb,task.threadId);
                    ///此时这个协程被安插进任务队列，不可用来复用处理下次的任务
                    task_fb.reset();
                }else if(task_fb->getState() == Fiber::FIBER_TERM||
                         task_fb->getState() == Fiber::FIBER_EXCEPT){
                    ///当前协程出现异常或者执行结束，可以复用该协程处理其他任务
                    task_fb->reset(nullptr);
                }else{
                    ///其他情况不处理,由协程创建者自行处理。
                    task_fb.reset();
                }
            }else{
                if(wait->getState() == Fiber::FIBER_TERM){
                    ///此时调度器已经停止，不再进行调度。
                    break;
                }
                ++m_idle_thread_num;
                wait->resume();
                --m_idle_thread_num;
            }
        }
    }

    void Scheduler::registerSelf(){
        t_scheduler = this;
    }

    void Scheduler::wait(){
        XZMJX_LOG_INFO(g_logger)<<"wait";
        while(!canStop()){
            Fiber::YieldToHold();
        }
        ///@details 只有当调度器退出的时候，wait协程才会退出
        XZMJX_LOG_INFO(g_logger)<<"wait exit";
    }

    bool Scheduler::canStop(){
        MutexType::Lock lock(m_mutex);
        return m_stopping && m_active_thread_num == 0 && m_task.empty();
    }
}
