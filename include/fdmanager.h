//
// Created by 20132 on 2022/4/6.
//

#ifndef XZMJX_FDMANAGER_H
#define XZMJX_FDMANAGER_H

#include <memory>
#include <vector>
#include "mutex.h"
#include "singleton.h"

namespace xzmjx{
class FdCtx:std::enable_shared_from_this<FdCtx>{
public:
    typedef std::shared_ptr<FdCtx> ptr;

    FdCtx(int fd);
    ~FdCtx();
    bool init();

    bool isInit() const {return m_is_init;}

    bool isSocket() const {return m_is_socket;}

    bool isClose() const { return m_is_closed;}

    void setSysNonblock(bool flag){ m_sys_nonblock = flag;}
    bool getSysNonblock() const { return m_sys_nonblock;}

    void setUserNonblock(bool flag){ m_user_nonblock = flag;}
    bool getUserNonblock() const { return m_user_nonblock;}

    void setTimeout(int type,uint64_t timeout);
    uint64_t getTimeout(int type) const;
private:
    bool m_is_init:1;
    bool m_is_socket:1;
    bool m_sys_nonblock:1;
    bool m_user_nonblock:1;
    bool m_is_closed:1;
    int m_fd;
    uint64_t m_recv_timeout;
    uint64_t m_send_timeout;
};


class FdManager{
public:
    typedef RWMutex RWMutexType;
    typedef std::shared_ptr<FdManager> ptr;

    FdManager();
    ~FdManager()  =default;

    FdCtx::ptr get(int fd,bool auto_create = false);
    void del(int fd);

private:
    RWMutexType m_mutex;
    std::vector<FdCtx::ptr> m_container;
};

typedef SingletonPtr<FdManager> FdMgr;
}


#endif //XZMJX_FDMANAGER_H
