//
// Created by 20132 on 2022/5/7.
//
#include "http/http_parser.h"
#include "log.h"
#include "config.h"
namespace xzmjx {
namespace http {
static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");

static xzmjx::ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    xzmjx::Config::Lookup("http.request.buffer_size", (uint64_t)4 * 1024, "http request buffer");

static xzmjx::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    xzmjx::Config::Lookup("http.request.max_body_size", (uint64_t)4 * 1024 * 1024, "http request max_body_size");

static xzmjx::ConfigVar<uint64_t>::ptr g_http_response_buffer_size =
    xzmjx::Config::Lookup("http.response.buffer_size", (uint64_t)4 * 1024, "http response buffer");

static xzmjx::ConfigVar<uint64_t>::ptr g_http_response_max_body_size =
    xzmjx::Config::Lookup("http.response.max_body_size", (uint64_t)4 * 1024 * 1024, "http response max_body_size");

static uint64_t s_http_request_buffer_size = 0;
static uint64_t s_http_request_max_body_size = 0;
static uint64_t s_http_response_buffer_size = 0;
static uint64_t s_http_response_max_body_size = 0;

namespace {
struct HttpParserIniter {
    HttpParserIniter() {
        s_http_request_buffer_size = g_http_request_buffer_size->getValue();
        s_http_request_max_body_size = g_http_request_max_body_size->getValue();
        s_http_response_buffer_size = g_http_response_buffer_size->getValue();
        s_http_response_max_body_size = g_http_response_max_body_size->getValue();

        g_http_request_buffer_size->addListener(
            [](const uint64_t& old_val, const uint64_t& new_val) { s_http_request_buffer_size = new_val; });

        g_http_request_max_body_size->addListener(
            [](const uint64_t& old_val, const uint64_t& new_val) { s_http_request_max_body_size = new_val; });

        g_http_response_buffer_size->addListener(
            [](const uint64_t& old_val, const uint64_t& new_val) { s_http_response_buffer_size = new_val; });

        g_http_response_max_body_size->addListener(
            [](const uint64_t& old_val, const uint64_t& new_val) { s_http_response_max_body_size = new_val; });
    }
};
HttpParserIniter g_http_parser_initer;
} // namespace

uint64_t HttpRequestParser::GetHttpRequestBufferSize() { return s_http_request_buffer_size; }

uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() { return s_http_request_max_body_size; }

uint64_t HttpResponseParser::GetHttpResponseBufferSize() { return s_http_response_buffer_size; }

uint64_t HttpResponseParser::GetHttpResponseMaxBodySize() { return s_http_response_max_body_size; }

static int on_request_message_begin_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_request_message_begin_cb";
    return 0;
}

static int on_request_url_cb(http_parser* p, const char* buf, size_t length) {
    /*
     foo://user@sylar.com:8042/over/there?name=ferret#nose
       \_/   \______________/\_________/ \_________/ \__/
        |           |            |            |        |
     scheme     authority       path        query   fragment
    */
    XZMJX_LOG_DEBUG(g_logger) << "on_request_url_cb";
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    int ret;
    struct http_parser_url url_parser;
    http_parser_url_init(&url_parser);
    ret = http_parser_parse_url(buf, length, 0, &url_parser);
    if (ret != 0) {
        XZMJX_LOG_DEBUG(g_logger) << "parser url failed";
        return 1;
    }
    if (url_parser.field_set & (1 << UF_PATH)) {
        parser->getData()->setPath(
            std::string(buf + url_parser.field_data[UF_PATH].off, url_parser.field_data[UF_PATH].len));
    }

    if (url_parser.field_set & (1 << UF_QUERY)) {
        parser->getData()->setQuery(
            std::string(buf + url_parser.field_data[UF_QUERY].off, url_parser.field_data[UF_QUERY].len));
    }

    if (url_parser.field_set & (1 << UF_FRAGMENT)) {
        parser->getData()->setFragment(
            std::string(buf + url_parser.field_data[UF_FRAGMENT].off, url_parser.field_data[UF_FRAGMENT].len));
    }

    return 0;
}

static int on_request_status_cb(http_parser* p, const char* at, size_t length) {
    XZMJX_LOG_DEBUG(g_logger) << "on_status_cb";
    return 0;
}

static int on_request_header_field_cb(http_parser* p, const char* at, size_t length) {
    std::string field(at, length);
    XZMJX_LOG_DEBUG(g_logger) << "on_request_header_field_cb,field = " << field;
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->setField(field);
    return 0;
}

static int on_request_header_value_cb(http_parser* p, const char* at, size_t length) {
    std::string value(at, length);
    XZMJX_LOG_DEBUG(g_logger) << "on_request_header_value_cb,field = " << value;
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->getData()->setHeader(parser->getField(), value);
    return 0;
}

static int on_request_header_complete_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_request_header_complete_cb";
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->getData()->setVersion(p->http_major << 4 | p->http_minor);
    parser->getData()->setMethod((HttpMethod)p->method);
    return 0;
}

static int on_request_body_cb(http_parser* p, const char* at, size_t length) {
    std::string body(at, length);
    XZMJX_LOG_DEBUG(g_logger) << "on_request_body_cb,field = " << body;
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->getData()->appendBody(body);
    return 0;
}

static int on_request_message_complete_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_request_message_complete_cb";
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->setFinish(true);
    return 0;
}

static int on_request_chunk_header_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_request_chunk_header_cb";
    return 0;
}

