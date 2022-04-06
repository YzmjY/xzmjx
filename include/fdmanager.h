//
// Created by 20132 on 2022/4/6.
//

#ifndef XZMJX_FDMANAGER_H
#define XZMJX_FDMANAGER_H

#include <memory>

namespace xzmjx{
class FdCtx:std::enable_shared_from_this<FdCtx>{
public:
    typedef std::shared_ptr<FdCtx> ptr;

    FdCtx(int fd);
    ~FdCtx();

private:
    bool m_isInit:1;
    bool m_isSocket:1;
    bool m_sysNonblock:1;
    bool m_userNonblock:1;
    bool m_isClosed:1;
    int m_fd;
    uint64_t m_recvTimeout;
    uint64_t m_sendTimeout;
};
}


#endif //XZMJX_FDMANAGER_H
