//
// Created by 20132 on 2022/4/19.
//
#include "http/http.h"

namespace xzmjx{
namespace http{

HttpMethod StringToHttpMethod(const std::string& m){
#define XX(num,name,decription)              \
    if(strcmp(m.cstr(),#decription) == 0) {  \
        return HttpMethod::name;             \
    }
    HTTP_METHOD_MAP(XX);
#undef XX
    return  HttpMethod::INVAILD_METHOD;
}

static const char* s_method_string[] = {
#define XX(num, name, string) #string,
        HTTP_METHOD_MAP(XX)
#undef XX
};

const std::string HttpMethodToString(const HttpMethod& m){
    switch(m) {
#define XX(num,name,decription) \
        case HttpMethod::name: \
            return #decription;
        HTTP_METHOD_MAP(XX);
#undef XX
        default:
            return "<unknown>";
    }
}

const std::string HttpStatusToString(const HttpStatus& s){
    switch(s) {
#define XX(code, name, msg) \
        case HttpStatus::name: \
            return #msg;
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
            return "<unknown>";
    }
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    :m_method(HttpMethod::GET)
    ,m_version(version)
    ,m_close(close)
    ,m_parserParamFlag(0)
    ,m_path("/"){
}

std::shared_ptr<HttpResponse> HttpRequest::createResponse(){

}

std::string HttpRequest::getHeader(const std::string& key, const std::string& def) const {
    auto iter = m_headers.find(key);
    return iter == m_headers.end()?def:iter->second;
}

std::string HttpRequest::getParam(const std::string& key, const std::string& def){
    initBodyParam();
    initQueryParam();
    auto iter = m_params.find(key);
    return iter == m_params.end()?def:iter->second;
}

std::string HttpRequest::getCookie(const std::string &key, const std::string &def) {
    initCookies();
    auto iter = m_cookies.find(key);
    return iter == m_cookies.end()?def:iter->second;
}

void HttpRequest::setHeader(const std::string& key, const std::string& val){
    m_headers[key] = val;
}

void HttpRequest::setParam(const std::string& key, const std::string& val){
    m_params[key] = val;
}

void HttpRequest::setCookie(const std::string& key, const std::string& val){
    m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string& key){
    m_headers.erase(key);
}

void HttpRequest::delParam(const std::string& key){
    m_params.erase(key);
}

void HttpRequest::delCookie(const std::string& key){
    m_cookies.erase(key);
}

bool HttpRequest::hasHeader(const std::string& key, std::string* val ){
    auto iter = m_headers.find(key);
    if(iter == m_headers.end()){
        return false;
    }
    if(val){
        *val = iter->second;
    }
    return true;
}

bool HttpRequest::hasParam(const std::string& key, std::string* val ){
    auto iter = m_params.find(key);
    if(iter == m_params.end()){
        return false;
    }
    if(val){
        *val = iter->second;
    }
    return true;
}

bool HttpRequest::hasCookie(const std::string& key, std::string* val ){
    auto iter = m_cookies.find(key);
    if(iter == m_cookies.end()){
        return false;
    }
    if(val){
        *val = iter->second;
    }
    return true;
}



void HttpRequest::initQueryParam() {

}

void HttpRequest::initBodyParam() {

}

void HttpRequest::initCookies() {

}

void HttpRequest::initParam() {

}

void HttpRequest::init() {

}







}
}