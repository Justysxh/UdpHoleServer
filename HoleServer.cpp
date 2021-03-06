//
// Created by sxh on 2017/3/7.
//

#include <string.h>
#include <errno.h>
#include <cstdio>
#include <sys/time.h>
#include <list>
#include <map>
#include <alloca.h>
#include "SocketUDP.h"
#include "ThreadSyncUtil.h"
#include "NetApiDefine.h"
#include "UtilMakePack.h"


#define CODE_BUFF_SIZE  36

#define PACK_EXPIRE_TIME 5


void printBuffHexString(const void *pBuf, int len)
{
    len = std::min(1024,len);
    const unsigned char *buf = (const unsigned char*)pBuf;
    //总字节数=行数x每行字节数  每行字节数=字节数*3字节+ 空格+回车+16个字符  每个节点用3个,是16进制占两位,加空格占1位
    int buffSize = (len+15)/16 * (16*3+1+16+1);
    char *str = (char*)alloca(buffSize);
    char *outBuf = str;
    char temp[0x20]={0};
    int j = 0;
    for(int i=0; i<len; ++i)
    {
        sprintf(outBuf,"%02X ",buf[i]);
        temp[j] = buf[i];
        outBuf += 3;
        ++j;
        if( (i+1)%16==0)
        {
            temp[j] = 0;
            for(int k=0;k<16; ++k)
            {
                if(temp[k]<0x20 || temp[k]==0x7F)
                {
                    temp[k]='.';
                }
            }
            j = sprintf(outBuf," %s\n",temp);
            outBuf += j;
            j=0;
        }
    }
    if(j>0)
    {
        memset(outBuf,0x20,(16-j)*3);//不满16个, 以空格填充
        outBuf += (16-j)*3;
        temp[j]=0;
        for(int k=0;k<j; ++k)
        {
            if(temp[k]<0x20 || temp[k]==0x7F)
            {
                temp[k]='.';
            }
        }
        j = sprintf(outBuf," %s\n",temp);
        outBuf += j;
    }
    outBuf[0]='\n';
    outBuf[1] = 0;
    myprintf("\nprint buffer len:%d\n%s",len,str);
}


char* SafeCopy(char* buf, int size, const char*src)
{
    int len = strlen(src);
    len = std::min(size,len);
    char*ret =  strncpy(buf, src,len);
    buf[len]=0;
    return ret;
}



class CPackInfo
{
public:
    int mIdentifyCode;
    int mPublicIP; //公网IP
    int mPrivateIP; //私网IP
    unsigned short mPublicPort; //公网端口
    unsigned short mPrivatePort; //私网端口
    char mP2pHashCode[CODE_BUFF_SIZE];//MD5码作为盒子标识
    char mP2pKey[CODE_BUFF_SIZE];
    unsigned short mInterfaceType; //命令协议类型
    timeval mStartTime; //包接收时间
public:
    CPackInfo()
    {
        clear();
        gettimeofday(&mStartTime, 0);        
    }
    CPackInfo(const CPackInfo &other)
    {
        copy(other);
        gettimeofday(&mStartTime, 0);
    }
    void copy(const CPackInfo &other)
    {
        mPublicIP = other.mPublicIP;
        mPublicPort = other.mPublicPort;
        mPrivateIP = other.mPrivateIP;
        mPrivatePort = other.mPrivatePort;
        mStartTime = other.mStartTime;
        mInterfaceType = other.mInterfaceType;
        mIdentifyCode = other.mIdentifyCode;
        memcpy(mP2pHashCode, other.mP2pHashCode,CODE_BUFF_SIZE);
        SafeCopy(mP2pKey,CODE_BUFF_SIZE,other.mP2pKey);
    }
    void clear()
    {
        mPublicIP = 0;
        mPrivatePort = 0;
        mPrivateIP = 0;
        mPublicPort = 0;
        mP2pHashCode[0] = 0;
        mP2pKey[0]=0;
        mInterfaceType = 0;
        mStartTime.tv_sec = 0;
        mStartTime.tv_usec = 0;
        mIdentifyCode = 0;
    }
    void parse(const void *pBuf,int len, int publicIP, unsigned short publicPort)
    {
        mPublicIP = publicIP;
        mPublicPort = publicPort;
        mInterfaceType = (unsigned short) CUtilMakePack::GetPackFun(pBuf);
        mIdentifyCode = CUtilMakePack::GetPackID(pBuf);
        int offset = CUtilMakePack::GetPackHeadLen();
        mPrivateIP = CUtilMakePack::GetPackInt(pBuf, offset);
        offset += 4;
        mPrivatePort  = CUtilMakePack::GetPackShort(pBuf, offset);
        offset += 2;
        const char *pCode = (const  char*)pBuf;

        unsigned char strLen = (unsigned char)pCode[offset]; //p2p key length
        offset += 1;
        SafeCopy(mP2pKey,CODE_BUFF_SIZE, &pCode[offset]); //p2p key
        offset += strLen;

        offset += 1;
        SafeCopy(mP2pHashCode, CODE_BUFF_SIZE, &pCode[offset]); // p2p hash code
        printfInfo("parse pack ");
        
    }
    
