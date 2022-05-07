//
// Created by 20132 on 2022/5/7.
//

#ifndef XZMJX_HTTP_PARSER_H
#define XZMJX_HTTP_PARSER_H
#include "http_parser/http_parser.h"
#include "http/http.h"
#include <memory>
namespace xzmjx{
namespace http{
class HttpRequestParser{
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    HttpRequestParser();
    size_t execute(char* data,size_t len);
    bool isFinish();
    bool hasError();
    HttpRequest::ptr getData() const {return m_data; }
    void setError(int v) { m_error = v; }
    uint64_t getContentLength();

    const http_parser& getParser() const { return m_parser; }

public:
    static uint64_t GetHttpRequestBufferSize();

    static uint64_t GetHttpRequestMaxBodySize();
private:
    http_parser m_parser;
    HttpRequest::ptr m_data;
    int m_error;
    bool m_finished;
    std::string m_field;
};


class HttpResponseParser{
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;
    HttpResponseParser();
    size_t execute(char* data,size_t len);
    bool isFinish();
    bool hasError();
    HttpResponse::ptr getData() const {return m_data; }
    void setError(int v) { m_error = v; }
    uint64_t getContentLength();

    const http_parser& getParser() const { return m_parser; }

public:
    static uint64_t GetHttpResponseBufferSize();

    static uint64_t GetHttpResponseMaxBodySize();
private:
    http_parser m_parser;
    HttpResponse::ptr m_data;
    int m_error;
    bool m_finished;
    std::string m_field;
};
}
}
#endif //XZMJX_HTTP_PARSER_H
