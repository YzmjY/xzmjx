//
// Created by 20132 on 2022/4/21.
//

#ifndef XZMJX_ECHO_SERVER_H
#define XZMJX_ECHO_SERVER_H
#include "tcp_server.h"
namespace xzmjx{
class EchoServer:public TcpServer{
public:
    typedef std::shared_ptr<EchoServer> ptr;
    void handleClient(Socket::ptr client) override;
};
}




#endif //XZMJX_ECHO_SERVER_H
