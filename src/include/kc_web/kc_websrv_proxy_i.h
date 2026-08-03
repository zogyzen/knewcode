#pragma once

#include "kc_web/kc_main_work_i.h"
#include "framework/bundle_context_i.h"
#include "kc_web/web_struct.h"

namespace KC
{
    // web请求回调接口
    class IWSProxyRequestCB
    {
    public:
        // 设置请求应答接口
        virtual void CALL_TYPE SetRe(ISrcRequestRespond&) = 0;
        // 得到本地完整文件名
        virtual const char* CALL_TYPE GetLocalFilename(void) const = 0;
        virtual const char* CALL_TYPE GetLocalFilename(const char*) const = 0;
        // 得到网络根路径
        virtual const char* CALL_TYPE GetUrlPageRootPath(const char*) const = 0;
        // 虚拟目录
        virtual unsigned CALL_TYPE VirtualPathCount(void) const = 0;
        virtual const char* CALL_TYPE GetVirtualPath(unsigned) const = 0;
        virtual const char* CALL_TYPE GetVirtualPathUri(unsigned) const = 0;
        // 判断是否ssl
        virtual bool CALL_TYPE IsSSL(void) const = 0;
        // 得到服务器端各单个请求信息
        virtual const char* CALL_TYPE GetSingleInfo(const char* pName, const char* pDef = "") const = 0;
        // 是否post请求
        virtual bool CALL_TYPE IsPost(void) const = 0;
        // 按名称得到请求头
        virtual const char* CALL_TYPE GetRequestHeader(const char* pName) const = 0;
        // 获取客户端post输入
        virtual unsigned CALL_TYPE GetClientBlock(char*, unsigned) const = 0;
        // 设置应答类型
        virtual void CALL_TYPE SetContentType(const char*) = 0;
        // 设置响应状态
        virtual void CALL_TYPE SetResponseStatus(int iStt) = 0;
        // 按名称添加、删除响应头
        virtual void CALL_TYPE AddResponseHeader(const char* name, const char* val) = 0;
        virtual void CALL_TYPE DelResponseHeader(const char* name) = 0;
        // 输出网页内容
        virtual bool CALL_TYPE AddResponseBody(const char* buf, int nbyte = 0) = 0;
        // 提交响应（将这段时间，服务器端的输出，同时推给客户端）
        virtual void CALL_TYPE CommitResponse(void) = 0;

    protected:
        virtual ~IWSProxyRequestCB(void) = default;
    };

    // web连接回调接口
    class IWSProxyConnectCB
    {
    public:
        // 获取连接ID
        virtual long CALL_TYPE GetID(void) const = 0;
        // 判断是否ssl
        virtual bool CALL_TYPE IsSSL(void) const = 0;
        // 设置长连接
        virtual void CALL_TYPE SetKeepalive(bool = true) = 0;
        // 设置断开的回调函数
        typedef bool(*FBreakCB)(void*);
        virtual void CALL_TYPE SetBreakCB(void*, FBreakCB) = 0;
        // 关闭连接
        virtual void CALL_TYPE CloseConn(void) = 0;
        // 连接是否关闭
        virtual bool isCloseConn(void) const = 0;
        // 请求应答上下文的回调
        virtual IWSProxyRequestCB& CALL_TYPE MakeReCB(void) = 0;
        virtual void CALL_TYPE ReleaseReCB(IWSProxyRequestCB&) = 0;
        // 重置链接时间
        virtual void CALL_TYPE ResetTime(unsigned ms) = 0;
        // 发送
        virtual int CALL_TYPE Send(const char* buf, unsigned len) = 0;
        // socket发送
        virtual int CALL_TYPE SockSend(const char* buf, size_t& sz) = 0;
        // websocket接收
        virtual int CALL_TYPE WscRecv(char* buf, size_t& sz) = 0;

    protected:
        virtual ~IWSProxyConnectCB(void) = default;
    };

    // web服务回调接口
    class IWSProxyServerCB
    {
    public:
        // 宿主应用程序名称
        virtual const char* CALL_TYPE OwnName(void) const = 0;
        // 宿主应用程序版本
        virtual const char* CALL_TYPE OwnVersion(void) const = 0;
        // web服务程序的根目录
        virtual const char* CALL_TYPE apPath(void) const = 0;
        // 主平台根目录
        virtual const char* CALL_TYPE platformPath(void) const = 0;
        // 网站或应用根路径
        virtual const char* CALL_TYPE websitePath(void) const = 0;
        // KC系统根目录
        virtual const char* CALL_TYPE fxPath(void) const = 0;
        // 主页根目录
        virtual const char* CALL_TYPE pgPath(void) const = 0;
        // 配置文件完整路径
        virtual const char* CALL_TYPE cfgFile(void) const = 0;
        // web虚拟目录
        virtual const char* CALL_TYPE VirtualPath(void) const = 0;
        // 链接
        virtual IWSProxyConnectCB& CALL_TYPE MakeConnCB(IWSProxyRequestCB&) = 0;
        virtual void CALL_TYPE ReleaseConnCB(IWSProxyConnectCB&) = 0;
        // 按状态判断是否断线
        virtual bool isDisconn(int stt) const = 0;
        // 获取状态的错误信息
        virtual const char* GetStatus(int) const = 0;

    protected:
        virtual ~IWSProxyServerCB(void) = default;
    };

    // web应用服务器代理接口
    class IKCWebSrvProxy : public IKCStartWork
    {
    public:
        // 初始化
        virtual void CALL_TYPE Init(void) = 0;
        virtual void CALL_TYPE Free(void) = 0;
        // 处理请求
        virtual void CALL_TYPE Work(IWSProxyRequestCB&) = 0;
        // 静态页面处理
        virtual void CALL_TYPE StaticPage(IWSProxyRequestCB&) = 0;
        // 链接关闭前
        virtual void CALL_TYPE PreClose(IWSProxyConnectCB&) = 0;
        // 得到模块上下文
        virtual IBundleContext& GetContext(void) const = 0;

    protected:
        ~IKCWebSrvProxy(void) override = default;
    };

    constexpr const char c_proxyMakeFuncName[] = "makeProxy";
    constexpr const char c_proxyReleaseFuncName[] = "releaseProxy";
}

