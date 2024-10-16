#include "address.h"
#include "endian.h"
#include "log.h"

namespace shuai
{

shuai::Logger::ptr g_logger = SHUAI_LOG_NAME("system");

// 生成主机掩码
template <class T>
static T CreateMask(uint32_t bits)
{
    // 将最低的1 << (sizeof(T) * 8 - bits位设置为1 其余为0
    return (1 << (sizeof(T) * 8 - bits)) - 1; 
}

// 得到value中的位为1的个数
template<class T>
static uint32_t CountBytes(T value)
{
    uint32_t result = 0;
    for(; value; ++result) 
        value &= value - 1;
    return result;
}

Address::ptr Address::Create(const sockaddr* addr, socklen_t addrlen)
{
    if(addr == nullptr) return nullptr;

    Address::ptr result;
    switch (addr->sa_family)
    {
    case AF_INET:
        result.reset(new IPv4Address(*(const sockaddr_in*)addr)); //多态
        break;
    case AF_INET6:
        result.reset(new IPv6Address(*(const sockaddr_in6*)addr));
        break;
    
    default:
        result.reset(new UnknownAddress(*addr));
        break;
    }
    return result;
}

// 它用于解析主机名（host）并返回与该主机相关的地址信息
bool Address::Lookup(std::vector<Address::ptr>& result, const std::string& host, int family, int type, int protocol)
{
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;     // 允许ipv4 ipv6    
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;
    const char* service = NULL;

    // 检查 ipv6address service  检查host字符串是否以[开头，这通常表示一个IPv6地址
    if(!host.empty() && host[0] == '[')
    {
        // void *memchr(const void *str, int c, size_t n);是C标准库中的一个函数，用于在内存块中查找特定的字符。 
        // str: 指向要被搜索的内存块的指针 c: 要查找的字符，尽管此参数的类型为 int，但它会被自动转换为 unsigned char  n: str 中前多少个字节会被检查
        // 尝试找到与[匹配的]，如果找到，endipv6将指向]的位置
        const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
        if(endipv6)
        {
            // TODO check of range
            // 如果]后面紧跟着:，表示地址后可能跟有端口号，设置service指向端口号的开始。
            if(*(endipv6 + 1) == ':') service = endipv6 + 2;
            // 同时提取[和]之间的内容作为节点名node endipv6为指针
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }

    // 如果之前没有设置node（即不是IPv6或没有端口号的情况）-- 可能是ipv6没有设置端口号也可能是ipv4，则检查是否有端口号（IPv4的情况）
    if(node.empty())
    {
        // 在host中搜索第一个:
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service)
        {
            // 找到第一个但是没有找到第二个:,则不是ipv6的情况
            if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1))
            {
                // 此时node为ipv4的ip
                node = host.substr(0, service - host.c_str());
                // server指向端口号
                ++service;
            }
        }
    }

    // 这个情况应该为 host既不是ipv6 也没有端口号，那可能只是ipv4的ip 或者处理host为一个域名（如 "example.com"）的情况
    if(node.empty()) node = host;

    // 使用getaddrinfo根据节点名、服务名和hints提供的参数获取地址信息，结果存储在results中
    /*
        int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
        node: 指定一个主机名或地址字符串（如 “example.com” 或 “192.0.2.1”）。对于基于地址的查询，该参数包含一个数字地址字符串（IPv4或IPv6）。
        service: 指定端口号的字符串表示（如 “80”）。这对于基于服务的查询是必需的
        hints: 提供关于期望返回什么类型的地址信息的提示。这是一个指向addrinfo结构的指针
        res: 一个指向addrinfo结构指针的指针，这里将返回一个指向结果链表的指针
    */ 
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if(error)
    {
        SHUAI_LOG_ERROR(g_logger) << "Address::Lookup getaddress(" << host << ","
                                  << family << ", " << type << ") err = " << error
                                  << " errstr = " << strerror(errno);
        return false;
    }

    next = results;
    while(next)
    {
        result.push_back(Address::Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        next = next->ai_next;
    }

    freeaddrinfo(results);
    return !result.empty();
}