    void printfInfo(const char* msg)
    {
        in_addr priveteAddr={0};
        in_addr publicAddr={0};
        priveteAddr.s_addr = mPrivateIP;
        publicAddr.s_addr = mPublicIP;
        char tempIp[0x20] = {0};
        strcpy(tempIp,inet_ntoa(publicAddr));
        int tempPort = ntohs(mPublicPort);
        printf("%s %s %s  (%s:%d) (%s:%d)\n",msg,mP2pHashCode, mP2pKey, tempIp, tempPort, inet_ntoa(priveteAddr),ntohs(mPrivatePort));
    }

    bool checkTimeout()
    {
        timeval tv = {0};
        gettimeofday(&tv,0);
        printf("check timeout %li - %li = %li\n", tv.tv_sec,mStartTime.tv_sec,tv.tv_sec - mStartTime.tv_sec);
        return tv.tv_sec - mStartTime.tv_sec>= PACK_EXPIRE_TIME;
    }

};



class CHoleServer;

class CThreadHolePackProcess: public  CWorkThreadBase
{
    CHoleServer *mpServer;
public:
    CThreadHolePackProcess(CHoleServer *pServer)
    {
        mpServer = pServer;
        mIsSelfFree = false;
    }
    virtual void run();


    //
};




class CAutoCloseSock: public CSocketUDP
{
public:
    ~CAutoCloseSock()
    {
        close();
    }
};

class CHoleServer
{
private:
    unsigned  short mPort;
    CAutoCloseSock mSock;
    CThreadCond mSignal;
    std::list<CPackInfo*> mPacks; //待处理的包
    CThreadMutex mPackLocker;
    std::map<std::string,CPackInfo*> mPeers; //等待被打洞的peer列表
    bool mIsRun;

public:
    CHoleServer(unsigned short port)
    {
        mPort = port;
        mIsRun = false;
    }
    ~CHoleServer()
    {
        clear();
    }
    void run()
    {
        ssize_t ret = mSock.bind(mPort);
        if(ret!=0)
        {
            printf("bind port %d failed code:%d msg:%s\n", mPort, errno, strerror(errno));
            return;
        }

        mIsRun = true;
        CThreadHolePackProcess th(this);
        th.start();

        sockaddr_in recvAddr = {0};
        unsigned char recvBuf[0x100]={0};
        CPackInfo pack;
        while(true)
        {
            ret = mSock.recv(recvBuf, 0x100, &recvAddr);
            if(ret==-1)
            {
                if(errno == EINTR)
                {
                    continue;
                }
                printf("recvfrom failed error:%d msg:%s\n", errno,strerror(errno));
                break;
            }
            printf("client come: %s:%d\n", inet_ntoa(recvAddr.sin_addr), ntohs(recvAddr.sin_port));
            pack.parse(recvBuf, ret, recvAddr.sin_addr.s_addr, recvAddr.sin_port);
            printBuffHexString(recvBuf, ret);
            addPack(pack);

        }
        mIsRun = false;
        mSignal.signal();
        th.stop();
        th.join();
    }

    CPackInfo *peekPack()
    {
        CPackInfo *pInfo = NULL;
        mPackLocker.lock();
        if(mPacks.empty()==false)
        {
            pInfo = mPacks.front();
            mPacks.pop_front();
        }
        mPackLocker.unlock();
        return pInfo;
    }

    void addPack(CPackInfo &pack)
    {
        CPackInfo *pPack = new CPackInfo(pack);
        mPackLocker.lock();
        mPacks.push_back(pPack);
        mPackLocker.unlock();
        mSignal.signal();
    }

    void clear()
    {
        mPackLocker.lock();
        std::list<CPackInfo*>::iterator it = mPacks.begin();
        while(it!=mPacks.end())
        {
            CPackInfo *pPack = *it;
            delete pPack;
            ++it;
        }
        mPacks.clear();
        mPackLocker.unlock();
    }
    int wait(int timeMS)
    {
        return mSignal.wait(timeMS);
    }

