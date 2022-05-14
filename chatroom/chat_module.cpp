#include "chat_module.h"
#include "tcp_server.h"
#include "application.h"
#include "resource_servlet.h"
#include "env.h"
#include "http/ws_server.h"
#include "chat_servlet.h"

namespace chat{
ChatModule::ChatModule()
    :xzmjx::Module("chat_room","1.0",""){

}

bool ChatModule::onLoad(){
    XZMJX_LOG_INFO(g_logger)<<"Chatroom Module Load";
    return true;
}

bool ChatModule::onUnload(){
    XZMJX_LOG_INFO(g_logger)<<"Chatroom Module Unload";
    return true;
}

bool ChatModule::onServerReady(){
    XZMJX_LOG_INFO(g_logger)<<"onServerReady";
    std::vector<xzmjx::TcpServer::ptr> srvs;
    if(!xzmjx::Application::GetInstance()->getServer("http",srvs)){
        XZMJX_LOG_INFO(g_logger)<<"no http server running";
        return false;
    }
    for(auto&& i:srvs){
        xzmjx::http::HttpServer::ptr http_server =
                std::dynamic_pointer_cast<xzmjx::http::HttpServer>(i);
        auto dispatch = http_server->getDispatch();
        chat::ResourceServlet::ptr slt(new chat::ResourceServlet(
                xzmjx::EnvMgr::GetInstance()->getCwd()));
        dispatch->addGlobServlet("/html/*",slt);
    }
    srvs.clear();
    if(!xzmjx::Application::GetInstance()->getServer("ws",srvs)){
        XZMJX_LOG_INFO(g_logger)<<"no http server running";
        return false;
    }
    for(auto&& i:srvs){
        xzmjx::http::WSServer::ptr ws_server =
                std::dynamic_pointer_cast<xzmjx::http::WSServer>(i);
        xzmjx::http::ServletDispatch::ptr dispatch = ws_server->getWSServletDispatch();
        chat::ChatServlet::ptr slt(new chat::ChatServlet());
        dispatch->addGlobServlet("/xzmjx/chat",slt);
    }
    return true;
}

bool ChatModule::onServerUp() {
    XZMJX_LOG_INFO(g_logger)<<"onServerUp";
    return true;
}
}


extern "C" {
xzmjx::Module *CreateModule() {
    xzmjx::Module *module = new chat::ChatModule();
    XZMJX_LOG_INFO(g_logger) << "CreateModule" << module->statusString();
    return module;
}


void DestoryModule(xzmjx::Module *v) {
    XZMJX_LOG_INFO(g_logger) << "DestoryModule" << v->statusString();
    delete v;
}
}