static int on_request_chunk_complete_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_request_chunk_complete_cb";
    return 0;
}

static http_parser_settings s_request_settings = {.on_message_begin = on_request_message_begin_cb,
                                                  .on_url = on_request_url_cb,
                                                  .on_status = on_request_status_cb,
                                                  .on_header_field = on_request_header_field_cb,
                                                  .on_header_value = on_request_header_value_cb,
                                                  .on_headers_complete = on_request_header_complete_cb,
                                                  .on_body = on_request_body_cb,
                                                  .on_message_complete = on_request_message_complete_cb,
                                                  .on_chunk_header = on_request_chunk_header_cb,
                                                  .on_chunk_complete = on_request_chunk_complete_cb};

HttpRequestParser::HttpRequestParser() {
    http_parser_init(&m_parser, HTTP_REQUEST);
    m_data.reset(new HttpRequest);
    m_parser.data = this;
    m_error = 0;
    m_finished = false;
}

uint64_t HttpRequestParser::getContentLength() { return m_parser.content_length; }

size_t HttpRequestParser::execute(char* data, size_t len) {
    size_t n_parsered = http_parser_execute(&m_parser, &s_request_settings, data, len);
    if (m_parser.upgrade) {
        XZMJX_LOG_DEBUG(g_logger) << "found upgrade,ignore";
    } else if (m_parser.http_errno != 0) {
        XZMJX_LOG_DEBUG(g_logger) << "parser http request failed :"
                                  << http_errno_description(HTTP_PARSER_ERRNO(&m_parser));
        setError(m_parser.http_errno);
    } else {
        if (n_parsered < len) {
            memmove(data, data + n_parsered, len - n_parsered);
        } else {
            memset(data, 0, len);
        }
    }
    return n_parsered;
}

static int on_response_message_begin_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_response_message_begin_cb";
    return 0;
}

static int on_response_url_cb(http_parser* p, const char* buf, size_t length) {
    XZMJX_LOG_DEBUG(g_logger) << "on_response_url_cb,never reached";
    return 0;
}

static int on_response_status_cb(http_parser* p, const char* at, size_t length) {
    XZMJX_LOG_DEBUG(g_logger) << "on_response_status_cb";
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
    parser->getData()->setStatus(HttpStatus(p->status_code));
    return 0;
}

static int on_response_header_field_cb(http_parser* p, const char* at, size_t length) {
    std::string field(at, length);
    XZMJX_LOG_DEBUG(g_logger) << "on_response_header_field_cb,field = " << field;
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
    parser->setField(field);
    return 0;
}

static int on_response_header_value_cb(http_parser* p, const char* at, size_t length) {
    std::string value(at, length);
    XZMJX_LOG_DEBUG(g_logger) << "on_response_header_value_cb,field = " << value;
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
    parser->getData()->setHeader(parser->getField(), value);
    return 0;
}

static int on_response_header_complete_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_response_header_complete_cb";
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
    parser->getData()->setVersion(p->http_major << 4 | p->http_minor);
    parser->getData()->setStatus(HttpStatus(p->status_code));
    return 0;
}

static int on_response_body_cb(http_parser* p, const char* at, size_t length) {
    std::string body(at, length);
    XZMJX_LOG_DEBUG(g_logger) << "on_response_body_cb,field = " << body;
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(p->data);
    parser->getData()->appendBody(body);
    return 0;
}

static int on_response_message_complete_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_response_message_complete_cb";
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(p->data);
    parser->setFinish(true);
    return 0;
}

static int on_response_chunk_header_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_response_chunk_header_cb";
    return 0;
}

static int on_response_chunk_complete_cb(http_parser* p) {
    XZMJX_LOG_DEBUG(g_logger) << "on_response_chunk_complete_cb";
    return 0;
}

static http_parser_settings s_response_settings = {.on_message_begin = on_response_message_begin_cb,
                                                   .on_url = on_response_url_cb,
                                                   .on_status = on_response_status_cb,
                                                   .on_header_field = on_response_header_field_cb,
                                                   .on_header_value = on_response_header_value_cb,
                                                   .on_headers_complete = on_response_header_complete_cb,
                                                   .on_body = on_response_body_cb,
                                                   .on_message_complete = on_response_message_complete_cb,
                                                   .on_chunk_header = on_response_chunk_header_cb,
                                                   .on_chunk_complete = on_response_chunk_complete_cb};

HttpResponseParser::HttpResponseParser() {
    http_parser_init(&m_parser, HTTP_RESPONSE);
    m_data.reset(new HttpResponse);
    m_parser.data = this;
    m_error = 0;
    m_finished = false;
}
uint64_t HttpResponseParser::getContentLength() { return m_parser.content_length; }

size_t HttpResponseParser::execute(char* data, size_t len) {
    size_t n_parsered = http_parser_execute(&m_parser, &s_response_settings, data, len);
    if (m_parser.upgrade) {
        XZMJX_LOG_DEBUG(g_logger) << "found upgrade,ignore";
    } else if (m_parser.http_errno != 0) {
        XZMJX_LOG_DEBUG(g_logger) << "parser http response failed :"
                                  << http_errno_description(HTTP_PARSER_ERRNO(&m_parser));
        setError(m_parser.http_errno);
    } else {
        if (n_parsered < len) {
            memmove(data, data + n_parsered, len - n_parsered);
        } else {
            memset(data, 0, len);
        }
    }
    return n_parsered;
}

} // namespace http
} // namespace xzmjx
