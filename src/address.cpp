//
// Created by 20132 on 2022/4/9.
//
#include "address.h"
#include "log.h"
#include <netdb.h>
namespace xzmjx {
namespace {
    xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
}

template<typename T>
static uint32_t CountBits(T v){
    uint32_t ans=0;
    while(v){
        v&=(v-1);
        ans++;
    }
    return ans;
}

template<typename T>
static T CreateMask(uint32_t bits){
    return (1<<(sizeof(T)*8-bits)) -1;
}
Address::ptr Address::Create(const sockaddr *addr, socklen_t addrlen) {
    if(addr == nullptr){
        return nullptr;
    }
    Address::ptr result;
    switch (addr->sa_family) {
        case AF_INET:
            result.reset(new IPv4Address(*(reinterpret_cast<const sockaddr_in*>(addr))));
            break;
        case AF_INET6:
            result.reset(new IPv6Address(*(reinterpret_cast<const sockaddr_in6*>(addr))));
            break;
        default:
            result.reset(new UnknownAddress(*addr));
            break;
    }
    return result;
}

bool Address::Lookup(std::vector<Address::ptr> &result,
                     const std::string &host,
                     int family,
                     int type,
                     int protocol) {
    addrinfo hint,*results,*next;
    hint.ai_flags = 0;
    hint.ai_family = family;
    hint.ai_socktype = type;
    hint.ai_protocol = protocol;
    hint.ai_addrlen = 0;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;

    std::string node;
    const char* service = NULL;


    int error = getaddrinfo(node.c_str(),service,&hint,&results);
    if(error){
        XZMJX_LOG_ERROR(g_logger)<<"Address::Lookup getaddress("<<host<<","
                                 <<family<<", "<<type<<") err = "<<error<<" errstr = "
                                 <<gai_strerror(error);
        return false;
    }
    next = results;
    while(next){
        result.push_back(Create(next->ai_addr,(socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }
    freeaddrinfo(results);
    return !result.empty();
}


Address::ptr Address::LookupAny(const std::string &host,
                                int family,
                                int type,
                                int protocol) {
    std::vector<Address::ptr> result;
    if(Lookup(result,host,family,type,protocol)){
        return result[0];
    }
    return nullptr;
}

std::shared_ptr<IPAddress> Address::LookupAnyIPAddress(const std::string &host,
                                                       int family,
                                                       int type,
                                                       int protocol) {
    std::vector<Address::ptr> result;
    if(Lookup(result,host,family,type,protocol)){
        for(auto it:result){
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(it);
            if(v){
                return v;
            }
        }
    }
    return nullptr;
}


bool Address::GetInterfaceAddress(std::multimap<std::string, std::pair<Address::ptr, uint32_t>> &result,
                                  int family) {

}

bool Address::GetInterfaceAddress(std::pair<Address::ptr, uint32_t> &result,
                                  const std::string &iface,
                                  int family) {

}

int Address::getFamily() const {
    return getAddr()->sa_family;
}


std::string Address::toString() {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address &rhs) const {
    int min_size = std::min(getAddrLen(),rhs.getAddrLen());
    int result = memcmp(getAddr(),rhs.getAddr(),min_size);
    if(result<0){
        return true;
    }else if(result>0){
        return false;
    }else if(getAddrLen()<getAddrLen()){
        return true;
    }
    return false;
}

bool Address::operator==(const Address &rhs) const {
    return getAddrLen() == rhs.getAddrLen()
        && memcmp(getAddr(),rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address &rhs) const {
    return !(*this == rhs);
}


IPAddress::ptr IPAddress::Create(const char *address, uint16_t port) {

}





IPv4Address::ptr IPv4Address::Create(const char *address, uint16_t port) {
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_address.sin_port = EndianCast(port);
    int result = inet_pton(AF_INET, address, &rt->m_address.sin_addr);
    if (result <= 0) {
        XZMJX_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", "
                                  << port << ") rt=" << result << " errno=" << errno
                                  << " errstr=" << strerror(errno);
        return nullptr;
    }
    return rt;
}

IPv4Address::IPv4Address(const sockaddr_in &address) {
    m_address = address;
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port) {
    m_address.sin_addr.s_addr = EndianCast(address);
    m_address.sin_family = AF_INET;
    m_address.sin_port = EndianCast(port);

}

const sockaddr *IPv4Address::getAddr() const {
    return (sockaddr*)&m_address;
}

sockaddr *IPv4Address::getAddr() {
    return (sockaddr*)&m_address;
}

socklen_t IPv4Address::getAddrLen() const {
    return sizeof(m_address);
}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
    if(prefix_len>32){
        ///@details ipv4地址只有32位;
        return nullptr;
    }
    uint32_t ipv4_mask = CreateMask<uint32_t>(prefix_len);
    sockaddr_in copy = m_address;
    copy.sin_addr.s_addr |= EndianCast(ipv4_mask);
    IPv4Address::ptr ans = std::make_shared<IPv4Address>(copy);
    return ans;
}

IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len) {
    if(prefix_len>32){
        ///@details ipv4地址只有32位;
        return nullptr;
    }
    uint32_t ipv4_mask = CreateMask<uint32_t>(prefix_len);
    sockaddr_in copy = m_address;
    copy.sin_addr.s_addr &= EndianCast(ipv4_mask);
    IPv4Address::ptr ans = std::make_shared<IPv4Address>(copy);
    return ans;
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in subnet;
    memset(&subnet,0,sizeof(subnet));
    subnet.sin_addr.s_addr = ~EndianCast(CreateMask<uint32_t>(prefix_len));
    subnet.sin_family = AF_INET;
    IPv4Address::ptr ans = std::make_shared<IPv4Address>(subnet);
    return ans;
}

std::ostream& IPv4Address::insert(std::ostream& out){

}

uint32_t IPv4Address::getPort() const {
    return EndianCast(m_address.sin_port);
}

void IPv4Address::setPort(uint16_t port) {
    m_address.sin_port = EndianCast(port);
}

IPv6Address::ptr IPv6Address::Create(const char *address, uint16_t port) {
    IPv6Address::ptr ans = std::make_shared<IPv6Address>();
    ans->m_address.sin6_port = EndianCast(port);
    ans->m_address.sin6_family = AF_INET6;
    int error = inet_pton(AF_INET6,address,&ans->m_address.sin6_addr);
    if(error<=0){
        XZMJX_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address << ", "
                                  << port << ") rt=" << error << " errno=" << errno
                                  << " errstr=" << strerror(errno);
        return nullptr;
    }
    return ans;
}

IPv6Address::IPv6Address(const sockaddr_in6 &address) {
    m_address = address;
}
IPv6Address::IPv6Address() {
    memset(&m_address, 0, sizeof(m_address));
    m_address.sin6_family = AF_INET6;
}
IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port) {
    m_address.sin6_port = EndianCast(port);
    m_address.sin6_family = AF_INET6;
    ///@TODO： 这里已经是网络字节序了？怎么与IPv4的不一致
    memcpy(&m_address.sin6_addr.s6_addr,address,16);
}

const sockaddr *IPv6Address::getAddr() const {
    return (sockaddr*)&m_address;
}

sockaddr *IPv6Address::getAddr() {
    return (sockaddr*)&m_address;
}

socklen_t IPv6Address::getAddrLen() const {
    return sizeof(m_address);
}

std::ostream& IPv6Address::insert(std::ostream& out){

}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
    if(prefix_len>32){
        ///@details ipv6地址只有32位;
        return nullptr;
    }
    uint32_t ipv4_mask = CreateMask<uint32_t>(prefix_len);
    sockaddr_in copy = m_address;
    copy.sin_addr.s_addr |= EndianCast(ipv4_mask);
    IPv4Address::ptr ans = std::make_shared<IPv4Address>(copy);
    return ans;
}

IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len) {

}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {

}

uint32_t IPv6Address::getPort() const {
    return EndianCast(m_address.sin6_port);
}

void IPv6Address::setPort(uint16_t port) {
    m_address.sin6_port = EndianCast(port);
}


UnixAddress::UnixAddress() {

}

UnixAddress::UnixAddress(const std::string &path) {

}

UnixAddress::~UnixAddress() {

}

const sockaddr *UnixAddress::getAddr() const {

}

sockaddr *UnixAddress::getAddr() {

}

socklen_t UnixAddress::getAddrLen() const {

}

void UnixAddress::setAddrLen(uint32_t v) {

}

std::string UnixAddress::getPath() const {

}


UnknownAddress::UnknownAddress(int family) {

}

UnknownAddress::UnknownAddress(const sockaddr &addr) {

}

UnknownAddress::~UnknownAddress() {

}

const sockaddr *UnknownAddress::getAddr() const {

}

sockaddr *UnknownAddress::getAddr() {

}

socklen_t UnknownAddress::getAddrLen() const {

}

}