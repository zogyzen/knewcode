#pragma once

#include "std.h"

class CWSProxyServerCB;
class CApiWork
{
public:
    CApiWork(void);
    ~CApiWork(void);

    // 处理请求
    void Work(intptr_t r);
    // 判断是否ssl
    bool IsSSL(intptr_t c);
    // 得到模块上下文
    IBundleContext& GetContext(void) const
    {
        return m_proxy.GetContext();
    }

public:
    // 设置回调函数指针
    static int SetCBFuncPointer(int argCount, string flag, void* func);
    // 检查重要的回调函数是否设置
    static bool CheckVitalCBFuncPtr(void);

protected:
    // 获取服务器配置信息的回调函数指针类型。参数依次为：3；"FGetSrvInfoCB"；信息名称。返回信息内容。
    static FGetSrvInfoCB s_fGetSrvInfoCB;

    // 获取链接信息的回调函数指针类型。参数依次为：4；"FGetConnInfoCB"；链接的句柄；信息名称。返回信息内容。
    static FGetConnInfoCB s_fGetConnInfoCB;

    // 获取请求信息（请求头信息、get参数等）的回调函数指针类型。参数依次为：4；"FGetRequestHeadCB"；请求的句柄；信息名称。返回信息内容。
    static FGetRequestHeadCB s_fGetRequestHeadCB;
    // 获取请求内容（post参数、多表单数据、其他二进制流等）的回调函数指针类型。参数依次为：3；"FGetRequestBodyCB"；请求的句柄。返回内容。
    // 内容的类型可通过头名称“content_type”获取，内容的长度可通过“content_length”获取。
    static FGetRequestBodyCB s_fGetRequestBodyCB;
    // 获取请其他求信息（get参数等）的回调函数指针类型。参数依次为：4；"FGetRequestOtherCB"；请求的句柄；信息名称。返回信息内容。
    static FGetRequestOtherCB s_fGetRequestOtherCB;
    // 获取请求的链接句柄的回调函数指针类型。参数依次为：3；"FGetRequestConnCB"；请求的句柄。返回链接句柄。
    static FGetRequestConnCB s_fGetRequestConnCB;

    // 设置应答头信息的回调函数指针类型。参数依次为：5；"FSetRespondHeadCB"；请求的句柄；头名称；内容（为空时，删除）。返回错误码：0无错误。
    static FSetRespondHeadCB s_fSetRespondHeadCB;
    // 设置应答内容的回调函数指针类型。参数依次为：6；"FSetRespondBodyCB"；请求的句柄；内容的类型（json数据、二进制流等）；内容；内容的字节数。返回错误码：0无错误。
    static FSetRespondBodyCB s_fSetRespondBodyCB;
    // 设置其他应答信息（200或500状态等）的回调函数指针类型。参数依次为：5；"FSetRespondOtherCB"；请求的句柄；头名称；内容（为空时，删除）。返回错误码：0无错误。
    static FSetRespondOtherCB s_fSetRespondOtherCB;
    // 设置提交应答的回调函数指针类型。参数依次为：3；"FSetCommitResponseCB"；请求的句柄。返回错误码：0无错误。
    static FSetCommitResponseCB s_fSetCommitResponseCB;

    // 设置通过错误码判断是否掉线的回调函数指针类型。参数依次为：3；"FSetIsDisconnByErrCodeCB"；错误码。返回：true已掉线；false未掉线。
    static FSetIsDisconnByErrCodeCB s_fSetIsDisconnByErrCodeCB;
    // 设置通过错误码得到错误信息的回调函数指针类型。参数依次为：3；"FSetGetStatusByErrCodeCB"；错误码。返回错误信息。
    static FSetGetStatusByErrCodeCB s_fSetGetStatusByErrCodeCB;

