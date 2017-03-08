//
//  NetApiDefine.h
//  FProject
//
//  Created by wdkjapp on 2016/12/26.
//  Copyright © 2016年 UNICOMCQ. All rights reserved.
//

#ifndef NetApiDefine_h
#define NetApiDefine_h

#include <stdint.h>
#include <string.h>
#include <string>
#include <sstream>

#define PACK_FALG 0x07363110   //包头标识
#define PTC_VER  1

void fnetPrintf(const char* pstrFormat, ...);

#ifdef __ANDROID__
#include <android/log.h>
#define myprintf(...) __android_log_print(ANDROID_LOG_INFO,"FPlus",__VA_ARGS__)
#else
#define myprintf(...) fnetPrintf(__VA_ARGS__)
#endif

void printBuffHexString(const void *pBuf, int len);

//包功能字段
enum enumFunFlag: short
{
    Fun_Unknow = 0
    ,Fun_SearchBox = 1 //搜索盒子
    ,Fun_SearchBoxResponse=2
    ,Fun_BindBox = 3 //绑定命盒子
    ,Fun_BindBoxResponse=4
    ,Fun_CheckConnect=5 //内网连接盒子
    ,Fun_CheckConnectResponse=6
    ,Fun_FileList = 7 //获取文件列表
    ,Fun_FileListResponse=8
    ,Fun_DelFile=9 //删除文件
    ,Fun_DelFileResponse=10
    ,Fun_FileTranslate = 11 //文件传输请求
    ,Fun_FileTranslateResponse=12
    ,Fun_FileData //文件数据
    ,Fun_LocalConnect  //本地广播连接盒子
    ,Fun_LocalConnectResponse //本地连接盒子响应
    ,Fun_Hole = 0xF0  //打洞请求
    ,Fun_HoleResponse //打洞响应
    ,Fun_HoleHeart  //打洞时心跳
    ,Fun_HoleHeartResponse  //打洞时心跳响应
    ,Fun_IpAndPort//盒子上报ip和端口
    ,Fun_IpAndPortResponse  //响应盒子上报
    ,Fun_P2P    //p2p打洞
    ,Fun_ConnectFor  //决定连接类型.
};

enum enumErrorCode
{
    FE_NOERROR = 0
    ,FE_UDT_BROKEN  //udt连接断开
};

enum enumConnectFor
{
    ConnectFor_Command=0  //连接是用于命令通道
    ,ConnectFor_FileDownload //连接用于文件下载
    ,ConnectFor_FileUpload //连接用于文件上传
};

int GetPackFlag(const void *buf);
int FindPackFlag(const void *pBuf,int len);
unsigned char GetPackVer(const void* buf);
unsigned short GetCheckSum(const void* buf);
int GetPackLen(const void *buf);
short GetPackFun(const void* buf);
int GetPackID(const void* buf);

unsigned int GetPackInt(const void *pBuf, int offset);
unsigned short GetPackShort(const void*pBuf, int offset);

int GetPackHeadLen();

void SetPackFlag(void *buf, int flag);
void SetPackVer(void* buf, unsigned char ver);
void SetPackLen(void*buf, int packLen);
void SetPackFun(void*buf, short fun);
void SetPackID(void*buf, int packID);
void SetCheckSum(void *buf, int packLen);

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

FNetPackHeader GetPackHeader(void *buf);

/**
*回调接口类
*在接口中使用的时候外部需要new出来,传入函数, 由系统自动delete.
*当FNetCallback内部的回函数返回true时,系统会自动delete释放
*/

class FNetCallback
{
protected:
    static int sIDSeed;
    int mCallbackID;
public:
    static int makeID()
    {
        int tID = sIDSeed;
        ++sIDSeed;
        return tID;
    }
    virtual ~FNetCallback()
    {

    }

    //当api返回true,表示请求已经处理完成, 一般情况都需要返回true.
    virtual bool onFNetEvent(int connectID,bool bNetSuccess, FNetPackHeader *header, const char* result, int len)
    {
        return true;
    }
    virtual int getIdentifyCode()
    {
        return mCallbackID;
    }
    virtual bool isIdMatch(int pid)
    {
        return pid == mCallbackID;
    }
    virtual void preResponse(){}
    virtual void afterResponse(){}
};