// 在很多网络应用中，DNS解析系统会根据一定的策略（如负载均衡、地理位置等）排序返回的地址。因此，返回第一个地址是基于这样的假设：第一个地址是最合适的。然而，对于需要高可用性或特定需求的应用，可能需要更复杂的逻辑来选择或验证多个返回的地址。
// 通过域名的方式找地址 第一个地址通常认为是最优选的地址 在通常情况下（特别是在简单的应用场景中），第一个解析的地址足以满足需求
Address::ptr Address::LookupAny(const std::string& host, int family, int type, int protocol)
{
    std::vector<Address::ptr> result;
    if(Address::Lookup(result, host, family, type, protocol))
    {
        return result[0];
    }
    return nullptr;
}

IPAddress::ptr Address::LookupAnyIPAddress(const std::string& host, int family, int type, int protocol)
{
    std::vector<Address::ptr> result;
    if(Lookup(result, host, family, type, protocol))
    {
        for(auto& i : result)
        {
            // 若 i 指向其他非 IPAddress 类型的 Address 对象：转换将失败，并返回空指针（nullptr）
            IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
            if(v) return v;
        }
    }
    return nullptr;
}

//  函数的作用是获取当前系统上所有网络接口的地址信息，并将这些信息存储在一个多重映射中
bool Address::GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>>& result, int family)
{
    struct ifaddrs *next, *results;      // 声明两个指针，next 用于遍历链表，results 用于存储 getifaddrs() 的结果。
    if(getifaddrs(&results) != 0)
    {
        SHUAI_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses getifaddrs "
                                    << " err = " << errno
                                    << " errstr = " << strerror(errno);
        return false;
    }
    try
    {
        for(next = results; next; next = next->ifa_next)
        {
            Address::ptr addr;
            uint32_t prefix_length = ~0u;
            if(family != AF_UNSPEC && family != next->ifa_addr->sa_family) continue;

            switch(next->ifa_addr->sa_family)
            {
                case AF_INET:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in));
                        uint32_t netmask = ((sockaddr_in*)next->ifa_netmask)->sin_addr.s_addr;  // 仍然获取子网掩码
                        prefix_length = CountBytes(netmask);  // 计算子网掩码的前缀长度
                    }
                    break;
                case AF_INET6:
                    {
                        addr = Create(next->ifa_addr, sizeof(sockaddr_in6));
                        in6_addr& netmask = ((sockaddr_in6*)next->ifa_netmask)->sin6_addr;
                        prefix_length = 0;
                        // 在 IPv6 中，地址的长度为 128 位，这意味着每个 IPv6 地址由 16 个字节组成
                        for(int i = 0; i < 16; ++i)
                        {
                            prefix_length += CountBytes(netmask.s6_addr[i]);
                        }
                    }
                    break;
                default:
                    break;
            }             
            if(addr)
                result.insert(std::make_pair(next->ifa_name, std::make_pair(addr, prefix_length)));

        }
    }
    catch(...)
    {
        SHUAI_LOG_ERROR(g_logger) << "Address::GetInterfaceAddresses exception";
        return false;
    }
    freeifaddrs(results);
    return true;
}

// 获取指定网络接口的地址信息，并将其存储在结果向量中
bool Address::GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>>& result, const std::string& iface, int family)
{
    // 检查 iface 是否为空或是否为 *，表示用户希望获取所有接口的地址
    if(iface.empty() || iface == "*")
    {
        if(family == AF_INET || family == AF_UNSPEC)
            result.push_back(std::make_pair(Address::ptr(new IPv4Address()), 0u));
        if(family == AF_INET6 || family == AF_UNSPEC)
            result.push_back(std::make_pair(Address::ptr(new IPv6Address()), 0u));
        return true;
    }
    std::multimap<std::string, std::pair<Address::ptr, uint32_t>> results;
    if(!GetInterfaceAddresses(results, family)) return false;

    // 使用 equal_range 查找指定接口 iface 的地址范围，返回迭代器对
    auto its = results.equal_range(iface);
    for(; its.first != its.second; ++its.first)
        result.push_back(its.first->second);
    return true;
}