    // 设置链接为长连接的回调函数指针类型。参数依次为：4；"FSetKeepaliveCB"；链接的句柄；启用或关闭长连接。返回错误码：0无错误。
    static FSetKeepaliveCB s_fSetKeepaliveCB;
    // 设置连接断开事件的回调函数。参数依次为：5；"FSetBreakConnCB"；链接的句柄；回调的实例句柄；回调函数。返回错误码：0无错误。
    static FSetBreakConnCB s_fSetBreakConnCB;
    // 关闭连接的回调函数。参数依次为：3；"FCloseConnCB"；链接的句柄。返回错误码：0无错误。
    static FCloseConnCB s_fCloseConnCB;
    // 得到是否关闭连接的回调函数。参数依次为：3；"FIsCloseConnCB"；链接的句柄。返回：true连接已关闭；false未关闭。
    static FIsCloseConnCB s_fIsCloseConnCB;
    // 重置链接时间的回调函数。参数依次为：4；"FResetConnTimeCB"；链接的句柄；超时的时间（毫秒）。返回错误码：0无错误。
    static FResetConnTimeCB s_fResetConnTimeCB;
    // 通过链接发送数据的回调函数。参数依次为：5；"FSendByConnCB"；链接的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
    static FSendByConnCB s_fSendByConnCB;
    // 通过链接进行Socket发送数据的回调函数。参数依次为：5；"FSockSendByConnCB"；链接的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
    static FSockSendByConnCB s_fSockSendByConnCB;
    // 通过链接进行websocket接收数据的回调函数。参数依次为：5；"FWscRecvByConnCB"；请求的句柄；接收缓冲器；缓存区最大字节数。返回实际接收数据的字节数。
    static FWscRecvByConnCB s_fWscRecvByConnCB;

public:
    // 获取服务器配置信息
    static const char* GetSrvInfo(const char*);
    // 设置目录
    string GetSrvDir(const char* name, string def = "")
    {
        string sPth = CUtilFunc::PCharSafeToStr(GetSrvInfo(name));
        if (sPth.empty() && !def.empty()) sPth = def;
        return CUtilFunc::ToAbsPath(sPth, m_WebSrvPath);
    }

    // 获取链接信息
    static const char* GetConnInfo(intptr_t, const char*);

    // 获取请求头信息
    static const char* GetRequestHead(intptr_t, const char*);
    // 获取请求内容
    static std::tuple<unsigned, const char*> GetRequestBody(intptr_t);
    // 获取请其他求信息（get参数等）
    static const char* GetRequestOther(intptr_t, const char*);
    // 获取请求的链接句柄
    static intptr_t GetRequestConn(intptr_t);

    // 设置应答头信息
    static int SetRespondHead(intptr_t, const char*, string);
    // 设置应答内容
    static int SetRespondBody(intptr_t, string);
    // 设置应答头信息
    static int SetRespondOther(intptr_t, const char*, string);
    // 提交应答
    static int CommitResponse(intptr_t);

    // 通过错误码判断是否掉线
    static bool IsDisconnByErrCode(int);
    // 通过错误码得到错误信息
    static const char* GetStatusByErrCode(int);

    // 设置链接为长连接
    static int SetKeepalive(intptr_t, bool);
    // 设置连接断开事件
    static int SetBreakConnCB(intptr_t, intptr_t, FBreakConnCBFromOwn);
    // 关闭连接
    static int CloseConn(intptr_t);
    // 得到是否连接
    static bool IsCloseConn(intptr_t);
    // 重置链接时间
    static int ResetConnTime(intptr_t, unsigned);
    // 通过链接发送数据
    static int SendByConn(intptr_t, const char*, unsigned);
    // 通过链接进行Socket发送数据
    static int SockSendByConn(intptr_t, const char*, unsigned);
    // 通过链接进行websocket接收数据
    static int WscRecvByConn(intptr_t, char*, unsigned);

protected:
    // 设置回调函数指针
    template<typename TFuncPtr>
    static bool SetCBFuncPointer(TFuncPtr& dst, int dstArgCount, int argCount, string flag, void* func)
    {
        if (dstArgCount == argCount && nullptr != func)
        {
            dst = reinterpret_cast<TFuncPtr>(func);
            return true;
        }
        else
        {
            string sMsg = (boost::format("%s - %d // %d  %X") % flag % dstArgCount % argCount % func).str();
            string sFxPth = GetSrvInfo("KnewcodeRoot");
            if (!sFxPth.empty())
                CTempLog::WriteInDir(sFxPth, "Set Call Back Function Pointer Error", __CURR_CODE_PLACE_C__, sMsg);
            else
                CTempLog::Write("Set Call Back Function Pointer Error", __CURR_CODE_PLACE_C__, sMsg);
            return false;
        }
    }

private:
    // 虚拟目录
    map<string, string> m_vPath;
    // web服务器软件（apache、nginx、tomcat等）名称和版本、启动程序目录、主平台目录、网站或应用根路径、KC框架路径、主页目录、配置文件、虚拟目录
    string m_ownName, m_ownVersion, m_WebSrvPath, m_PlatformPath, m_WebsitePath, m_FxPath, m_PgPath, m_CfgFile, m_virtualPath;
    // Web服务类
    CWSProxyServerCB &m_srvCB;
    // 代理接口
    LoadWebSrvProxy m_load;
    IKCWebSrvProxy& m_proxy;

    // 友元
    friend class CWSProxyServerCB;
    friend class CWSProxyConnectCB;
    friend class CWSProxyRequestCB;
};