    void packProc()
    {
        while(mIsRun)
        {
            //从链表中取得数据包
            CPackInfo *pInfo = peekPack();
            if(pInfo == NULL)
            {
                printf("no data wait signa....peer(%d)\n", mPeers.size());
                mSignal.wait();
                printf("some data should proc!!!!\n");
                clearExpirePeer();
                continue;
            }
            //处理数据包
            if(pInfo->mInterfaceType == Fun_Hole || pInfo->mInterfaceType == Fun_HoleHeart)
            {
                if(pInfo->mInterfaceType == Fun_Hole)
                {
                    printf("[*** hole request ***] %s %s\n", pInfo->mP2pKey,pInfo->mP2pHashCode);
                } else
                {
                    printf("[=== hole heart ===] %s %s\n", pInfo->mP2pKey,pInfo->mP2pHashCode);
                }
                CPackInfo *pPeer = findPeer(pInfo);
                if(pPeer)
                {
                    printf("send p2p start.------------------------------------------------\n");
                    //向盒子发达打洞命令
                    holeResponse(pPeer, pInfo, Fun_IpAndPortResponse);

                }
                //响应打app打洞
                holeResponse(pInfo, pPeer, Fun_HoleResponse);
                delete pInfo;
                if(pPeer) delete pPeer;
            }
            else if(pInfo->mInterfaceType == Fun_IpAndPort)
            {
                //记录信息到列表中
                //删除过期数据
                clearExpirePeer();
                addPeer(pInfo);
                holeResponse(pInfo, 0, Fun_IpAndPortResponse);
            }
            else if(pInfo->mInterfaceType == Fun_PeerLogout)
            {
                removePeer(pInfo);
            }
            
        }
    }
    
    void removePeer(CPackInfo *pPack)
    {
        std::map<std::string,CPackInfo*>::iterator it = mPeers.find(pPack->mP2pHashCode);
        if(it != mPeers.end())
        {
            delete pPack;
            pPack = it->second;
            delete pPack;
            mPeers.erase(it);
        }
        printf("< Box > logout peer(%d) %s\n", mPeers.size(),pPack->mP2pHashCode);
    }

    void clearExpirePeer()
    {
        std::map<std::string,CPackInfo*>::iterator it = mPeers.begin();
        std::map<std::string,CPackInfo*>::iterator itSave;
        while(it!= mPeers.end())
        {
            CPackInfo *pPack = it->second;
            if(pPack->checkTimeout())
            {
                printf("peer is timeout %s\n", pPack->mP2pHashCode);
                itSave = it;
                mPeers.erase(itSave);
                ++it;
                delete pPack;
            }
            else
            {
                ++it;
            }
        }
    }

    void addPeer(CPackInfo *pPack)
    {
        std::map<std::string,CPackInfo*>::iterator it = mPeers.find(pPack->mP2pHashCode);
        if(it != mPeers.end())
        {
            printf("update peer %s\n", pPack->mP2pHashCode);
            it->second->copy(*pPack);
            delete pPack;
        }
        else
        {
            printf("[+++ box online +++] %s \n", pPack->mP2pHashCode);
            mPeers.insert(std::pair<std::string,CPackInfo*>(pPack->mP2pHashCode,pPack));
        }

    }

    CPackInfo* findPeer(CPackInfo *pPack)
    {
        printf("find peer(%d)\n", mPeers.size());
        CPackInfo *pInfo = NULL;
        std::map<std::string,CPackInfo*>::iterator it = mPeers.find(pPack->mP2pHashCode);
        if(it != mPeers.end())
        {
            pInfo = it->second;
            mPeers.erase(it);
        }
        return pInfo;
    }



    int holeResponse(CPackInfo *selfPack, CPackInfo* peerPack, unsigned short interfaceType)
    {
        unsigned char buf[0x100]={0};
        CUtilMakePack::build(buf,0x100, selfPack->mIdentifyCode,interfaceType,buf,0);
        int packHeadLen = CUtilMakePack::GetPackHeadLen();
        int len = packHeadLen;
        buf[len]= peerPack==0?0:1;
        len += 1;
        *(int*)&buf[len] = selfPack->mPublicIP;
        len += 4;
        *(unsigned short*)&buf[len] = selfPack->mPublicPort;
        len += 2;
        if(peerPack)
        {
            *(int*)&buf[len] = peerPack->mPublicIP; //公网ip
            len += 4;
            *(unsigned short*)&buf[len] = peerPack->mPublicPort;
            len += 2;
            *(int*)&buf[len] = peerPack->mPrivateIP; //内网ip
            len += 4;
            *(unsigned short*)&buf[len] = peerPack->mPrivatePort;
            len += 2;
            len += CUtilMakePack::fillWidthString(&buf[len],0x100,peerPack->mP2pKey,strlen(peerPack->mP2pKey));
        }
        CUtilMakePack::SetPackLen(buf,len);
        selfPack->printfInfo("response to ");
        printBuffHexString(buf,len);
        ssize_t ret = mSock.send(selfPack->mPublicIP,selfPack->mPublicPort, buf, len);
        if(ret<1)
        {
            printf("send hole response failed code:%d msg:%s\n", errno, strerror(errno));
        }
        return len;
    }
};


void CThreadHolePackProcess::run()
{
    printf("HolePack process thread is run\n");
    mpServer->packProc();
    printf("hole pack process thread is end\n");
}

int main()
{
    CHoleServer server(18901);
    server.run();
    return  0;
}
