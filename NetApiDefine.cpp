//
//  NetApiDefine.cpp
//  FProject
//
//  Created by wdkjapp on 2016/12/29.
//  Copyright © 2016年 UNICOMCQ. All rights reserved.
//

#include <stdio.h>
#include <cstring>
#include "NetApiDefine.h"
#include <sys/time.h>
#include <stdarg.h>

int FNetCallback::sIDSeed = 1;

void SetPackFlag(void*buf, int flag)
{
    *(int*)buf = flag;
}
void SetPackVer(void* buf, unsigned char ver)
{
    ((char*)buf)[10] = ver;
}

int GetPackLen(const void *buf)
{
    return *(int*)&((char*)buf)[4];
}
void SetPackLen(void*buf, int packLen)
{
    *(int*)&((char*)buf)[4] = packLen;
}
short GetPackFun(const void* buf)
{
    return *(short*)&((char*)buf)[8];
}
void SetPackFun( void*buf, short fun)
{
    *(short*)&((char*)buf)[8] = fun;
}
int GetPackID(const void* buf)
{
    return *(int*)&((char*)buf)[11];
}
void SetPackID(void*buf, int packID)
{
    *(int*)&((char*)buf)[11] = packID;
}


unsigned int GetPackInt(const void *pBuf, int offset)
{
    const char *buf = (const char*)pBuf;
    return *(unsigned int*)&buf[offset];
}

unsigned short GetPackShort(const void *pBuf, int offset)
{
    const char *buf = (const char*)pBuf;
    return *(unsigned short*)&buf[offset];
}




int GetPackHeadLen()
{
    return 17;
}

int GetPackFlag(const void *buf)
{
    return *(int*)buf;
}
unsigned char GetPackVer(const void* buf)
{
    return ((char*)buf)[10];
}
unsigned short GetCheckSum(const void* buf)
{
    return *(unsigned short*)&((char*)buf)[15];
}

int FindPackFlag(const void *pBuf,int len)
{
    int index = 0;
    char *pData = (char*)pBuf;
    do
    {
        if( *(int*)&pData[index] == PACK_FALG)
        {
            int dataLen = GetPackLen(&pData[index]);
            if(dataLen<0 || dataLen>10*1024*1024)//包长不正确,也继续查找
            {
                myprintf("pack len is error len=%d\n", dataLen);
                ++index;
                continue;
            }
            return  index;
        }
        ++index;
    }while(index+3<len);
    return -1;
}

void SetCheckSum(void *buf, int packLen)
{
    char *pBuf = (char*)buf;
    *(unsigned short*)&pBuf[15] = 0;
}

FNetPackHeader GetPackHeader(void *buf)
{
    FNetPackHeader header = {0};
    header.headFlag = GetPackFlag(buf);
    header.identifyCode = GetPackID(buf);
    header.interfaceType = GetPackFun(buf);
    header.checkSum = GetCheckSum(buf);
    header.packLen = GetPackLen(buf);
    header.ver = GetPackVer(buf);
    memcpy(header.ori,buf,GetPackHeadLen());
    return header;
}

void GenPackHead(void *buf, unsigned short fun, int id)
{
    SetPackFlag((unsigned  char*)buf,PACK_FALG);
    SetPackFun((unsigned char*)buf,fun);
    SetPackID((unsigned char*)buf,id);
    SetPackVer((unsigned char*)buf,1);
}


int GenPack(void *pBuf, int bufBytes, int identifyCode, unsigned short interfaceType, const void*content, int contentBytes)
{
    int packHeadLen = GetPackHeadLen();
    int needLen = contentBytes + packHeadLen;
    if(bufBytes< needLen)
    {
        return  needLen;
    }
    char *buf = (char*)pBuf;
    GenPackHead(buf, interfaceType,identifyCode);
    memcpy(&buf[packHeadLen], content, contentBytes);
    SetPackLen(buf, needLen);
    SetCheckSum(buf, needLen);
    return needLen;
}

//本地广播搜索包
int GenBroadcastPack(void*pBuf, int bufBytes,int pid,const char*ip, int port,int st)
{
    CJsonParam jp;
    jp.add("ip",ip);
    jp.add("port",port);
    jp.add("st",st);
    return GenPack(pBuf,bufBytes,pid, Fun_SearchBox, jp.toString().c_str(),jp.length());
}

//响应本地广播搜索包
int GenBroadcastResponsePack(void*pBuf, int bufBytes,int pid,const char*ip, int port,int st,const char*name,const char*code)
{
    CJsonParam jp;
    jp.add("ip",ip);
    jp.add("port",port);
    jp.add("st",st);
    jp.add("name",name);
    jp.add("code",code);
    return GenPack(pBuf,bufBytes,pid,Fun_SearchBoxResponse,jp.toString().c_str(),jp.length());
}

//本地连接广播包
int GenLocalConnect(void *pBuf, int bufSize, int pid, const char*localIP, int localPort,int udtPort, int st, const char*user,const char*code)
{
    CJsonParam param;
    param.add("ip", localIP);
    param.add("port", localPort);
    param.add("st",st);
    param.add("user",user);
    param.add("code",code);
    param.add("udtPort",udtPort);
    return GenPack(pBuf,bufSize,pid, Fun_LocalConnect, param.toString().c_str(),param.length());
}

//响应本地连接广播包
int GenLocalConnectResponse(void *pBuf, int bufSize,int pid,bool bSuccsee,const char*msg,
                            const char*localIP, int localPort, const char*name, const char* code)
{
    CJsonParam jp;
    jp.addBool("success",bSuccsee);
    jp.add("msg",msg);
    jp.add("ip",localIP);
    jp.add("port", localPort);
    jp.add("st",2);
    jp.add("name",name);
    jp.add("code", code);
    return GenPack(pBuf,bufSize,pid, Fun_LocalConnectResponse, jp.toString().c_str(),jp.length());
}

int GenMsgOnlyJson(void *pBuf, int bufSize, bool bSuccess, const char*msg)
{
    CJsonParam param;
    param.addBool("success", bSuccess);
    param.add("msg", msg);
    std::string tStr = param.toString();
    strcpy((char*)pBuf,tStr.c_str());
    return tStr.length();
}


void fnetPrintf(const char* pstrFormat, ...)
{
    va_list args;
    va_start(args, pstrFormat);
    timeval tv = {0};
    gettimeofday(&tv, NULL);
    printf("%li.%li ", tv.tv_sec,tv.tv_usec);
    vprintf( pstrFormat, args);
    va_end(args);
}

