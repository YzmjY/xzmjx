//
// Created by 20132 on 2022/5/7.
//

#ifndef XZMJX_HTTP_SERVER_H
#define XZMJX_HTTP_SERVER_H
#include "tcp_server.h"

namespace xzmjx{
namespace http{
class HttpServer:public TcpServer{
public:
    typedef std::shared_ptr<HttpServer> ptr;
    explicit HttpServer(bool keep_alive = false,
            IOManager* worker = IOManager::Self(),
            IOManager* io_worker = IOManager::Self(),
            IOManager* accept_worker = IOManager::Self());

protected:
    virtual void handleClient(Socket::ptr client) override;


private:
    bool m_keep_alive;
};
}
}
#endif //XZMJX_HTTP_SERVER_H
