#ifndef XZMJX_RESOURCE_SERVLET_H
#define XZMJX_RESOURCE_SERVLET_H
#include "http/servlet.h"
namespace chat {
class ResourceServlet : public xzmjx::http::Servlet {
public:
    typedef std::shared_ptr<ResourceServlet> ptr;

    ResourceServlet(const std::string &path);

    int32_t handle(xzmjx::http::HttpRequest::ptr req, xzmjx::http::HttpResponse::ptr rsp,
                   xzmjx::http::HttpSession::ptr session) override;

private:
    std::string m_path;
};
}

#endif