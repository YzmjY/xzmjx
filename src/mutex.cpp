//
// Created by 20132 on 2022/3/15.
//
#include "mutex.h"
namespace xzmjx{
    Mutex::Mutex(){
        pthread_mutex_init(&m_mutex,NULL);
    }

    Mutex::~Mutex(){
        pthread_mutex_destroy(&m_mutex);
    }

    void Mutex::lock(){
        pthread_mutex_lock(&m_mutex);
    }

    void Mutex::unlock(){
        pthread_mutex_unlock(&m_mutex);
    }

    Semaphore::Semaphore(int count /*=0*/){
        sem_init(&m_sem,0,count);
    }

    Semaphore::~Semaphore(){
        sem_destroy(&m_sem);
    }

    void Semaphore::wait(){
        sem_wait(&m_sem);
    }

    void Semaphore::notify(){
        sem_post(&m_sem);
    }

    RWMutex::RWMutex(){
        pthread_rwlock_init(&m_rwmutex, NULL);
    }

    RWMutex::~RWMutex(){
        pthread_rwlock_destroy(&m_rwmutex);
    }

    void RWMutex::rdlock(){
        pthread_rwlock_rdlock(&m_rwmutex);
    }

    void RWMutex::wrlock(){
        pthread_rwlock_wrlock(&m_rwmutex);
    }

    void RWMutex::unlock(){
        pthread_rwlock_unlock(&m_rwmutex);
    }

    Spinlock::Spinlock(){
        pthread_spin_init(&m_mutex,0);
    }

    Spinlock::~Spinlock(){
        pthread_spin_destroy(&m_mutex);
    }

    bool Spinlock::tryLock() {
        return !(pthread_spin_trylock(&m_mutex));
    }

    void Spinlock::lock(){
        pthread_spin_lock(&m_mutex);
    }

    void Spinlock::unlock(){
        pthread_spin_unlock(&m_mutex);
    }
}


