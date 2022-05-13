//
// Created by 20132 on 2022/3/27.
//
#include "iomanager.h"
#include "log.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <cstring>

xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");

int sock = 0;

void test_fiber() {
    XZMJX_LOG_ERROR(g_logger) << "test_fiber sock=" << sock;

    //sleep(3);

    //close(sock);
    //sylar::IOManager::Self()->cancelAll(sock);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "220.181.38.251", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if(errno == EINPROGRESS) {
        XZMJX_LOG_ERROR(g_logger) << "add event errno=" << errno << " " << strerror(errno);
        xzmjx::IOManager::Self()->addEvent(sock, xzmjx::IOManager::Event_READ, [](){
            XZMJX_LOG_ERROR(g_logger) << "read callback";
        });
        xzmjx::IOManager::Self()->addEvent(sock, xzmjx::IOManager::Event_WRITE, [](){
            XZMJX_LOG_ERROR(g_logger) << "write callback";
            //close(sock);
            xzmjx::IOManager::Self()->cancelEvent(sock, xzmjx::IOManager::Event_READ);
            close(sock);
        });
    } else {
        XZMJX_LOG_ERROR(g_logger) << "else " << errno << " " << strerror(errno);
    }

}

void test1() {
    std::cout << "EPOLLIN=" << EPOLLIN
              << " EPOLLOUT=" << EPOLLOUT << std::endl;
    xzmjx::IOManager iom(2);
    iom.submit(&test_fiber);
    iom.stop();
}

int main(){
    g_logger->setLevel(xzmjx::LogLevel::DEBUG);
    test1();
    return 0;
}
