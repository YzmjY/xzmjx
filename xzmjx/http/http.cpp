//
// Created by 20132 on 2022/4/19.
//
#include "http/http.h"
#include "util.h"

namespace xzmjx {
namespace http {

#define PARSE_PARAM(str, m, flag, trim)                                                             \
    size_t pos = 0;                                                                                 \
    do {                                                                                            \
        size_t last = pos;                                                                          \
        pos = str.find('=', pos);                                                                   \
        if (pos == std::string::npos) {                                                             \
            break;                                                                                  \
        }                                                                                           \
        size_t key = pos;                                                                           \
        pos = str.find(flag, pos);                                                                  \
        m.insert(std::make_pair(trim(str.substr(last, key - last)),                                 \
                                xzmjx::StringUtil::UrlDecode(str.substr(key + 1, pos - key - 1)))); \
        if (pos == std::string::npos) {                                                             \
            break;                                                                                  \
        }                                                                                           \
        ++pos;                                                                                      \
    } while (true);

HttpMethod StringToHttpMethod(const std::string& m) {
#define XX(num, name, decription)              \
    if (strcmp(m.c_str(), #decription) == 0) { \
        return HttpMethod::name;               \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVAILD_METHOD;
}

const std::string HttpMethodToString(const HttpMethod& m) {
    switch (m) {
#define XX(num, name, decription) \
    case HttpMethod::name: return #decription;
        HTTP_METHOD_MAP(XX);
#undef XX
        default: return "<unknown>";
    }
}

const std::string HttpStatusToString(const HttpStatus& s) {
    switch (s) {
#define XX(code, name, msg) \
    case HttpStatus::name: return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
        default: return "<unknown>";
    }
}
bool CaseInsensitiveLess::operator()(const std::string& lhs, const std::string& rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    : m_method(HttpMethod::GET),
      m_version(version),
      m_close(close),
      m_websocket(false),
      m_parserParamFlag(0),
      m_path("/") {}

std::shared_ptr<HttpResponse> HttpRequest::createResponse() {
    HttpResponse::ptr rsp = std::make_shared<HttpResponse>(getVersion(), isClose());
    return rsp;
}

std::string HttpRequest::getHeader(const std::string& key, const std::string& def) const {
    auto iter = m_headers.find(key);
    return iter == m_headers.end() ? def : iter->second;
}

std::string HttpRequest::getParam(const std::string& key, const std::string& def) {
    initBodyParam();
    initQueryParam();
    auto iter = m_params.find(key);
    return iter == m_params.end() ? def : iter->second;
}

std::string HttpRequest::getCookie(const std::string& key, const std::string& def) {
    initCookies();
    auto iter = m_cookies.find(key);
    return iter == m_cookies.end() ? def : iter->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val) { m_headers[key] = val; }

void HttpRequest::setParam(const std::string& key, const std::string& val) { m_params[key] = val; }

void HttpRequest::setCookie(const std::string& key, const std::string& val) { m_cookies[key] = val; }

void HttpRequest::delHeader(const std::string& key) { m_headers.erase(key); }

void HttpRequest::delParam(const std::string& key) { m_params.erase(key); }

void HttpRequest::delCookie(const std::string& key) { m_cookies.erase(key); }

bool HttpRequest::hasHeader(const std::string& key, std::string* val) {
    auto iter = m_headers.find(key);
    if (iter == m_headers.end()) {
        return false;
    }
    if (val) {
        *val = iter->second;
    }
    return true;
}

bool HttpRequest::hasParam(const std::string& key, std::string* val) {
    auto iter = m_params.find(key);
    if (iter == m_params.end()) {
        return false;
    }
    if (val) {
        *val = iter->second;
    }
    return true;
}

bool HttpRequest::hasCookie(const std::string& key, std::string* val) {
    auto iter = m_cookies.find(key);
    if (iter == m_cookies.end()) {
        return false;
    }
    if (val) {
        *val = iter->second;
    }
    return true;
}

void HttpRequest::initQueryParam() {
    if (m_parserParamFlag & 0x1) {
        /// 已经解析过了
        return;
    }
    PARSE_PARAM(m_query, m_params, '&', );
    m_parserParamFlag |= 0x1;
}

void HttpRequest::initBodyParam() {
    if (m_parserParamFlag & 0x2) {
        return;
    }
    std::string content_type = getHeader("content-type");
    if (strcasestr(content_type.c_str(), "application/x-www-form-urlencoded") == nullptr) {
        m_parserParamFlag |= 0x2;
        return;
    }
    PARSE_PARAM(m_body, m_params, '&', );
    m_parserParamFlag |= 0x2;
}

void HttpRequest::initCookies() {
    if (m_parserParamFlag & 0x4) {
        return;
    }
    std::string cookie = getHeader("cookie");
    if (cookie.empty()) {
        m_parserParamFlag |= 0x4;
        return;
    }
    PARSE_PARAM(cookie, m_cookies, ';', xzmjx::StringUtil::Trim);
    m_parserParamFlag |= 0x4;
}

void HttpRequest::initParam() {
    initQueryParam();
    initBodyParam();
    initCookies();
}

void HttpRequest::init() {
    std::string conn = getHeader("connection");
    if (!conn.empty()) {
        if (strcasecmp(conn.c_str(), "keep-alive") == 0) {
            m_close = false;
        } else {
            m_close = true;
        }
    }
}

std::ostream& HttpRequest::dump(std::ostream& os) const {
    os << HttpMethodToString(m_method) << " " << m_path << ((m_query.empty()) ? "" : "?") << m_query
       << (m_fragment.empty() ? "" : "#") << m_fragment << " HTTP/" << ((uint32_t)(m_version >> 4)) << "."
       << ((uint32_t)(m_version & 0x0f)) << "\r\n";
    if (!m_websocket) {
        os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }

    for (auto&& header : m_headers) {
        if (!m_websocket && strcasecmp(header.first.c_str(), "connection") == 0) {
            continue;
        }
        os << header.first << ": " << header.second << "\r\n";
    }

    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}

std::string HttpRequest::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

HttpResponse::HttpResponse(uint8_t version, bool close)
    : m_status(HttpStatus::OK), m_version(version), m_close(close), m_websocket(false) {}

std::string HttpResponse::getHeader(const std::string& key, const std::string& def) {
    auto iter = m_header.find(key);
    return iter == m_header.end() ? def : iter->second;
}

void HttpResponse::setHeader(const std::string& key, const std::string& val) { m_header[key] = val; }

void HttpResponse::delHeader(const std::string& key) { m_header.erase(key); }

void HttpResponse::setRedirect(const std::string& uri) {
    m_status = HttpStatus::FOUND;
    setHeader("Location", uri);
}
void HttpResponse::setCookie(const std::string& key, const std::string& val, time_t expired, const std::string& path,
                             const std::string& domain, bool secure) {
    std::stringstream ss;
    ss << key << "=" << val;
    if (expired > 0) {
        ss << ";expires=" << xzmjx::Time2Str(expired, "%a,%d %b %Y %H:%M:%S");
    }
    if (!domain.empty()) {
        ss << ";domain=" << domain;
    }
    if (!path.empty()) {
        ss << ";path=" << path;
    }
    if (secure) {
        ss << ";secure";
    }
    m_cookies.push_back(ss.str());
}

std::ostream& HttpResponse::dump(std::ostream& os) const {
    os << "HTTP/" << ((uint32_t)(m_version >> 4)) << "." << ((uint32_t)(m_version & 0x0F)) << " " << (uint32_t)m_status
       << " " << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason) << "\r\n";
    for (auto& i : m_header) {
        if (!m_websocket && strcasecmp(i.first.c_str(), "connection") == 0) {
            continue;
        }
        os << i.first << ": " << i.second << "\r\n";
    }
    for (auto& i : m_cookies) {
        os << "Set-Cookie: " << i << "\r\n";
    }
    if (!m_websocket) {
        os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    }

    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n" << m_body;
    } else {
        os << "\r\n";
    }
    return os;
}
std::string HttpResponse::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& req) { return req.dump(os); }
std::ostream& operator<<(std::ostream& os, const HttpResponse& rsp) { return rsp.dump(os); }

} // namespace http
} // namespace xzmjx