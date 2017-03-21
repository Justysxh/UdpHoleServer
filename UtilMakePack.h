//
// Created by sxh on 2017/3/21.
//

#ifndef UDTDEMO_UTILMAKEPACK_H
#define UDTDEMO_UTILMAKEPACK_H

struct FNetPackHeader
{
    int headFlag;  //包头标识
    int packLen; //包长
    unsigned short interfaceType; //功能字
    unsigned char ver; //协议版本号
    int identifyCode; //包id号
    unsigned short checkSum; //校验和
    unsigned char ori[0x20]; //原始字节

};

class CUtilMakePack
{
public:

    inline static void SetPackFlag(void*buf, int flag)
    {
        *(int*)buf = flag;
    }
    inline static void SetPackVer(void* buf, unsigned char ver)
    {
        ((char*)buf)[10] = ver;
    }

    inline static int GetPackLen(const void *buf)
    {
        return *(int*)&((char*)buf)[4];
    }
    inline static void SetPackLen(void*buf, int packLen)
    {
        *(int*)&((char*)buf)[4] = packLen;
    }
    inline static short GetPackFun(const void* buf)
    {
        return *(short*)&((char*)buf)[8];
    }
    inline static void SetPackFun( void*buf, short fun)
    {
        *(short*)&((char*)buf)[8] = fun;
    }
    inline static int GetPackID(const void* buf)
    {
        return *(int*)&((char*)buf)[11];
    }
    inline static void SetPackID(void*buf, int packID)
    {
        *(int*)&((char*)buf)[11] = packID;
    }


    inline static unsigned int GetPackInt(const void *pBuf, int offset)
    {
        const char *buf = (const char*)pBuf;
        return *(unsigned int*)&buf[offset];
    }

    inline  static unsigned short GetPackShort(const void *pBuf, int offset)
    {
        const char *buf = (const char*)pBuf;
        return *(unsigned short*)&buf[offset];
    }




    inline static int GetPackHeadLen()
    {
        return 17;
    }

    inline static int GetPackFlag(const void *buf)
    {
        return *(int*)buf;
    }
    inline static unsigned char GetPackVer(const void* buf)
    {
        return ((char*)buf)[10];
    }
    inline static unsigned short GetCheckSum(const void* buf)
    {
        return *(unsigned short*)&((char*)buf)[15];
    }


    inline static void SetCheckSum(void *buf, int packLen)
    {
        char *pBuf = (char*)buf;
        *(unsigned short*)&pBuf[15] = 0;
    }


public:
    static int build(void *buf, int bufBytes, int identifyCode, unsigned short interfaceType, const void*content, int contentBytes);

    static FNetPackHeader GetPackHeader(void *buf);

    static int fillWidthString(void *pBuf, int bufSize,const char*content, unsigned char contenLen);


};


#endif //UDTDEMO_UTILMAKEPACK_H
