//
// Created by sxh on 2017/3/21.
//

#include <cstring>
#include "UtilMakePack.h"

#define PACK_MAGIC_NUMBER  0x07363110

int CUtilMakePack::build(void *pBuf, int bufBytes, int identifyCode, unsigned short interfaceType, const void *content, int contentBytes)
{
    char *buf = (char*)pBuf;
    int packHeadLen = GetPackHeadLen();
    int needLen = contentBytes + packHeadLen;
    if(bufBytes< needLen)
    {
        return  needLen;
    }
    SetPackFlag(buf,PACK_MAGIC_NUMBER);
    SetPackFun(buf,interfaceType);
    SetPackID(buf,identifyCode);
    SetPackVer(buf,1);

    memcpy(&buf[packHeadLen], content, contentBytes);
    SetPackLen(buf, needLen);
    SetCheckSum(buf, needLen);
    return needLen;
}

FNetPackHeader CUtilMakePack::GetPackHeader(void *buf)
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

int CUtilMakePack::fillWidthString(void *pBuf, int bufSize, const char*content, unsigned char contenLen)
{
    unsigned char *buf = (unsigned char*)pBuf;
    int len = 0;
    buf[len] = contenLen;
    len += 1;
    strncpy((char*)&buf[len], content, contenLen);
    len += contenLen;
    buf[len]=0;
    len += 1;
    return  len;
}
