#include "server/http/http.h"
#include "server/server.h"

void test_request()
{
    shuai::http::HttpRequest::ptr req(new shuai::http::HttpRequest);
    req->setHeader("host", "www.sylar.top");
    req->setBody("hello shuai");

    req->dump(std::cout) << std::endl;
}

void test_response()
{
    shuai::http::HttpResponse::ptr rsp(new shuai::http::HttpResponse);
    rsp->setHeader("X-X", "shuai");
    rsp->setBody(" hello shuai");
    rsp->setStatus((shuai::http::HttpStatus)400);
    rsp->setClose(false);

    rsp->dump(std::cout) << std::endl;
}

int main(int argc, char** argv)
{
    test_request();
    test_response();
    return 0;
}