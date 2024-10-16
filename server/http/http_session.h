#pragma once

#include "server/stream.h"
#include "server/socket_stream.h"
#include "http.h"

#include <memory>

namespace shuai
{
namespace http
{

class HttpSession: public SocketStream
{
public:
    typedef std::shared_ptr<HttpSession> ptr;

    HttpSession() = default;
    HttpSession(Socket::ptr sock, bool owner = true);
    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);

};

}
}