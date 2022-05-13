//
// Created by 20132 on 2022/4/21.
//
#include "example/echo_server.h"
#include "bytearray.h"
#include "log.h"
namespace xzmjx{
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("echo_server");
void EchoServer::handleClient(Socket::ptr client){
    XZMJX_LOG_INFO(g_logger)<<"handleClient";
    while(true){
        char buf[1024];
        int n = client->recv(buf,1024);
        XZMJX_LOG_INFO(g_logger)<<"client:"<<client->toString()<<"data: "<<buf;
        if(n == 0){
            break;
        }
        client->send(buf,n);
    }
    client->close();
}
}

