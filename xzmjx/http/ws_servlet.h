#ifndef XZMJX_WS_SERVLET_H
#define XZMJX_WS_SERVLET_H
#include "http/servlet.h"
#include "http/ws_session.h"
namespace xzmjx{
namespace http{
class WSServlet:public Servlet{
public: 
    typedef std::shared_ptr<WSServlet> ptr;
    typedef std::function<int32_t (xzmjx::http::HttpRequest::ptr header
                              ,xzmjx::http::WSSession::ptr session)> on_connect_cb;
    typedef std::function<int32_t (xzmjx::http::HttpRequest::ptr header
                             ,xzmjx::http::WSSession::ptr session)> on_close_cb; 
    typedef std::function<int32_t (xzmjx::http::HttpRequest::ptr header
                           ,xzmjx::http::WSFrameMessage::ptr msg
                           ,xzmjx::http::WSSession::ptr session)> callback;
    WSServlet(const std::string& name);
    virtual ~WSServlet();

    virtual int32_t handle(xzmjx::http::HttpRequest::ptr req
                           ,xzmjx::http::HttpResponse::ptr rsp
                           ,xzmjx::http::HttpSession::ptr session) override{
        return 0;
    }

    virtual int32_t onConnect(xzmjx::http::HttpRequest::ptr header
                              ,xzmjx::http::WSSession::ptr session) = 0;
    virtual int32_t onClose(xzmjx::http::HttpRequest::ptr header
                              ,xzmjx::http::WSSession::ptr session) = 0;
    virtual int32_t handle(xzmjx::http::HttpRequest::ptr header
                              ,xzmjx::http::WSFrameMessage::ptr msg
                              ,xzmjx::http::WSSession::ptr session) = 0;
    const std::string& getName() const { return m_name;}
};
class FunctionWSServlet : public WSServlet {
public:
    typedef std::shared_ptr<FunctionWSServlet> ptr;
    FunctionWSServlet(callback cb
                      ,on_connect_cb connect_cb = nullptr
                      ,on_close_cb close_cb = nullptr);

    virtual int32_t onConnect(xzmjx::http::HttpRequest::ptr header
                              ,xzmjx::http::WSSession::ptr session) override;
    virtual int32_t onClose(xzmjx::http::HttpRequest::ptr header
                             ,xzmjx::http::WSSession::ptr session) override;
    virtual int32_t handle(xzmjx::http::HttpRequest::ptr header
                           ,xzmjx::http::WSFrameMessage::ptr msg
                           ,xzmjx::http::WSSession::ptr session) override;
protected:
    callback m_callback;
    on_connect_cb m_onConnect;
    on_close_cb m_onClose;
};

class WSServletDispatch:public ServletDispatch{
public:
    typedef std::shared_ptr<WSServletDispatch> ptr;

    WSServletDispatch();
    void addServlet(const std::string& uri
                    ,WSServlet::callback cb
                    ,WSServlet::on_connect_cb connect_cb = nullptr
                    ,WSServlet::on_close_cb close_cb = nullptr);
    void addGlobServlet(const std::string& uri
                    ,WSServlet::callback cb
                    ,WSServlet::on_connect_cb connect_cb = nullptr
                    ,WSServlet::on_close_cb close_cb = nullptr);
    WSServlet::ptr getWSServlet(const std::string& uri);
};
}
}

#endif
