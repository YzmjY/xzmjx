//
// Created by 20132 on 2022/3/27.
//
#include "timer.h"
#include "log.h"

namespace xzmjx{
    static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");

    bool Timer::cancel(){
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(m_cb == nullptr){
            return false;
        }
        m_cb = nullptr;
        auto iter = m_manager->m_timers.find(shared_from_this());
        if(iter !=  m_manager->m_timers.end()){
            m_manager->m_timers.erase(iter);
            return true;
        }
        return false;
    }

    bool Timer::refresh(){
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(m_cb == nullptr){
            return false;
        }
        auto iter = m_manager->m_timers.find(shared_from_this());
        if(iter != m_manager->m_timers.end()){
            m_manager->m_timers.erase(iter);
            m_next = xzmjx::GetCurMS()+m_ms;
            m_manager->m_timers.insert(shared_from_this()); ///只会比原来更靠后，所以不会触发onInsertFront
            return true;
        }
        return false;
    }

    bool Timer::reset(uint64_t ms,bool from_now){
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if(m_cb == nullptr){
            return false;
        }
        auto iter = m_manager->m_timers.find(shared_from_this());
        if(iter != m_manager->m_timers.end()){
            m_manager->m_timers.erase(iter);
            uint64_t start = 0;
            if(from_now){
                start = xzmjx::GetCurMS();
            }else{
                start = m_next-m_ms;
            }
            m_next =start+ms;
            m_ms = ms;
            m_manager->addTimer(shared_from_this(),lock);
            return true;
        }
        return false;
    }

    Timer::Timer(uint64_t ms,std::function<void()> cb,bool recurring,TimerManager* manager)
                :m_recurring(recurring),m_ms(ms),m_cb(std::move(cb)),m_manager(manager){
        m_next = xzmjx::GetCurMS()+m_ms;
    }

    Timer::Timer(uint64_t next):m_next(next){}

    bool Timer::Comparator::operator()(const Timer::ptr& lhs,const Timer::ptr& rhs) const{
        if(!lhs&&!rhs){
            return false;
        }else if(!lhs){
            return true;
        }else if(!rhs){
            return false;
        }else if(lhs->m_next<rhs->m_next){
            return true;
        }else if(lhs->m_next>rhs->m_next){
            return false;
        }else{
            return lhs.get()<rhs.get();
        }
    }


    TimerManager::TimerManager(){

    }

    TimerManager::~TimerManager(){

    }

    Timer::ptr TimerManager::addTimer(uint64_t ms,std::function<void()> cb,bool recurring){
        Timer::ptr timer(new Timer (ms,std::move(cb),recurring,this));
        RWMutexType::WriteLock lock(m_mutex);
        addTimer(timer,lock);
        return timer;
    }

    static void OnTimer(std::weak_ptr<void> cond,std::function<void()> cb){
        std::shared_ptr<void> ptr = cond.lock();
        if(ptr){
            cb();
        }
    }
    Timer::ptr TimerManager::addCondTimer(uint64_t ms,std::function<void()> cb,std::weak_ptr<void> cond,bool recurring){
        return addTimer(ms,std::bind(&OnTimer,cond,cb),recurring);
    }

    void TimerManager::listExpiredCb(std::vector<std::function<void()>>& cbs){
        uint64_t now = xzmjx::GetCurMS();
        Timer::ptr now_timer(new Timer(now));
        RWMutexType::WriteLock lock(m_mutex);
        auto iter = m_timers.lower_bound(now_timer);
        while(iter!=m_timers.end()&&(*iter)->m_next == now){
            iter++;
        }
        std::vector<Timer::ptr> expire;
        expire.insert(expire.begin(),m_timers.begin(),iter);
        m_timers.erase(m_timers.begin(),iter);
        cbs.reserve(expire.size());

        for(size_t i = 0; i< expire.size();i++){
            if(expire[i]->m_cb){
                cbs.push_back(expire[i]->m_cb);
            }
            if(expire[i]->m_recurring){
                expire[i]->m_next = xzmjx::GetCurMS()+expire[i]->m_ms;
                m_timers.insert(expire[i]);
            }else{
                expire[i]->m_cb = nullptr;
            }
        }

    }

    uint64_t TimerManager::getNextTimerInterval(){
        RWMutexType::ReadLock lock(m_mutex);
        if(m_timers.empty()){
            XZMJX_LOG_INFO(g_logger)<<"No Timer Left";
            return ~0ull;
        }
        Timer::ptr next = *m_timers.begin();
        uint64_t now = xzmjx::GetCurMS();
        if(now>=next->m_next){
            return 0;
        }else{
            return next->m_next-now;
        };
    }

    void TimerManager::addTimer(Timer::ptr timer,RWMutexType::WriteLock& lock){
        auto iter = m_timers.insert(timer);
        bool at_front = false;
        if(iter.first == m_timers.begin()){
            at_front = true;
        }
        lock.unlock();
        if(at_front&&m_onTimerInsertAtFrontCb){
            m_onTimerInsertAtFrontCb();
        }
    }

    void TimerManager::setInsertAtFrontCb(std::function<void()> cb){
        m_onTimerInsertAtFrontCb = std::move(cb);
    }


}


