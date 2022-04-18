//
// Created by 20132 on 2022/4/15.
//
#include "log.h"
#include "address.h"
#include <iostream>

xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();
int main(){
    std::vector<xzmjx::Address::ptr> res;
    xzmjx::Address::Lookup(res,"www.baidu.com:80");
    for(size_t i = 0; i< res.size();i++){
        XZMJX_LOG_DEBUG(g_logger)<<
        std::dynamic_pointer_cast<xzmjx::IPv4Address>(res[i])->broadcastAddress(16)->toString();

        XZMJX_LOG_DEBUG(g_logger)<<
        std::dynamic_pointer_cast<xzmjx::IPv4Address>(res[i])->subnetMask(16)->toString();

        XZMJX_LOG_DEBUG(g_logger)<<
        std::dynamic_pointer_cast<xzmjx::IPv4Address>(res[i])->networkAddress(16)->toString();

        XZMJX_LOG_DEBUG(g_logger)<<res[i]->toString();
    }


    xzmjx::IPv6Address::ptr ipv6 = xzmjx::IPv6Address::Create("1050:0000:0000:0000:0005:0600:300c:326b",8088);

    sockaddr* addr = ipv6->getAddr();
    sockaddr_in6* p = (sockaddr_in6*)addr;
    uint8_t* out = (uint8_t*)p->sin6_addr.s6_addr;
    for(int i=0;i<16;i++){
        std::cout<<std::hex<<(int)out[i];
    }
    std::cout<<std::endl;

    XZMJX_LOG_INFO(g_logger)<<ipv6->toString();
    return 0;
}
