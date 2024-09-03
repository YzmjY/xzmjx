#include "http/ws_server.h"
#include "log.h"
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace xzmjx {
namespace http {
WSServer::WSServer(IOManager* worker, IOManager* io_worker, IOManager* accept_worker)
    : TcpServer(worker, io_worker, accept_worker) {
    m_ws_dispatch.reset(new WSServletDispatch);
    m_type = "websocket_server";
}

void WSServer::handleClient(Socket::ptr client) {
    XZMJX_LOG_INFO(g_logger) << "handleClient:" << client->toString();
    WSSession::ptr session(new WSSession(client));
    do {
        // 建立连接，握手阶段
        HttpRequest::ptr header = session->handleShake();
        if (!header) {
            XZMJX_LOG_DEBUG(g_logger) << "handshake failed";
            break;
        }

        WSServlet::ptr slt = m_ws_dispatch->getWSServlet(header->getPath());
        if (!slt) {
            XZMJX_LOG_DEBUG(g_logger) << "no match WSServlet";
            break;
        }
        int rt = slt->onConnect(header, session);
        if (rt) {
            XZMJX_LOG_DEBUG(g_logger) << "onConnect return " << rt;
            break;
        }

        // 连接建立
        while (true) {
            auto msg = session->recvMessage();
            if (!msg) {
                break;
            }
            rt = slt->handle(header, msg, session);
            if (rt) {
                XZMJX_LOG_DEBUG(g_logger) << "handle return " << rt;
                break;
            }
        }
        slt->onClose(header, session);
    } while (0);
    session->close();
}
} // namespace http
} // namespace xzmjx