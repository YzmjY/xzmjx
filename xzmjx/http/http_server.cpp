//
// Created by 20132 on 2022/5/7.
//
#include "http/http_server.h"
#include "http/http_session.h"
#include "http/http.h"
#include "log.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace xzmjx {
namespace http {
HttpServer::HttpServer(bool keep_alive, IOManager* worker, IOManager* io_worker, IOManager* accept_worker)
    : TcpServer(worker, io_worker, accept_worker), m_keep_alive(keep_alive) {
    m_dispatch.reset(new ServletDispatch);
    m_type = "http";
}
void HttpServer::handleClient(Socket::ptr client) {
    XZMJX_LOG_INFO(g_logger) << "handleClient:" << client->toString();
    HttpSession::ptr session(new HttpSession(client));
    do {
        auto req = session->recvRequest();
        if (req == nullptr) {
            XZMJX_LOG_ERROR(g_logger) << "recv http request fail, errno=" << errno << " errstr=" << strerror(errno)
                                      << " cliet:" << client->toString() << " keep_alive=" << m_keep_alive;
            break;
        }
        HttpResponse::ptr rsp = req->createResponse();
        rsp->setHeader("Server", getName());
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);
        if (!m_keep_alive || req->isClose()) {
            rsp->setClose(true);
            break;
        }
    } while (true);
    session->close();
}
} // namespace http
} // namespace xzmjx
