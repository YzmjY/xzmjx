//
// Created by 20132 on 2022/5/10.
//
#include "http/http_server.h"
#include "log.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_ROOT();



xzmjx::IOManager::ptr worker;
void run() {
    g_logger->setLevel(xzmjx::LogLevel::INFO);
    //sylar::http::HttpServer::ptr server(new sylar::http::HttpServer(true, worker.get(), sylar::IOManager::GetThis()));
    xzmjx::http::HttpServer::ptr server(new xzmjx::http::HttpServer(true));
    xzmjx::Address::ptr addr = xzmjx::Address::LookupAnyIPAddress("0.0.0.0:8020");
    while(!server->bind(addr)) {
        sleep(2);
    }

    server->start();
}

int main(int argc, char** argv) {
    xzmjx::IOManager iom(1, "main");
    worker.reset(new xzmjx::IOManager(3, "worker"));
    iom.submit(run);
    return 0;
}

