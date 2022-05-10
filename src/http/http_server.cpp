//
// Created by 20132 on 2022/5/7.
//
#include "http/http_server.h"
#include "http/http_session.h"
#include "http/http.h"
#include "log.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace xzmjx{
namespace http{
HttpServer::HttpServer(bool keep_alive
                       ,IOManager* worker
                       ,IOManager* io_worker
                       ,IOManager* accept_worker)
    :TcpServer(worker,io_worker,accept_worker)
    ,m_keep_alive(keep_alive){

}
void HttpServer::handleClient(Socket::ptr client){
    XZMJX_LOG_INFO(g_logger)<<"handleClient:"<<client->toString();
    HttpSession::ptr session(new HttpSession(client));
    do{
        auto req = session->recvRequest();
        if(req == nullptr){
            XZMJX_LOG_ERROR(g_logger)<< "recv http request fail, errno="
            << errno << " errstr=" << strerror(errno)
            << " cliet:" << client->toString() << " keep_alive=" << m_keep_alive;
            break;
        }
        HttpResponse::ptr rsp = req->createResponse();


        rsp->setStatus(xzmjx::http::HttpStatus::NOT_FOUND);
        rsp->setHeader("Server", "sylar/1.0.0");
        rsp->setHeader("Content-Type", "text/html");
        rsp->setBody("<html><head><title>404 Not Found"
                     "</title></head><body><center><h1>404 Not Found</h1></center>"
                     "<hr><center>XZMJX</center></body></html>");
        rsp->appendBody("Hello XZMJX");
        session->sendResponse(rsp);
        if(!m_keep_alive||req->isClose()){
            break;
        }
    }while(true);
    session->close();
}
}
}