int Address::getFamily() const
{
    return getAddr()->sa_family;
}   

std::string Address::toString() const
{
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address& rhs) const
{
    socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());   // 获取最小地址长度
    int result = memcmp(getAddr(), rhs.getAddr(), minlen);         // 比较地址数据
    if(result < 0) return true;                                    // 当前对象小于右侧对象
    else if(result > 0) return false;                              // 当前对象大于右侧对象
    else if(getAddrLen() < rhs.getAddrLen()) return true;          // 如果相等，比较长度
    return false;
}

bool Address::operator==(const Address& rhs) const
{
    return getAddrLen() == rhs.getAddrLen() && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address& rhs) const
{
    return !(*this == rhs);
}

// 将明文数字转sock_addr
IPAddress::ptr IPAddress::Create(const char* address, uint16_t port)
{
    // hints 是 addrinfo 结构体，用于指定地址信息的类型和条件。results 是一个指向 addrinfo 结构体的指针，将存放 getaddrinfo 函数的结果。
    addrinfo hints, *results;     
    memset(&hints, 0, sizeof(addrinfo));

    hints.ai_flags = 0;               // 这将允许函数接受域名，而不仅仅是数字格式的地址
    // hints.ai_flags = AI_NUMERICHOST;     // // hints 是 addrinfo 结构体，用于指定地址信息的类型和条件。results 是一个指向 addrinfo 结构体的指针，将存放 getaddrinfo 函数的结果。
    hints.ai_family = AF_UNSPEC;         // 允许 IPv4 和 IPv6

    int error = getaddrinfo(address, NULL, &hints, &results);  // 调用 getaddrinfo 函数来解析给定的 address。第二个参数为 NULL，表示没有服务名称。
    if(error == EAI_NONAME) 
    {
        SHUAI_LOG_ERROR(g_logger) << "IPAddress::Create(" << address
                                  << ", " << port << ") error = " << error
                                  << " errno = " << errno
                                  << " errstr = " << strerror(errno);
        return nullptr;
    }
    try
    {
        // Create为new出IPv4或者IPv6的对象
        IPAddress::ptr result = std::dynamic_pointer_cast<IPAddress>(Address::Create(results->ai_addr, (socklen_t)results->ai_addrlen));
        if(result)
        {
            result->setPort(port);
        }
        freeaddrinfo(results);
        return result;
    }
    catch(...)
    {
        freeaddrinfo(results);
        return nullptr;
    }
}

IPv4Address::IPv4Address(uint32_t address, uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = byteswapToBigEndian(port);
    m_addr.sin_addr.s_addr = byteswapToBigEndian(address);
}

IPv4Address::IPv4Address(const sockaddr_in& address)
{
    m_addr = address;
}

IPv4Address::ptr IPv4Address::Create(const char* address, uint16_t port)
{
    IPv4Address::ptr rt(new IPv4Address);
    rt->m_addr.sin_port = byteswapToBigEndian(port);
    int result = inet_pton(AF_INET, address, &rt->m_addr.sin_addr);
    if(result <= 0)
    {
        SHUAI_LOG_ERROR(g_logger) << "IPv4Address::Create(" << address <<", "
                                  << port << ") rt = " << result
                                  << " errno = " << errno
                                  << " errstr = " << strerror(errno);
        return nullptr;
    }   
    return rt;
}   

const sockaddr* IPv4Address::getAddr() const
{
    return (sockaddr*)&m_addr;
}

sockaddr* IPv4Address::getAddr()
{
    return (sockaddr*)&m_addr;
}

socklen_t IPv4Address::getAddrLen() const
{
    return sizeof(m_addr);
}

