#include "server/address.h"
#include "server/log.h"

shuai::Logger::ptr g_logger = SHUAI_LOG_ROOT();

void test()
{
    std::vector<shuai::Address::ptr> addrs;

    bool v= shuai::Address::Lookup(addrs, "www.baidu.com");
    if(!v)
    {
        SHUAI_LOG_ERROR(g_logger) << "lokkup error";
        return;
    }
    for(size_t i = 0; i < addrs.size(); ++i)
    {
        SHUAI_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

void test_iface()
{
    std::multimap<std::string, std::pair<shuai::Address::ptr, uint32_t>> results;
    bool v= shuai::Address::GetInterfaceAddresses(results);

    if(!v)
    {
        SHUAI_LOG_ERROR(g_logger) << "GetInterfaceAddress fail";
        return;
    }

    for(auto& i : results)
    {
        SHUAI_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - " << i.second.second;
    }

}

void test_ipv4()
{
    // auto addr = shuai::IPAddress::Create("www.sylar.top");
    auto addr = shuai::IPAddress::Create("127.0.0.8");
    if(addr)
    {
        SHUAI_LOG_INFO(g_logger) << addr->toString();
    }
}

int main(int argc, char* argv[])
{
    // test();
    // test_iface();
    test_ipv4();
    return 0;
}