#pragma once 

#include <memory>
#include <netinet/tcp.h> 
#include <sys/socket.h>

#include "address.h"
#include "hook.h"
#include "macro.h"
#include "noncopyable.h"

namespace shuai
{
class Socket: public std::enable_shared_from_this<Socket>, Noncopyable
{
public:
    typedef std::shared_ptr<Socket> ptr;
    typedef std::weak_ptr<Socket> weak_ptr;

    enum Type
    {
        TCP = SOCK_STREAM,
        UDP = SOCK_DGRAM
    };

    enum Family
    {
        IPv4 = AF_INET,
        IPv6 = AF_INET6,
        UNIX = AF_UNIX
    };

    static Socket::ptr CreateTCP(shuai::Address::ptr address);
    static Socket::ptr CreateUDP(shuai::Address::ptr address);

    static Socket::ptr CreateTCPSocket();
    static Socket::ptr CreateUDPSocket();

    static Socket::ptr CreateTCPSocket6();
    static Socket::ptr CreateUDPSocket6();

    static Socket::ptr CreateUnixTCPSocket();
    static Socket::ptr CreateUnixUDPSocket();

    Socket(int family, int type, int procotol = 0);
    ~Socket();

    int64_t getSendTimeout();
    void setSendTimeout(int64_t v);
  
    int64_t getRecvTimeout();
    void setRecvTimeout(int64_t v);

    // 获取socket句柄上的相关信息
    bool getOption(int level, int option, void* result, size_t* len);
    template<class T>
    bool getOption(int level, int option, T& result)
    {
        size_t length = sizeof(T);
        return getOption(level, option, &result, &length);
    }

    bool setOption(int level, int option, const void* result, size_t len);
    template<class T>
    bool setOption(int level, int option, const T& value)
    {
        return setOption(level, option, &value, sizeof(T));
    }

    Socket::ptr accept();

    bool init(int sock);
    bool bind(const Address::ptr addr);
    bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);
    bool listen(int backlog = SOMAXCONN);
    bool close();

    int send(const void* buffer, size_t length, int flags = 0);
    int send(const iovec* buffers, size_t length, int flags = 0);
    int sendTo(const void* buffer, size_t length, const Address::ptr to, int flags = 0);
    int sendTo(const iovec* buffers, size_t length, const Address::ptr to, int flags = 0);

    int recv(void* buffer, size_t length, int flags = 0);
    int recv(iovec* buffers, size_t length, int flags = 0);
    int recvfrom(void* buffer, size_t length, Address::ptr from, int flags = 0);
    int recvfrom(iovec* buffers, size_t length, Address::ptr from, int flags = 0);

    Address::ptr getRemoteAddress();
    Address::ptr getLocalAddress();

    int getFamily() const { return m_family; }
    int getType() const { return m_type; }
    int getProtocol() const { return m_protocol; }

    bool isConnected() const { return m_isConnected; }
    bool isValid() const;
    int getError();

    std::ostream& dump(std::ostream& os) const;
    int getSocket() const { return m_sock; }

    bool cancelRead();
    bool cancelWrite();
    bool cancelAccept();
    bool cancelAll();

private:
    void initSock();
    void newSock();

private:
    int m_sock;
    int m_family;
    int m_type;
    int m_protocol;
    bool m_isConnected;

    Address::ptr m_localAddress;
    Address::ptr m_remoteAddress;
};

}