// 输出ipv4的可视化的地址
std::ostream& IPv4Address::insert(std::ostream& os) const
{
    int32_t addr = byteswapToBigEndian(m_addr.sin_addr.s_addr);
    os << ((addr >> 24) & 0xff) << "."  // 提取最高的8位，即第一个字节 掩码 0xff 保证只保留最低的8位，这里它不会改变结果，因为右移后只剩下8位了。
       << ((addr >> 16) & 0xff) << "." 
       << ((addr >> 8) & 0xff) << "."  
       << (addr & 0xff);                // 提取最低的8位，即第四个字节
    os << ":" << byteswapToBigEndian(m_addr.sin_port);
    return os;
}

// 广播地址是一个特定子网中所有主机的广播目的地地址，通常在发送给子网上所有设备的数据包时使用 使得发送到该地址的数据包将被网络上的所有设备接收。
IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len)    // prefix_len网络前缀长度
{
    // 如果 prefix_len 大于 32（IPv4地址的最大长度），函数返回 nullptr，表示输入无效
    if(prefix_len > 32) return nullptr;

    sockaddr_in baddr(m_addr);
    // CreateMask 函数基于 prefix_len 生成一个32位主机掩码  ip地址|主机掩码(即~子网掩码) = 广播地址
    baddr.sin_addr.s_addr |= byteswapToBigEndian(CreateMask<uint32_t>(prefix_len));

    return IPv4Address::ptr(new IPv4Address(baddr));
}

// 网络地址是指一个网络的起始地址，用于标识一个特定的网络。网络地址用于路由器将数据包发送到正确的网络。在网络地址中，主机部分全为0。。它用于唯一标识一个网络
IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len)
{
    if(prefix_len > 32) return nullptr;

    sockaddr_in baddr(m_addr);
    // 网络地址 = ip地址 & 子网掩码
    baddr.sin_addr.s_addr &= byteswapToBigEndian(CreateMask<uint32_t>(prefix_len));

    return IPv4Address::ptr(new IPv4Address(baddr));
}

IPAddress::ptr IPv4Address::subnetAddress(uint32_t prefix_len)
{
    sockaddr_in subnet;
    memset(&subnet, 0, sizeof(subnet));
    subnet.sin_family = AF_INET;
    // 子网掩码 = ~主机掩码
    subnet.sin_addr.s_addr = ~byteswapToBigEndian(CreateMask<uint32_t>(prefix_len));
    return IPv4Address::ptr(new IPv4Address(subnet));
}

uint32_t IPv4Address::getPort() const
{
    return byteswapToBigEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint16_t v)
{
    // m_addr.sin_port = htons(v);
    m_addr.sin_port = byteswapToBigEndian(v);
    // SHUAI_LOG_DEBUG(g_logger) << "setPort port v = " << v; 
    // SHUAI_LOG_DEBUG(g_logger) << "setPort port " << m_addr.sin_port; 
}

// IPv6
IPv6Address::IPv6Address()
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const sockaddr_in6& address)
{
    m_addr = address;
}

// 将字符串类型的明文的网络地址转换为网络address
IPv6Address::ptr IPv6Address::Create(const char* address, uint16_t port)
{
    IPv6Address::ptr rt(new IPv6Address);
    rt->m_addr.sin6_port = byteswapToBigEndian(port);
    int result = inet_pton(AF_INET6, address, &rt->m_addr.sin6_addr);
    if(result <= 0)
    {
        SHUAI_LOG_ERROR(g_logger) << "IPv6Address::Create(" << address <<", "
                                  << port << ") rt = " << result
                                  << " errno = " << errno
                                  << " errstr = " << strerror(errno);
        return nullptr;
    }   
    return rt;
}

IPv6Address::IPv6Address(const uint8_t address[16], uint16_t port)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = byteswapToBigEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

const sockaddr* IPv6Address::getAddr() const
{
    return (sockaddr*)&m_addr;
}

sockaddr* IPv6Address::getAddr() 
{
    return (sockaddr*)&m_addr;
}

socklen_t IPv6Address::getAddrLen() const
{
    return sizeof(m_addr);
}

