//
// Created by 20132 on 2022/4/16.
//
#include "log.h"
#include "socket.h"
#include "iomanager.h"
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fdmanager.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();

void test_socket(){
    xzmjx::IPAddress::ptr addr = xzmjx::Address::LookupAnyIPAddress("127.0.0.1");
    if(addr){
        XZMJX_LOG_INFO(g_logger)<<"host:www.baidu.com address:"<<addr->toString();
    }else{
        XZMJX_LOG_ERROR(g_logger)<<"host:www.baidu.com address: failed";
    }
    xzmjx::Socket::ptr sock = xzmjx::Socket::CreateTCP(addr);
    addr->setPort(8089);
    XZMJX_LOG_INFO(g_logger)<<"addr="<<addr->toString();
    int rt = sock->connect(addr);
    if(!rt){
        XZMJX_LOG_ERROR(g_logger)<<"connect "<<addr->toString()<<" failed";
    }else{
        XZMJX_LOG_INFO(g_logger)<<"connect "<<addr->toString()<<" connected";
    }
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    rt = sock->send(buff,sizeof(buff));
    if(rt<=0){
        XZMJX_LOG_INFO(g_logger)<<"send fail rt="<<rt;
        return;
    }
    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0],buffs.size());
    if(rt<=0){
        XZMJX_LOG_INFO(g_logger)<<"recv fail rt="<<rt;
        return;
    }
    XZMJX_LOG_INFO(g_logger)<<rt;
    buffs.resize(rt);
    XZMJX_LOG_INFO(g_logger)<<buffs;
}


int main(){


    xzmjx::IOManager iom(1);
    iom.submit(std::bind(test_socket));
    return 0;
}
