//
// Created by 20132 on 2022/3/15.
//

#ifndef XZMJX_MUTEX_H
#define XZMJX_MUTEX_H
#include "noncopyable.h"
#include <pthread.h>
#include <semaphore.h>

namespace xzmjx{
template<typename T>
class ScopedLockImpl{
public:
    ScopedLockImpl(T& mutex):m_mutex(mutex){
        m_mutex.lock();
        m_locked = true;
    }
    ~ScopedLockImpl(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }

    void lock(){
        if(!m_locked){
            m_mutex.lock();
            m_locked = true;
        }
    }
    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

template<typename T>
class WriteScopedLockImpl{
public:
    WriteScopedLockImpl(T& mutex):m_mutex(mutex){
        m_mutex.wrlock();
        m_locked = true;
    }
    ~WriteScopedLockImpl(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }

    void lock(){
        if(!m_locked){
            m_mutex.wrlock();
            m_locked = true;
        }
    }
    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

template<typename T>
class ReadScopedLockImpl{
public:
    ReadScopedLockImpl(T& mutex):m_mutex(mutex){
        m_mutex.rdlock();
        m_locked = true;
    }
    ~ReadScopedLockImpl(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }

    void lock(){
        if(!m_locked){
            m_mutex.rdlock();
            m_locked = true;
        }
    }
    void unlock(){
        if(m_locked){
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};
class Mutex:public Noncopyable{
public:
    typedef ScopedLockImpl<Mutex> Lock;

    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    pthread_mutex_t m_mutex;
};

class Semaphore: public Noncopyable{
public:
    Semaphore(int count = 0);
    ~Semaphore();

    void wait();
    void notify();
private:
    sem_t m_sem;
};

class RWMutex:public Noncopyable{
public:
    typedef  WriteScopedLockImpl<RWMutex> WriteLock;
    typedef ReadScopedLockImpl<RWMutex> ReadLock;

    RWMutex();
    ~RWMutex();

    void rdlock();
    void wrlock();
    void unlock();
private:
    pthread_rwlock_t m_rwmutex;
};

class Spinlock:public Noncopyable{
public:
    typedef ScopedLockImpl<Spinlock> Lock;

    Spinlock();
    ~Spinlock();

    void lock();
    void unlock();

private:
    pthread_spinlock_t m_mutex;
};
}///namespace xzmjx

#endif //XZMJX_MUTEX_H
