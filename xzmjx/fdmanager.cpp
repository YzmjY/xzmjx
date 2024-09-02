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
#include <sstream>


namespace xzmjx{
    FdCtx::FdCtx(int fd):
            m_is_init(false),
            m_is_socket(false),
            m_sys_nonblock(false),
            m_user_nonblock(false),
            m_is_closed(false),
            m_fd(fd),
            m_recv_timeout(-1),
            m_send_timeout(-1){
        init();
    }

    FdCtx::~FdCtx(){

    }


    bool FdCtx::init(){
        if(m_is_init){
            return true;
        }
        m_recv_timeout = -1;
        m_send_timeout = -1;
        struct stat fd_stat;
        if(-1 == fstat(m_fd,&fd_stat)){
            m_is_init = false;
            m_is_socket = false;
        }else{
            m_is_init = true;
            m_is_socket = S_ISSOCK(fd_stat.st_mode);
        }

        if(m_is_socket){
            int flags = fcntl_f(m_fd,F_GETFL,0);
            if(!(flags&O_NONBLOCK)){
                fcntl_f(m_fd,F_SETFL,flags|O_NONBLOCK);
                m_sys_nonblock = true;
            }else{
                m_sys_nonblock = false;
            }
        }

        m_user_nonblock = false;
        m_is_closed = false;
        return m_is_init;
    }
    void FdCtx::setTimeout(int type,uint64_t timeout){
        if(type == SO_RCVTIMEO){
            m_recv_timeout = timeout;
        }else{
            m_send_timeout = timeout;
        }
    }
    uint64_t FdCtx::getTimeout(int type) const{
        if(type == SO_RCVTIMEO){
            return m_recv_timeout;
        }else{
            return m_send_timeout;
        }
    }

    std::string FdCtx::toString(){
        std::stringstream os;
        os<<"[fd="<<m_fd<<" ,init="<<m_is_init<<" ,isSock="<<m_is_socket<<" ,isClosed="<<m_is_closed
        <<" ,sysNonblock="<<m_sys_nonblock<<" ,userNonblock="<<m_user_nonblock<<" ,recvTimeout="<<m_recv_timeout
        <<" ,sendTimeout="<<m_send_timeout<<"]";
        return os.str();
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
