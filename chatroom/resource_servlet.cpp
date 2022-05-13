#include "resource_servlet.h"
#include "log.h"
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace chat{
ResourceServlet::ResourceServlet(const std::string &path)
    : xzmjx::http::Servlet("ResourceServlet")
    ,m_path(path){

}

int32_t ResourceServlet::ResourceServlet::handle(xzmjx::http::HttpRequest::ptr req
             , xzmjx::http::HttpResponse::ptr rsp
             , xzmjx::http::HttpSession::ptr session) {
    std::string path = m_path + req->getPath();
    if(m_path.find("..") != std::string::npos){
        XZMJX_LOG_INFO(g_logger)<<"invalid path";
        return 0;
    }
    std::ifstream ifs(path);
    if(!ifs){
        rsp->setBody("invalid path");
        rsp->setStatus(xzmjx::http::HttpStatus::NOT_FOUND);
        return 0;
    }

    std::stringstream ss;
    std::string line;
    while(std::getline(ifs,line)){
        ss<<line;
    }
    rsp->setBody(ss.str());
    rsp->setHeader("content-type","text/html;charset=utf-8");
    return 0;
}
}