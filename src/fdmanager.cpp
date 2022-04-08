//
// Created by 20132 on 2022/4/6.
//
#include "fdmanager.h"
#include "hook.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>


namespace xzmjx{
    FdCtx::FdCtx(int fd):
        m_isInit(false),
        m_isSocket(false),
        m_sysNonblock(false),
        m_userNonblock(false),
        m_isClosed(false),
        m_fd(fd),
        m_recvTimeout(-1),
        m_sendTimeout(-1){
        init();
    }

    FdCtx::~FdCtx(){

    }


    bool FdCtx::init(){
        if(m_isInit){
            return true;
        }
        m_recvTimeout = -1;
        m_sendTimeout = -1;
        struct stat fd_stat;
        if(-1 == fstat(m_fd,&fd_stat)){
            m_isInit = false;
            m_isSocket = false;
        }else{
            m_isInit = true;
            m_isSocket = S_ISSOCK(m_fd);
        }

        if(m_isSocket){
            int flags = fcntl_f(m_fd,F_GETFL,0);
            if(!(flags&O_NONBLOCK)){
                fcntl_f(m_fd,F_SETFL,flags|O_NONBLOCK);
                m_sysNonblock = true;
            }else{
                m_sysNonblock = false;
            }
        }

        m_userNonblock = false;
        m_isClosed = false;
        return m_isInit;
    }
    void FdCtx::setTimeout(int type,uint64_t timeout){
        if(type == SO_RCVTIMEO){
            m_recvTimeout = timeout;
        }else{
            m_sendTimeout = timeout;
        }
    }
    uint64_t FdCtx::getTimeout(int type) const{
        if(type == SO_RCVTIMEO){
            return m_recvTimeout;
        }else{
            return m_sendTimeout;
        }
    }

    FdManager::FdManager(){
        m_container.resize(64);
    }


    FdCtx::ptr FdManager::get(int fd,bool auto_create){
        if(fd<0){
            return nullptr;
        }
        RWMutexType::ReadLock lock(m_mutex);
        if(fd>=(int)m_container.size()){
            if(!auto_create){
                return nullptr;
            }
        }else{
            if(m_container[fd]||!auto_create){
                return m_container[fd];
            }
        }
        lock.unlock();

        RWMutexType::WriteLock lock1(m_mutex);
        FdCtx::ptr ctx = std::make_shared<FdCtx>(fd);
        if(fd>=(int)m_container.size()){
            m_container.resize(fd*1.5);
        }
        m_container[fd] = ctx;
        return ctx;
    }
    void FdManager::del(int fd){
        RWMutexType::WriteLock lock(m_mutex);
        if(fd>=(int)m_container.size()){
            return;
        }
        m_container[fd].reset();
    }
}///namespace xzmjx
