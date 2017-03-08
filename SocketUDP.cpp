//
// Created by sxh on 2017/2/10.
//

#include <cstdlib>
#include <errno.h>
#include "SocketUDP.h"
#include "NetApiDefine.h"

CSocketUDP::CSocketUDP()
{
    mSock = socket(AF_INET,SOCK_DGRAM,0);
}
CSocketUDP::CSocketUDP(int sock)
{
    mSock = sock;
}

CSocketUDP::~CSocketUDP()
{
}
void CSocketUDP::close()
{
    if(mSock != -1)
    {
        myprintf("UDP sock(%d) closed\n",mSock);
        shutdown(mSock,SHUT_RDWR);
        ::close(mSock);
        mSock = -1;
    }

}

void CSocketUDP::setDefaultTarget(const char*ip, int port)
{
    setDefaultTarget(inet_addr(ip), htons(port));
}
void CSocketUDP::setDefaultTarget(int ip, int port)
{
    mTargetIP = ip;
    mTargetPort = port;
}
ssize_t CSocketUDP::setRecvTimeout(int timeoutMS)
{
    struct timeval to = {timeoutMS/1000, (timeoutMS%1000) *1000};
    if(timeoutMS == -1)
    {
        to.tv_sec = -1;
        to.tv_usec = 0;
    }
    size_t ret = setsockopt(mSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&to, sizeof(to));
    if(ret != 0)
    {
        printError("UDP set recv timeout error",errno);
    }
    return ret;
}

ssize_t CSocketUDP::setSendTimeout(int timeoutMS)
{
    struct timeval to = {timeoutMS/1000, (timeoutMS%1000) *1000};
    if(timeoutMS == -1)
    {
        to.tv_sec = -1;
        to.tv_usec = -1;
    }
    ssize_t ret = setsockopt(mSock, SOL_SOCKET, SO_SNDTIMEO, (char*)&to, sizeof(to));
    if(ret != 0)
    {
        printError("set send timeout error",errno);
    }
    return ret;
}

ssize_t CSocketUDP::setReuseAddr(bool bReuse)
{
    int val = bReuse?1:0;
    ssize_t ret = setsockopt(mSock, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));
    if(ret != 0)
    {
        printError("set reuse addr  error",errno);
    }
    return ret;
}

ssize_t CSocketUDP::setBroadcast(bool bBroadcast)
{
    int opt = bBroadcast?1:0;
    ssize_t ret = setsockopt(mSock,SOL_SOCKET,SO_BROADCAST,(char*)&opt,sizeof(opt));
    if(ret != 0)
    {
        printError("set broadcast error",errno);
    }
    return ret;
}

ssize_t CSocketUDP::send(int ip, int port, const void *pBuf, size_t lenBytes)
{
    //myprintf("UDP(%d) send len:%d\n",mSock,lenBytes);
    //CFNetUtil::printBuffHexString(pBuf,lenBytes);
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = (u_short )port;
    addr.sin_addr.s_addr = (u_int )ip;
    ssize_t ret = sendto(mSock, pBuf, lenBytes,0,(sockaddr*)&addr, sizeof(sockaddr_in));
    if(ret <1)
    {
        printError("UDP send error",errno);
    }
    return ret;
}
ssize_t CSocketUDP::send(const char*ip, int port, const void *pBuf, size_t lenBytes)
{
    return send(inet_addr(ip),htons(port), pBuf, lenBytes);
}
ssize_t CSocketUDP::send(const void *pBuf, size_t lenBytes)
{
    return send(mTargetIP,mTargetPort, pBuf, lenBytes);
}

ssize_t CSocketUDP::recv(void *outBuf, int wantLen, sockaddr_in *addr)
{
    socklen_t len = sizeof(sockaddr_in);
    ssize_t ret = recvfrom(mSock,outBuf,wantLen,0, (sockaddr*)addr, &len);
    if(ret <1)
    {
        printError("UDP recv error",errno);
    }
    return ret;
}
ssize_t CSocketUDP::bind(u_short port)
{
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    socklen_t addrLen = sizeof(sockaddr_in);
    ssize_t ret = ::bind(mSock,(sockaddr*)&addr,addrLen);
    if(ret != 0)
    {
        printError("bind error",errno);
    }
    mLocalPort = port;
    myprintf("bind ok ip port:%d\n",mLocalPort);
    return ret;
}

//随机端口绑定
ssize_t CSocketUDP::randBind(u_short *port)
{
    srand(mSock);
    sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    int randPort = 10000+(rand()%29000);
    addr.sin_port = htons(randPort);
    socklen_t addrLen = sizeof(sockaddr_in);
    ssize_t ret = ::bind(mSock,(sockaddr*)&addr,addrLen);
    int retryTimes = 100;//定一个次数, 避免极端情况下死循环
    while(ret != 0 && errno == EADDRINUSE && retryTimes>0)
    {
        randPort = 10000+(rand()%29000);
        addr.sin_port = htons(randPort);
        ret = ::bind(mSock,(sockaddr*)&addr,addrLen);
        --retryTimes;
    }
    if(ret != 0)
    {
        printError("rand bind error",errno);
    }
    else
    {
        myprintf("rand bind ok ip port:%d\n",randPort);
        *port = (u_short)randPort;
    }
    mLocalPort = (u_short)randPort;
    return ret;
}



void CSocketUDP::printError(const char *headInfo, int errnoCode)
{
    myprintf("%s code:%d msg: %s\n",headInfo,errnoCode, strerror(errnoCode));
}