//客户端连接状态通知(全局只有一个)
class IClientConnectNotify
{

public:
    virtual  ~IClientConnectNotify(){}
    virtual void onConnected(int connectID,const char*ip, int port,const char*user)=0;
    virtual void onBroken(int connectID ,const char*ip, int port,const char*user)=0;
};


class CJsonParam
{
private:
    std::string mStr;
    char emptyStr[16];
public:
    CJsonParam()
    {
        strcpy(emptyStr," ");
    }
    void add(const char* jsonStr)
    {
        std::ostringstream str;
        if(mStr.empty()==false)
        {
            str<<',';
        }
        if(jsonStr[0]=='{')
        {
            ++jsonStr;
        }
        str<<jsonStr;
        std::string tStr = str.str();
        char ch = tStr.at(tStr.length()-1);
        if(ch == '}')
        {
            tStr.erase(tStr.begin()+(tStr.length()-1));
        }
        mStr.append(tStr);
    }
    void add(const char *key, const char *value)
    {
        if(value==NULL)
        {
            value = emptyStr;
        }
        std::ostringstream str;
        if(mStr.empty() == false)
        {
            str << ',';
        }
        str << '\"' << key << "\":\"" << value << "\"";
        mStr.append(str.str());
    }

    void add(const char*key, long long val)
    {
        std::ostringstream str;
        if(mStr.empty() == false)
        {
            str << ',';
        }
        str << '\"' << key << "\":" << val;
        mStr.append(str.str());
    }
    void addBool(const char*key, bool val)
    {
        std::ostringstream str;
        if(mStr.empty() == false)
        {
            str << ',';
        }
        str << '\"' << key << "\":" << (val == true ? "true" : "false");
        mStr.append(str.str());
    }
    /**
     * value已经有双引号
     */
    void addNS(const char *key, const char *value)
    {
        if(value==NULL)
        {
            value = emptyStr;
        }
        std::ostringstream str;
        if(mStr.empty() == false)
        {
            str << ',';
        }
        str << '\"' << key << "\":" << value;
        mStr.append(str.str());
    }
    void addAsSub(const char*key, CJsonParam &val)
    {
        std::ostringstream str;
        if(mStr.empty() == false)
        {
            str << ',';
        }
        str << '\"' << key << "\":" << val.toString();
        mStr.append(str.str());
    }

    void addAsArray(const char*key, const char*val)
    {
        if(val==NULL)
        {
            val = emptyStr;
        }
        std::ostringstream str;
        if(mStr.empty() == false)
        {
            str << ',';
        }
        str << '\"' << key << "\":[" << val<<"]";
        mStr.append(str.str());
    }


    std::string toString()
    {
        std::string str;
        str.append("{");
        str.append(mStr);
        str.append("}");
        return str;
    }

    int length()
    {
        return mStr.length()+2;
    }

};

//如下两个函数是在线程开始和结束时调用,
//因为Android中,线程需要附加到JVM虚拟机中,才能操作Java对象,这两个函数在Android端的实现,就是把线程附加到JVM
//iOS端只需要做一个空实现即可.
bool ThreadInit();
void ThreadRelease(bool bInit);

//android 空实现  IOS端需要设置缓冲区大小
void externUdtSockInit(int udtSock);

int GenPack(void *buf, int bufBytes, int identifyCode, unsigned short interfaceType, const void*content, int contentBytes);
void GenPackHead(void *buf, unsigned short fun, int id);

//生成手机查找盒子广播包
int GenBroadcastPack(void*buf, int bufBytes,int pid,const char*ip, int port,int st);
//生成手机查找盒子广播响应包
int GenBroadcastResponsePack(void*buf, int bufBytes,int pid,const char*ip, int port,int st,const char*name,const char*code);

int GenLocalConnect(void *pBuf, int bufSize, int pid, const char*localIP, int localPort, int udtPort,int st, const char*user,const char*code);

//响应本地连接广播包
int GenLocalConnectResponse(void *pBuf, int bufSize,int pid,bool bSuccsee,const char*msg,
                            const char*localIP, int localPort, const char*name, const char* code);

int GenMsgOnlyJson(void *pBuf, int bufSize, bool bSuccess, const char*msg);



#endif /* NetApiDefine_h */
