#include "http_session.h"
#include "http_parser.h"

namespace shuai
{
namespace http
{
HttpSession::HttpSession(Socket::ptr sock, bool owner)
    :SocketStream(sock, owner)
{}

HttpRequest::ptr HttpSession::recvRequest()
{
    HttpRequestParser::ptr parser(new HttpRequestParser);
    uint64_t buffer_size = HttpRequestParser::GetHttpRequestBufferSize();
    // uint64_t buffer_size = 150;
    std::shared_ptr<char> buffer(
        new char[buffer_size], [](char* ptr){
            delete []ptr;
        });

    char* data = buffer.get();
    int offset = 0;
    do
    {
        int len = read(data, buffer_size - offset);
        if(len <= 0) return nullptr;

        len += offset;
        size_t nparse = parser->execute(data, len);
        if(parser->hasError()) return nullptr;
        offset = len - nparse;
        if(offset == (int)buffer_size) return nullptr;

        if(parser->isFinished()) 
            break;
    
    }while(true);
    int64_t length = parser->getContentLength();
    if(length > 0)
    {
        std::string body;
        body.resize(length);

        int len = 0;
        if(length >= offset)
        {
            body.append(data, offset);
            len = offset;
        }
        else
        {
            memcpy(&body[0], data, offset);
            len = length;
        }
        length -= offset;

        if(length > 0)
        {   
            if(readFixSize(&body[len], length) <= 0) return nullptr;
        }
        parser->getData()->setBody(body);
    }
    std::string keep_alive = parser->getData()->getHeader("Connection");
    if(!strcasecmp(keep_alive.c_str(), "leep-alive"))
    {
        parser->getData()->setClose(false);
    }
    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp)
{
    std::stringstream ss;
    ss << *rsp;
    std::string data = ss.str();
    return writeFixSize(ss.str().c_str(), data.size());
}

}
}