//
// Created by 20132 on 2022/5/7.
//

#ifndef XZMJX_HTTP_SERVER_H
#define XZMJX_HTTP_SERVER_H
#include "tcp_server.h"
#include "http/servlet.h"

namespace xzmjx{
namespace http{
class HttpServer:public TcpServer{
public:
    typedef std::shared_ptr<HttpServer> ptr;
    explicit HttpServer(bool keep_alive = false,
            IOManager* worker = IOManager::Self(),
            IOManager* io_worker = IOManager::Self(),
            IOManager* accept_worker = IOManager::Self());
    ServletDispatch::ptr getDispatch() const { return m_dispatch; }
protected:
    virtual void handleClient(Socket::ptr client) override;


private:
    bool m_keep_alive;
    ServletDispatch::ptr m_dispatch;
};
}
}
#endif //XZMJX_HTTP_SERVER_H
