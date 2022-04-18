//
// Created by 20132 on 2022/4/18.
//
#include "log.h"
#include "iomanager.h"
#include "tcp_server.h"

xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();
void run(){
    auto addr = xzmjx::Address::LookupAny("0.0.0.0:8033");
    xzmjx::TcpServer::ptr tcp_server(new xzmjx::TcpServer);
    while(!tcp_server->bind(addr)){
        sleep(3);
    }
    tcp_server->start();
}

int main(){
    xzmjx::IOManager iom(1);
    iom.submit(run);
    iom.stop();
    return 0;
}
