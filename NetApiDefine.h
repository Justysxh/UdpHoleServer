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
	,Fun_PeerLogout  //盒子退出
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

#endif /* NetApiDefine_h */
