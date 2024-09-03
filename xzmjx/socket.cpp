//
// Created by 20132 on 2022/4/16.
//
#include "socket.h"
#include "log.h"
#include "hook.h"
#include "fdmanager.h"
#include "iomanager.h"
#include <netinet/tcp.h>

namespace xzmjx {
namespace {
xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
}

Socket::ptr Socket::CreateTCP(Address::ptr address) { return Socket::ptr(new Socket(address->getFamily(), TCP, 0)); }

Socket::ptr Socket::CreateUDP(Address::ptr address) { return Socket::ptr(new Socket(address->getFamily(), UDP, 0)); }

Socket::ptr Socket::CreateTCPSocket() { return Socket::ptr(new Socket(IPv4, TCP, 0)); }

Socket::ptr Socket::CreateUDPSocket() { return Socket::ptr(new Socket(IPv4, UDP, 0)); }

Socket::ptr Socket::CreateTCPSocket6() { return Socket::ptr(new Socket(IPv6, TCP, 0)); }

Socket::ptr Socket::CreateUDPSocket6() { return Socket::ptr(new Socket(IPv6, UDP, 0)); }

Socket::ptr Socket::CreateUnixTCPSocket() { return Socket::ptr(new Socket(UNIX, TCP, 0)); }

Socket::ptr Socket::CreateUnicUDPSocket() { return Socket::ptr(new Socket(UNIX, UDP, 0)); }

Socket::Socket(int family, int type, int protocol)
    : m_sock_fd(-1), m_family(family), m_type(type), m_protocol(protocol), m_is_connect(false) {}

Socket::~Socket() { close(); }

uint64_t Socket::getSendTimeout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock_fd);
    if (ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(uint64_t v) {
    struct timeval tv {
        int(v / 1000), int(v % 1000 * 1000)
    };
    ;
    setOpt(SOL_SOCKET, SO_SNDTIMEO, tv);
}

uint64_t Socket::getRecvTimeout() {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(m_sock_fd);
    if (ctx) {
        return ctx->getTimeout(SO_RCVTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(uint64_t v) {
    struct timeval tv {
        int(v / 1000), int(v % 1000 * 1000)
    };
    setOpt(SOL_SOCKET, SO_RCVTIMEO, tv);
}

bool Socket::getOpt(int level, int optname, void* optval, socklen_t* optlen) {
    int rt = getsockopt(m_sock_fd, level, optname, optval, optlen);
    if (rt != 0) {
        XZMJX_LOG_DEBUG(g_logger) << "getOpt sock=" << m_sock_fd << " level=" << level << " optname=" << optname
                                  << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOpt(int level, int optname, const void* optval, socklen_t optlen) {
    int rt = setsockopt(m_sock_fd, level, optname, optval, optlen);
    if (rt != 0) {
        XZMJX_LOG_DEBUG(g_logger) << "setOpt sock=" << m_sock_fd << " level=" << level << " optname=" << optname
                                  << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept() {
    Socket::ptr sock = std::make_shared<Socket>(m_family, m_type, m_protocol);
    int client_sock = ::accept(m_sock_fd, NULL, NULL);
    if (client_sock == -1) {
        XZMJX_LOG_ERROR(g_logger) << " accept(" << m_sock_fd << ")errno=" << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    if (sock->init(client_sock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::bind(const Address::ptr addr) {
    if (!isVaild()) {
        newSock();
        if (!isVaild()) {
            return false;
        }
    }

    ///@TODO: 考虑unix域套接字

    if (::bind(m_sock_fd, addr->getAddr(), addr->getAddrLen())) {
        XZMJX_LOG_ERROR(g_logger) << " bind():sock_fd=" << m_sock_fd << " address=" << addr->toString()
                                  << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if (!isVaild()) {
        XZMJX_LOG_ERROR(g_logger) << "listen error sock=-1";
    }
    if (::listen(m_sock_fd, backlog)) {
        XZMJX_LOG_ERROR(g_logger) << "listen error errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    m_peer_address = addr;
    if (!isVaild()) {
        newSock();
        if (!isVaild()) {
            return false;
        }
    }
    if (timeout_ms == (uint64_t)-1) {
        if (::connect(m_sock_fd, addr->getAddr(), addr->getAddrLen())) {
            XZMJX_LOG_ERROR(g_logger) << "connect() sock=" << m_sock_fd << " address=" << addr->toString()
                                      << " errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    } else {
        if (::connect_with_timeout(m_sock_fd, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
            XZMJX_LOG_ERROR(g_logger) << "connect() sock=" << m_sock_fd << " address=" << addr->toString()
                                      << " errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    }
    m_is_connect = true;
    getPeerAddress();
    getLocalAddress();
    return true;
}

bool Socket::reconnect(uint64_t timeout_ms) {
    if (!m_peer_address) {
        XZMJX_LOG_ERROR(g_logger) << "reconnect peer_address is null";
    }
    m_local_address.reset();
    return connect(m_peer_address, timeout_ms);
}

bool Socket::close() {
    if (!m_is_connect && m_sock_fd == -1) {
        return true;
    }
    m_is_connect = false;
    if (m_sock_fd != -1) {
        ::close(m_sock_fd);
        m_sock_fd = -1;
    }
    return -1;
}

int Socket::send(const void* buffer, size_t length, int flags) {
    if (m_is_connect) {
        return ::send(m_sock_fd, buffer, length, flags);
    }
    return -1;
}

int Socket::send(const iovec* buffers, size_t length, int flags) {
    if (m_is_connect) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::sendmsg(m_sock_fd, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void* buffer, size_t length, const Address::ptr address, int flags) {
    if (m_is_connect) {
        return ::sendto(m_sock_fd, buffer, length, flags, address->getAddr(), address->getAddrLen());
    }
    return -1;
}

int Socket::sendTo(const iovec* buffers, size_t length, const Address::ptr address, int flags) {
    if (m_is_connect) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = address->getAddr();
        msg.msg_namelen = address->getAddrLen();
        return ::sendmsg(m_sock_fd, &msg, flags);
    }
    return -1;
}

int Socket::recv(void* buffer, size_t length, int flags) {
    if (m_is_connect) {
        return ::recv(m_sock_fd, buffer, length, flags);
    }
    return -1;
}

int Socket::recv(iovec* buffers, size_t length, int flags) {
    if (m_is_connect) {
        msghdr msg;
        memset(&msg, 0, sizeof(msg));
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock_fd, &msg, flags);
    }
    return -1;
}

int Socket::recvFrom(void* buffer, size_t length, const Address::ptr address, int flags) {
    if (m_is_connect) {
        socklen_t len = address->getAddrLen();
        return ::recvfrom(m_sock_fd, buffer, length, flags, address->getAddr(), &len);
    }
    return -1;
}

int Socket::recvFrom(iovec* buffers, size_t length, const Address::ptr address, int flags) {
    if (m_is_connect) {
        msghdr msg;
        msg.msg_iov = (iovec*)buffers;
        msg.msg_iovlen = length;
        msg.msg_name = address->getAddr();
        msg.msg_namelen = address->getAddrLen();

        return ::recvmsg(m_sock_fd, &msg, flags);
    }
    return -1;
}

Address::ptr Socket::getLocalAddress() {
    if (m_local_address == nullptr) {
        Address::ptr ans;
        switch (m_family) {
            case AF_INET: ans.reset(new IPv4Address); break;
            case AF_INET6: ans.reset(new IPv6Address); break;
            case AF_UNIX: ans.reset(new UnixAddress); break;
            default: ans.reset(new UnknownAddress(m_family));
        }
        socklen_t len = ans->getAddrLen();
        int rt = getsockname(m_sock_fd, ans->getAddr(), &len);
        if (rt) {
            return Address::ptr(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX) {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(ans);
            addr->setAddrLen(len);
        }
        m_local_address = ans;
    }
    return m_local_address;
}

Address::ptr Socket::getPeerAddress() {
    if (m_peer_address == nullptr) {
        Address::ptr ans;
        switch (m_family) {
            case AF_INET: ans.reset(new IPv4Address); break;
            case AF_INET6: ans.reset(new IPv6Address); break;
            case AF_UNIX: ans.reset(new UnixAddress); break;
            default: ans.reset(new UnknownAddress(m_family));
        }
        socklen_t len = ans->getAddrLen();
        int rt = getpeername(m_sock_fd, ans->getAddr(), &len);
        if (rt) {
            return Address::ptr(new UnknownAddress(m_family));
        }
        if (m_family == AF_UNIX) {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(ans);
            addr->setAddrLen(len);
        }
        m_peer_address = ans;
    }
    return m_peer_address;
}

bool Socket::isVaild() const { return m_sock_fd != -1; }

std::ostream& Socket::dump(std::ostream& os) const {
    os << "[socket sock=" << m_sock_fd << " is_connect=" << m_is_connect << " family=" << m_family << " type=" << m_type
       << " protocol=" << m_protocol;
    if (m_local_address) {
        os << " local_address=" << m_local_address->toString();
    }
    if (m_peer_address) {
        os << " peer_address=" << m_peer_address->toString();
    }
    os << "]";
    return os;
}

std::string Socket::toString() {
    std::stringstream os;
    dump(os);
    return os.str();
}

int Socket::getError() {
    int error;
    bool rt = getOpt(SOL_SOCKET, SO_ERROR, error);
    if (!rt) {
        error = errno;
    }
    return error;
}

bool Socket::cancelRead() { return IOManager::Self()->cancelEvent(m_sock_fd, IOManager::Event_READ); }

bool Socket::cancelWrite() { return IOManager::Self()->cancelEvent(m_sock_fd, IOManager::Event_WRITE); }

bool Socket::cancelAll() { return IOManager::Self()->cancelAll(m_sock_fd); }

bool Socket::cancelAccept() { return IOManager::Self()->cancelEvent(m_sock_fd, IOManager::Event_READ); }

void Socket::initSock() {
    int val = 1;
    setOpt(SOL_SOCKET, SO_REUSEADDR, val);
    if (m_type == SOCK_STREAM) {
        setOpt(IPPROTO_TCP, TCP_NODELAY, val);
    }
}

void Socket::newSock() {
    m_sock_fd = socket(m_family, m_type, m_protocol);
    if (m_sock_fd != -1) {
        initSock();
    } else {
        XZMJX_LOG_ERROR(g_logger) << "socket(" << m_family << "," << m_type << "," << m_protocol << ")errno=" << errno
                                  << " strerr=" << strerror(errno);
    }
}

bool Socket::init(int sock) {
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(sock);
    if (ctx && ctx->isSocket() && !ctx->isClose()) {
        m_sock_fd = sock;
        m_is_connect = true;
        initSock();
        getLocalAddress();
        getPeerAddress();
        return true;
    }
    return false;
}
} // namespace xzmjx
