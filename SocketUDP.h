//
// Created by sxh on 2017/2/10.
//

#ifndef UDTDEMO_SOCKETUDP_H
#define UDTDEMO_SOCKETUDP_H


#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include "SocketBase.h"

class CSocketUDP:public CSocketBase
{
private:
    int mTargetIP;
    int mTargetPort;
    void printError(const char*headInfo, int errnoCode);
public:
    CSocketUDP();
    CSocketUDP(int sock);

    virtual ~CSocketUDP();

    void close();
    void setDefaultTarget(const char*ip, int port);
    void setDefaultTarget(int ip, int port);
    ssize_t setRecvTimeout(int timeoutMS);

    ssize_t setSendTimeout(int timeoutMS);

    ssize_t setReuseAddr(bool bReuse);

    ssize_t setBroadcast(bool bBroadcast);

    ssize_t send(int ip, int port, const void *pBuf, size_t lenBytes);
    ssize_t send(const char*ip, int port, const void *pBuf, size_t lenBytes);
    ssize_t send(const void *pBuf, size_t lenBytes);

    ssize_t recv(void *outBuf, int wantLen, sockaddr_in *addr);

    ssize_t bind(u_short port);

    ssize_t randBind(u_short *port);



};

class CSocketUDPAutoClose :public CSocketUDP
{
public:
    CSocketUDPAutoClose(int sock):CSocketUDP(sock)
    {

    }
    ~CSocketUDPAutoClose()
    {
        close();
    }
};


//禁止关闭
class CSocketUDPNotClose: public CSocketUDP
{

public:
    CSocketUDPNotClose(int sock):CSocketUDP(sock)
    {

    }
    virtual void close()
    {

    }


};


#endif //UDTDEMO_SOCKETUDP_H
