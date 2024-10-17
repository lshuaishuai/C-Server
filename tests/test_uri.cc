#include "server/uri.h"
#include <iostream>

int main(int argc, char** argv)
{
    shuai::Uri::ptr uri = shuai::Uri::Create("http://www.sylar.top/test/中文/uri?id=100&name=sylar&vv=中文#frg中文");
    std::cout << uri->toString() << std::endl;
    auto addr = uri->createAddress();
    std::cout << *addr << std::endl;
    return 0;
}