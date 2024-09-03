#ifndef XZMJX_WS_SERVER_H
#define XZMJX_WS_SERVER_H
#include "tcp_server.h"
#include "http/ws_session.h"
#include "http/ws_servlet.h"
namespace xzmjx {
namespace http {
class WSServer : public TcpServer {
public:
    typedef std::shared_ptr<WSServer> ptr;

    WSServer(IOManager* worker = IOManager::Self(), IOManager* io_worker = IOManager::Self(),
             IOManager* accept_worker = IOManager::Self());

    WSServletDispatch::ptr getWSServletDispatch() const { return m_ws_dispatch; }
    void setWSServletDispatch(WSServletDispatch::ptr d) { m_ws_dispatch = d; }

protected:
    void handleClient(Socket::ptr client) override;

private:
    WSServletDispatch::ptr m_ws_dispatch;
};
} // namespace http
} // namespace xzmjx

#endif