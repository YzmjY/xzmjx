//
// Created by 20132 on 2022/5/7.
//

#ifndef XZMJX_HTTP_SESSION_H
#define XZMJX_HTTP_SESSION_H
#include "socketstream.h"
#include "http/http.h"

namespace xzmjx{
namespace http{
class HttpSession : public SocketStream{
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr socket,bool owner = true);

    HttpRequest::ptr recvRequest();

    int sendResponse(HttpResponse::ptr rsp);
};
}
}
#endif //XZMJX_HTTP_SESSION_H
