//
// Created by 20132 on 2022/5/7.
//

#ifndef XZMJX_SERVLET_H
#define XZMJX_SERVLET_H
#include "http/http_session.h"
#include "http/http.h"
#include "mutex.h"
#include <memory>
#include <string>
#include <functional>

namespace xzmjx{
namespace http{
class Servlet{
public:
    typedef std::shared_ptr<Servlet> ptr;

    Servlet(std::string name);
    virtual ~Servlet() {}
    virtual int32_t handle(xzmjx::http::HttpRequest::ptr req
                           ,xzmjx::http::HttpResponse::ptr rsp
                           ,xzmjx::http::HttpSession::ptr session) = 0;
    const std::string& name() const { return m_name; }

protected:
    std::string m_name;
};

class FunctionServlet : public Servlet{
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    typedef std::function<int32_t(xzmjx::http::HttpRequest::ptr req
                                  ,xzmjx::http::HttpResponse::ptr rsp
                                  ,xzmjx::http::HttpSession::ptr session) > callback;
    FunctionServlet(callback cb);
    int32_t handle(xzmjx::http::HttpRequest::ptr req
                   ,xzmjx::http::HttpResponse::ptr rsp
                   ,xzmjx::http::HttpSession::ptr session) override;
private:
    callback m_cb;
};

class NotFoundServlet : public Servlet{
public:
    typedef std::shared_ptr<FunctionServlet> ptr;
    NotFoundServlet(const std::string& name);
    int32_t handle(xzmjx::http::HttpRequest::ptr req
                ,xzmjx::http::HttpResponse::ptr rsp
                ,xzmjx::http::HttpSession::ptr session) override;
private:
    std::string m_content;
    std::string m_name;
};

class ServletDispatch : public Servlet{
public:
    typedef std::shared_ptr<ServletDispatch> ptr;
    typedef RWMutex RWMutexType;

    ServletDispatch();
    int32_t handle(xzmjx::http::HttpRequest::ptr req
                   ,xzmjx::http::HttpResponse::ptr rsp
                   ,xzmjx::http::HttpSession::ptr session) override;

    void addServlet(const std::string& uri,Servlet::ptr slt);
    void addServlet(const std::string& uri,FunctionServlet::callback cb);
    void addGlobServlet(const std::string& uri,Servlet::ptr slt);
    void addGlobServlet(const std::string& uri,FunctionServlet::callback cb);

    void delServlet(const std::string& uri);
    void delGlobServlet(const std::string& uri);

    Servlet::ptr getServlet(const std::string& uri);
    Servlet::ptr getGlobServlet(const std::string& uri);
    Servlet::ptr getMatchedServlet(const std::string& uri);

    void setDefaultServlet(Servlet::ptr slt) { m_default = slt; }
    Servlet::ptr getDefaultServlet() const { return m_default; }

    void listAllServlet(std::map<std::string, Servlet::ptr>& infos);
    void listAllGlobServlet(std::map<std::string, Servlet::ptr>& infos);

protected:
    RWMutexType m_rwlock;
    std::unordered_map<std::string,Servlet::ptr> m_datas;
    std::vector<std::pair<std::string,Servlet::ptr>> m_globs;
    Servlet::ptr m_default;
};


}
}


#endif //XZMJX_SERVLET_H
