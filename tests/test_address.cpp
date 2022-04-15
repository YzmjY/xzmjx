//
// Created by 20132 on 2022/4/15.
//
#include "log.h"
#include "address.h"

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
    return 0;
}
