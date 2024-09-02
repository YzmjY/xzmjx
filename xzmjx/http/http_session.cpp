//
// Created by 20132 on 2022/5/7.
//
#include "http/http_session.h"
#include "log.h"
#include "http/http_parser.h"

static xzmjx::Logger::ptr g_logger = XZMJX_LOG_NAME("system");
namespace xzmjx{
namespace http{
HttpSession::HttpSession(Socket::ptr socket,bool owner)
    : SocketStream(socket,owner){

}

HttpRequest::ptr HttpSession::recvRequest(){
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    char* buf = (char*)malloc(buff_size);
    uint64_t offset = 0; // buf中现有数据量
    do{
        int len = read(buf+offset,buff_size-offset);
        if(len<=0){
            close();
            return nullptr;
        }
        len+=offset;
        uint64_t n_parser = parser->execute(buf,len);
        if(parser->hasError()){
            close();
            return nullptr;
        }
        offset = len - n_parser;
        if(offset>=buff_size){
            close();
            return nullptr;
        }
        if(parser->isFinish()){
            break;
        }
    }while(true);
    HttpRequest::ptr req = parser->getData();
    req->init();
    free(buf);
    return req;
}

int HttpSession::sendResponse(HttpResponse::ptr rsp){
    std::stringstream ss;
    rsp->dump(ss);
    std::string data = ss.str();
    return writeFixedSize(&data[0],data.size());
}
}
}
