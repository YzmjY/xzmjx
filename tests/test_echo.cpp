//
// Created by 20132 on 2022/4/21.
//
#include "example/echo_server.h"
#include "log.h"
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("root");
void echo(){
    xzmjx::EchoServer::ptr server(new xzmjx::EchoServer());
    auto addr = xzmjx::Address::LookupAny("0.0.0.0:8033");
    server->bind(addr);
    server->start();

    XZMJX_LOG_INFO(g_logger)<<"echo end";
}


int main(){
    xzmjx::IOManager iom(2);
    iom.submit(echo);
    iom.stop();

    XZMJX_LOG_INFO(g_logger)<<"main end";

    return 0;
}

