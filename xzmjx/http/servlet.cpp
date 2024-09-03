//
// Created by 20132 on 2022/5/7.
//
#include "http/servlet.h"
#include "log.h"
#include <fnmatch.h>
namespace xzmjx {
namespace http {
Servlet::Servlet(std::string name) : m_name(name) {}

FunctionServlet::FunctionServlet(callback cb) : Servlet("FunctionServlet"), m_cb(cb) {}
int32_t FunctionServlet::handle(xzmjx::http::HttpRequest::ptr req, xzmjx::http::HttpResponse::ptr rsp,
                                xzmjx::http::HttpSession::ptr session) {
    if (m_cb) {
        return m_cb(req, rsp, session);
    }
    return -1;
}

NotFoundServlet::NotFoundServlet(const std::string& name) : Servlet("NotFoundServlet"), m_name(name) {
    m_content = "<html><head><title>404 Not Found"
                "</title></head><body><center><h1>404 Not Found</h1></center>"
                "<hr><center>" +
                name + "</center></body></html>";
}
int32_t NotFoundServlet::handle(xzmjx::http::HttpRequest::ptr req, xzmjx::http::HttpResponse::ptr rsp,
                                xzmjx::http::HttpSession::ptr session) {
    rsp->setStatus(xzmjx::http::HttpStatus::NOT_FOUND);
    rsp->setHeader("Server", "xzmjx/1.0.0");
    rsp->setHeader("Content-Type", "text/html");
    rsp->setBody(m_content);
    return 0;
}

ServletDispatch::ServletDispatch() : Servlet("ServletDispatch") { m_default.reset(new NotFoundServlet("xzmjx/1.0")); }
int32_t ServletDispatch::handle(xzmjx::http::HttpRequest::ptr req, xzmjx::http::HttpResponse::ptr rsp,
                                xzmjx::http::HttpSession::ptr session) {
    auto slt = getMatchedServlet(req->getPath());
    if (slt) {
        slt->handle(req, rsp, session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string& uri, Servlet::ptr slt) {
    RWMutex::WriteLock lock(m_rwlock);
    m_datas[uri] = slt;
}

void ServletDispatch::addServlet(const std::string& uri, FunctionServlet::callback cb) {
    RWMutex::WriteLock lock(m_rwlock);
    m_datas[uri] = Servlet::ptr(new FunctionServlet(cb));
}

void ServletDispatch::addGlobServlet(const std::string& uri, Servlet::ptr slt) {
    RWMutex::WriteLock lock(m_rwlock);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
        if (it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri, slt));
}

void ServletDispatch::addGlobServlet(const std::string& uri, FunctionServlet::callback cb) {
    RWMutex::WriteLock lock(m_rwlock);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
        if (it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
    m_globs.push_back(std::make_pair(uri, Servlet::ptr(new FunctionServlet(cb))));
}

void ServletDispatch::delServlet(const std::string& uri) {
    RWMutex::WriteLock lock(m_rwlock);
    m_datas.erase(uri);
}

void ServletDispatch::delGlobServlet(const std::string& uri) {
    RWMutex::WriteLock lock(m_rwlock);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
        if (it->first == uri) {
            m_globs.erase(it);
            break;
        }
    }
}

Servlet::ptr ServletDispatch::getServlet(const std::string& uri) {
    RWMutex::ReadLock lock(m_rwlock);
    auto it = m_datas.find(uri);
    return it == m_datas.end() ? nullptr : it->second;
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string& uri) {
    RWMutex::ReadLock lock(m_rwlock);
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
        if (it->first == uri) {
            return it->second;
        }
    }
    return nullptr;
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string& uri) {
    RWMutex::ReadLock lock(m_rwlock);
    auto mit = m_datas.find(uri);
    if (mit != m_datas.end()) {
        return mit->second;
    }
    for (auto it = m_globs.begin(); it != m_globs.end(); ++it) {
        if (!fnmatch(it->first.c_str(), uri.c_str(), 0)) {
            return it->second;
        }
    }
    return m_default;
}

void ServletDispatch::listAllServlet(std::map<std::string, Servlet::ptr>& infos) {
    RWMutex::ReadLock lock(m_rwlock);
    for (auto&& it : m_datas) {
        infos[it.first] = it.second;
    }
}

void ServletDispatch::listAllGlobServlet(std::map<std::string, Servlet::ptr>& infos) {
    RWMutex::ReadLock lock(m_rwlock);
    for (auto&& it : m_globs) {
        infos[it.first] = it.second;
    }
}

} // namespace http
} // namespace xzmjx
