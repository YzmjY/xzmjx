#include "chat_servlet.h"
#include "chat_protocol.h"
#include "log.h"
xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace chat{
ChatServlet::ChatServlet()
    :xzmjx::http::WSServlet("ChatServlet"){

}

int32_t ChatServlet::onConnect(xzmjx::http::HttpRequest::ptr header
,xzmjx::http::WSSession::ptr session) {
    XZMJX_LOG_INFO(g_logger)<<"onConnect "<<session;
    return 0;
}

int32_t ChatServlet::onClose(xzmjx::http::HttpRequest::ptr header
,xzmjx::http::WSSession::ptr session) {
    auto id = header->getHeader("$id");
    XZMJX_LOG_INFO(g_logger)<<"onClose"<<session;
    if(!id.empty()){
        ChatSessionMgr::GetInstance()->del(id);
        ChatMessage::ptr notify(new ChatMessage);
        notify->set("type","user_leave");
        notify->set("time",xzmjx::Time2Str());
        notify->set("name",id);
        ChatSessionMgr::GetInstance()->notifyAll(notify);
    }
    return 0;
}

int32_t ChatServlet::handle(xzmjx::http::HttpRequest::ptr header
        ,xzmjx::http::WSFrameMessage::ptr msg
        ,xzmjx::http::WSSession::ptr session) {
    XZMJX_LOG_INFO(g_logger)<<"handle "<<session
        <<" opcode="<<msg->getOpcode()
        <<" data="<<msg->getData();
    auto chat_msg = ChatMessage::Create(msg->getData());
    auto id = header->getHeader("$id");
    if(!chat_msg){
        if(!id.empty()){
            ChatSessionMgr::GetInstance()->del(id);
        }
        return 1;
    }
    ChatMessage::ptr rsp_msg(new ChatMessage);
    auto type = chat_msg->get("type");
    if(type == "login_request"){
        //登录请求
        rsp_msg->set("type","login_response");
        auto login_id = chat_msg->get("name");
        if(login_id.empty()){
            rsp_msg->set("result","400");
            rsp_msg->set("msg","name is null");
            return ChatSessionMgr::GetInstance()->sendMsg(session,rsp_msg);
        }
        if(!id.empty()){
            rsp_msg->set("result","401");
            rsp_msg->set("msg","logined");
            return ChatSessionMgr::GetInstance()->sendMsg(session,rsp_msg);
        }
        if(ChatSessionMgr::GetInstance()->has(id)){
            rsp_msg->set("result","402");
            rsp_msg->set("msg","name exists");
            return ChatSessionMgr::GetInstance()->sendMsg(session,rsp_msg);
        }
        id = login_id;
        header->setHeader("$id",id);
        rsp_msg->set("result","200");
        rsp_msg->set("msg","ok");
        ChatSessionMgr::GetInstance()->add(id,session);

        ChatMessage::ptr nty(new ChatMessage);
        nty->set("type","user_enter");
        nty->set("time",xzmjx::Time2Str());
        nty->set("name",id);
        ChatSessionMgr::GetInstance()->notifyOthers(nty,session);

        return ChatSessionMgr::GetInstance()->sendMsg(session,rsp_msg);
    }else if(type == "send_request"){
        // 发送请求
        rsp_msg->set("type","send_response");
        auto m = chat_msg->get("msg");
        if(m.empty()){
            rsp_msg->set("result","500");
            rsp_msg->set("msg","msg is null");
            return ChatSessionMgr::GetInstance()->sendMsg(session,rsp_msg);
        }
        if(id.empty()){
            rsp_msg->set("result","501");
            rsp_msg->set("msg","not login");
            return ChatSessionMgr::GetInstance()->sendMsg(session,rsp_msg);
        }
        rsp_msg->set("result","200");
        rsp_msg->set("msg","ok");

        ChatMessage::ptr nty(new ChatMessage);
        nty->set("type","msg");
        nty->set("time",xzmjx::Time2Str());
        nty->set("name",id);
        nty->set("msg",m);
        ChatSessionMgr::GetInstance()->notifyAll(nty);

        return ChatSessionMgr::GetInstance()->sendMsg(session,rsp_msg);
    }
    return 0;
}

ChatSessionManager::ChatSessionManager(){

}

void ChatSessionManager::add(const std::string&id,xzmjx::http::WSSession::ptr session){
    XZMJX_LOG_INFO(g_logger)<<"session add, id="<<id;
    RWMutexType::WriteLock lock(m_rwlock);
    m_sessions[id] = session;
}

void ChatSessionManager::del(const std::string&id){
    XZMJX_LOG_INFO(g_logger)<<"session del, id="<<id;
    RWMutexType::WriteLock lock(m_rwlock);
    m_sessions.erase(id);
}

bool ChatSessionManager::has(const std::string&id){
    XZMJX_LOG_INFO(g_logger)<<"session exist, id="<<id;
    RWMutexType::ReadLock lock(m_rwlock);
    return !(m_sessions.find(id)==m_sessions.end());
}

void ChatSessionManager::notifyOthers(ChatMessage::ptr msg,xzmjx::http::WSSession::ptr session){
    RWMutexType::ReadLock lock(m_rwlock);
    auto sessions = m_sessions;
    lock.unlock();
    for(auto&&i:sessions){
        if(i.second == session){
            continue;
        }
        sendMsg(i.first,msg);
    }
}

void ChatSessionManager::notifyAll(ChatMessage::ptr msg){
    RWMutexType::ReadLock lock(m_rwlock);
    auto sessions = m_sessions;
    lock.unlock();
    for(auto&&i:sessions){
        sendMsg(i.first,msg);
    }
}

int32_t ChatSessionManager::sendMsg(const std::string& id,ChatMessage::ptr msg){
    RWMutexType::ReadLock lock(m_rwlock);
    auto iter = m_sessions.find(id);
    if(iter == m_sessions.end()){
        return 0;
    }
    lock.unlock();
    return iter->second->sendMessage(msg->toString())<=0;
}
int32_t ChatSessionManager::sendMsg(xzmjx::http::WSSession::ptr session,ChatMessage::ptr msg){
    return session->sendMessage(msg->toString())<=0;
}
}