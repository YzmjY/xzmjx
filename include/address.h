//
// Created by 20132 on 2022/4/9.
//

#ifndef XZMJX_ADDRESS_H
#define XZMJX_ADDRESS_H

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <sys/types.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>


namespace xzmjx{
class IPAddress;
class Address{
public:
    typedef std::shared_ptr<Address> ptr;
    Address() = default;
    virtual ~Address() = default;

    static Address::ptr Create(const sockaddr* addr,socklen_t addrlen);

    static bool Lookup(std::vector<Address::ptr>& result,
                       const std::string& host,
                       int family = AF_INET,
                       int type = 0,
                       int protocol = 0);

    static Address::ptr LookupAny(const std::string& host,
                                  int family = AF_INET,
                                  int type = 0,
                                  int protocol = 0);

    static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string& host,
                                                         int family = AF_INET,
                                                         int type = 0,
                                                         int protocol = 0);

    static bool GetInterfaceAddress(std::multimap<std::string,std::pair<Address::ptr,uint32_t>>&result,
                                    int family = AF_INET);

    static bool GetInterfaceAddress(std::pair<Address::ptr,uint32_t>&result,
                                    const std::string& iface,
                                    int family = AF_INET);

    int getFamily() const;

    virtual const sockaddr* getAddr() const = 0;

    virtual sockaddr* getAddr() = 0;

    virtual socklen_t getAddrLen() const = 0;

    virtual std::ostream& insert(std::ostream& out) = 0;

    std::string toString();

    bool operator<(const Address& rhs) const;

    bool operator==(const Address& rhs) const;
    bool operator!=(const Address& rhs) const;
};

class IPAddress:public Address{
public:
    typedef std::shared_ptr<IPAddress> ptr;
    IPAddress() = default;
    ~IPAddress() = default;

    static IPAddress::ptr Create(const char* address,uint16_t port = 0);

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

    virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;

    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint32_t getPort() const = 0;

    virtual void setPort(uint16_t port) = 0;
};

class IPv4Address:public IPAddress{
public:
    typedef std::shared_ptr<IPAddress> ptr;
    static IPv4Address::ptr Create(const char* address,uint16_t port = 0);

    IPv4Address(const sockaddr_in& address);
    IPv4Address(uint32_t address = INADDR_ANY,uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

    IPAddress::ptr networkAddress(uint32_t prefix_len) override;

    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    std::ostream& insert(std::ostream& out) override;

    uint32_t getPort() const override;

    void setPort(uint16_t port) override;

private:
    sockaddr_in m_address;
};

class IPv6Address:public IPAddress{
public:
    typedef std::shared_ptr<IPAddress> ptr;
    static IPv6Address::ptr Create(const char* address,uint16_t port = 0);

    IPv6Address(const sockaddr_in6& address);
    IPv6Address(const uint8_t address[16],uint16_t port = 0);

    const sockaddr* getAddr() const override;
    sockaddr* getAddr() override;
    socklen_t getAddrLen() const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

    IPAddress::ptr networkAddress(uint32_t prefix_len) override;

    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    std::ostream& insert(std::ostream& out) override;


    uint32_t getPort() const override;

    void setPort(uint16_t port) override;

private:
    sockaddr_in6 m_address;
};

class UnixAddress:public Address{
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress();
    UnixAddress(const std::string& path);
    ~UnixAddress();

    const sockaddr* getAddr() const override;

    sockaddr* getAddr() override;

    socklen_t getAddrLen() const override;

    std::ostream& insert(std::ostream& out) override;

    void setAddrLen(uint32_t v);

    std::string getPath() const;

private:
    sockaddr_un m_address;
    socklen_t m_length;
};

class UnknownAddress:public Address{
public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);
    ~UnknownAddress();

    const sockaddr* getAddr() const override;

    sockaddr* getAddr() override;

    socklen_t getAddrLen() const override;

    std::ostream& insert(std::ostream& out) override;

private:
    sockaddr m_address;
};




}

#endif //XZMJX_ADDRESS_H
