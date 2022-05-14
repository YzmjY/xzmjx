#ifndef XZMJX_CHAT_SERVLET_H
#define XZMJX_CHAT_SERVLET_H
#include "http/ws_servlet.h"
#include "chat_protocol.h"
#include "singleton.h"
#include "mutex.h"
namespace chat{
class ChatServlet : public xzmjx::http::WSServlet {
public:
    typedef std::shared_ptr<ChatServlet> ptr;
    ChatServlet();

    virtual int32_t onConnect(xzmjx::http::HttpRequest::ptr header
            ,xzmjx::http::WSSession::ptr session) override;
    virtual int32_t onClose(xzmjx::http::HttpRequest::ptr header
            ,xzmjx::http::WSSession::ptr session) override;
    virtual int32_t handle(xzmjx::http::HttpRequest::ptr header
            ,xzmjx::http::WSFrameMessage::ptr msg
            ,xzmjx::http::WSSession::ptr session) override ;
};


class ChatSessionManager{
public:
    typedef std::shared_ptr<ChatSessionManager> ptr;
    typedef xzmjx::RWMutex RWMutexType;
    ChatSessionManager();

    void add(const std::string&id,xzmjx::http::WSSession::ptr session);
    void del(const std::string&id);
    bool has(const std::string&id);

    void notifyAll(ChatMessage::ptr msg);
    void notifyOthers(ChatMessage::ptr msg,xzmjx::http::WSSession::ptr session);
    int32_t sendMsg(const std::string& id,ChatMessage::ptr msg);
    int32_t sendMsg(xzmjx::http::WSSession::ptr session,ChatMessage::ptr msg);
private:
    RWMutexType m_rwlock;
    std::map<std::string,xzmjx::http::WSSession::ptr> m_sessions;
};

typedef xzmjx::SingletonPtr<ChatSessionManager> ChatSessionMgr;
}
#endif