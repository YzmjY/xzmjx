//
// Created by 20132 on 2022/4/15.
//

#ifndef XZMJX_SOCKET_H
#define XZMJX_SOCKET_H
#include "address.h"
#include "noncopyable.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>

namespace xzmjx {
class Socket : public std::enable_shared_from_this<Socket>, Noncopyable {
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;
    enum Type { TCP = SOCK_STREAM, UDP = SOCK_DGRAM };

    enum Family { IPv4 = AF_INET, IPv6 = AF_INET6, UNIX = AF_UNIX };

    static Socket::ptr CreateTCP(Address::ptr address);
    static Socket::ptr CreateUDP(Address::ptr address);
    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();
    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();
    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnicUDPSocket();

    Socket(int family, int type, int protocol = 0);
    ~Socket();

    uint64_t getSendTimeout();
    void setSendTimeout(uint64_t v);

    uint64_t getRecvTimeout();
    void setRecvTimeout(uint64_t v);

    bool getOpt(int level, int optname, void* optval, socklen_t* optlen);

    template <class T>
    bool getOpt(int level, int optname, T& optval) {
        socklen_t length = sizeof(optval);
        return getOpt(level, optname, &optval, &length);
    }

    bool setOpt(int level, int optname, const void* optval, socklen_t optlen);

    template <class T>
    bool setOpt(int level, int optname, const T& optval) {
        socklen_t length = sizeof(optval);
        return setOpt(level, optname, &optval, length);
    }

    /// 服务端流程：socket->bind->listen->accept
    Socket::ptr accept();
    bool bind(const Address::ptr addr);
    bool listen(int backlog = 5);

    /// 客户端流程:connect
    bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
    bool reconnect(uint64_t timeout_ms = -1);

    bool close();
    int send(const void* buffer, size_t length, int flags = 0);
    int send(const iovec* buffers, size_t length, int flags = 0);
    int sendTo(const void* buffer, size_t length, const Address::ptr address, int flags = 0);
    int sendTo(const iovec* buffers, size_t length, const Address::ptr address, int flags = 0);
    int recv(void* buffer, size_t length, int flags = 0);
    int recv(iovec* buffers, size_t length, int flags = 0);
    int recvFrom(void* buffer, size_t length, const Address::ptr address, int flags = 0);
    int recvFrom(iovec* buffers, size_t length, const Address::ptr address, int flags = 0);

    Address::ptr getLocalAddress();
    Address::ptr getPeerAddress();

    int getFamily() const { return m_family; }
    int getType() const { return m_type; }
    int getProtocol() const { return m_protocol; }
    bool isConnected() const { return m_is_connect; }

    bool isVaild() const;
    std::ostream& dump(std::ostream& os) const;
    std::string toString();

    int getError();

    int getSocket() const { return m_sock_fd; }

    bool cancelRead();
    bool cancelWrite();
    bool cancelAll();
    bool cancelAccept();

public:
    void initSock();
    void newSock();
    bool init(int sock);

private:
    int m_sock_fd;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_is_connect;
    Address::ptr m_local_address;
    Address::ptr m_peer_address;
};
} // namespace xzmjx

#endif // XZMJX_SOCKET_H
