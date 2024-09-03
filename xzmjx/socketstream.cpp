//
// Created by 20132 on 2022/4/17.
//
#include "socketstream.h"
#include "log.h"

namespace xzmjx {
SocketStream::SocketStream(Socket::ptr sock, bool owner) : m_socket(sock), m_owner(owner) {}

SocketStream::~SocketStream() {
    if (m_owner && isConnected()) {
        m_socket->close();
    }
}

int SocketStream::read(void* buf, size_t length) {
    if (!isConnected()) {
        return -1;
    }

    return m_socket->recv(buf, length);
}

int SocketStream::read(ByteArray::ptr ba, size_t length) {
    if (!isConnected()) {
        return -1;
    }
    std::vector<iovec> iov;
    ba->getWriteBuffers(iov, length);
    int rt = m_socket->recv(&iov[0], iov.size());
    if (rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

int SocketStream::write(const void* buf, size_t length) {
    if (!isConnected()) {
        return -1;
    }
    return m_socket->send(buf, length);
}

int SocketStream::write(ByteArray::ptr ba, size_t length) {
    if (!isConnected()) {
        return -1;
    }
    std::vector<iovec> iov;
    ba->getReadBuffers(iov, length);
    int rt = m_socket->send(&iov[0], length);
    if (rt > 0) {
        ba->setPosition(ba->getPosition() + rt);
    }
    return rt;
}

void SocketStream::close() {
    if (m_socket) {
        m_socket->close();
    }
}

bool SocketStream::isConnected() const {
    if (m_socket) {
        return m_socket->isConnected();
    }
    return false;
}

Address::ptr SocketStream::getLocalAddress() {
    if (m_socket) {
        return m_socket->getLocalAddress();
    }
    return nullptr;
}

Address::ptr SocketStream::getPeerAddress() {
    if (m_socket) {
        return m_socket->getPeerAddress();
    }
    return nullptr;
}

std::string SocketStream::getLocalAddresString() {
    if (m_socket) {
        return m_socket->getLocalAddress()->toString();
    }
    return "";
}

std::string SocketStream::getPeerAddressString() {
    if (m_socket) {
        return m_socket->getPeerAddress()->toString();
    }
    return "";
}
} // namespace xzmjx