std::ostream& IPv6Address::insert(std::ostream& os) const
{
    os << "[";
    uint16_t* addr = (uint16_t*)m_addr.sin6_addr.s6_addr;
    bool used_zeros = false;
    for(size_t i = 0; i < 8; ++i)
    {
        if(addr[i] == 0 && !used_zeros) continue;

        if(i && addr[i-1] == 0 && !used_zeros)
        {
            os << ":";
            used_zeros = true;
        }
        if(i)
        {
            os << ":";
        }
        os << std::hex << (int)byteswapToBigEndian(addr[i]) << std::dec;
    }
    if(!used_zeros && addr[7] == 0)
        os << "::";

    os << "]:" << byteswapToBigEndian(m_addr.sin6_port);
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len)
{
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len);

    for(int i = prefix_len / 8 + 1; i < 16; ++i)
    {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len)
{
    sockaddr_in6 baddr(m_addr);
    baddr.sin6_addr.s6_addr[prefix_len / 8] &= ~CreateMask<uint8_t>(prefix_len);
 
    // for(int i = prefix_len / 8 + 1; i < 16; ++i)
    // {
        // baddr.sin6_addr.s6_addr[i] = 0xff;
    // }
    return IPv6Address::ptr(new IPv6Address(baddr));
}

IPAddress::ptr IPv6Address::subnetAddress(uint32_t prefix_len)
{
    sockaddr_in6 subnet;
    memset(&subnet, 0, sizeof(subnet));
    m_addr.sin6_family = AF_INET6;

    for(uint32_t i= 0; i < prefix_len / 8; ++i)
    {
        subnet.sin6_addr.s6_addr[i] = 0xFF;
    }
    return IPv6Address::ptr(new IPv6Address(subnet));
}

uint32_t IPv6Address::getPort() const
{
    return byteswapToBigEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint16_t v)
{
    m_addr.sin6_port = byteswapToBigEndian(v);
}

static const size_t MAX_PATH_LEN = sizeof(((sockaddr_un*)0)->sun_path) - 1;
// Unix
UnixAddress::UnixAddress()
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

UnixAddress::UnixAddress(const std::string& path)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;

    if(!path.empty() && path[0] == '\0')
        --m_length;

    if(m_length > sizeof(m_addr.sun_path))
        throw std::logic_error("path too long");

    memcpy(m_addr.sun_path, path.c_str(), m_length);
    m_length += offsetof(sockaddr_un, sun_path);
}

void UnixAddress::setAddrLen(uint32_t v) {
    m_length = v;
}

const sockaddr* UnixAddress::getAddr() const
{
    return (sockaddr*)&m_addr;
}

sockaddr* UnixAddress::getAddr()
{
    return (sockaddr*)&m_addr;
}

socklen_t UnixAddress::getAddrLen() const
{
    return m_length;
}

std::ostream& UnixAddress::insert(std::ostream& os) const
{
    if(m_length > offsetof(sockaddr_un, sun_path) && m_addr.sun_path[0] == '\0')
        return os << "\\0" << std::string(m_addr.sun_path + 1, m_length - offsetof(sockaddr_un, sun_path) - 1);

    return os << m_addr.sun_path;        
}

// Unknown
UnknownAddress::UnknownAddress(int family)
{
    memset(&m_addr, 0, sizeof(m_addr));
    m_addr.sa_family = family;
}

UnknownAddress::UnknownAddress(const sockaddr& addr)
{
    m_addr = addr;
}

const sockaddr* UnknownAddress::getAddr() const
{
    return &m_addr;
}

sockaddr* UnknownAddress::getAddr() 
{
    return &m_addr;
}

socklen_t UnknownAddress::getAddrLen() const
{
    return sizeof(m_addr);
}

std::ostream& UnknownAddress::insert(std::ostream& os) const
{
    os << "[UnknownAddress family = " << m_addr.sa_family << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, Address& addr)
{
    return addr.insert(os);
}

}
