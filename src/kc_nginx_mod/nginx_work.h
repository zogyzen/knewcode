#pragma once

#include "std.h"

// nginx 工作类
class CWSProxyServerCB;
class CNginxWork
{
public:
    CNginxWork(void);
    ~CNginxWork(void);

    // 初始化
    void Init(void);
    // 释放
    void Free(void);

    // 处理请求
    int Work(TNgxRequestData &r);

    // 自己的弱引用
    static std::weak_ptr<CNginxWork> m_self;

protected:
    // 获取服务器配置信息
    const char* GetSrvInfoCB(string name);
    // 获取链接信息
    const char* GetConnInfoCB(intptr_t hConn, string name);
    // 获取请求头信息
    const char* GetRequestHeadCB(intptr_t hRequest, string name);
    // 获取请求内容（post参数、多表单数据、其他二进制流等）
    const char* GetRequestBodyCB(intptr_t hRequest);
    // 获取请其他求信息（get参数、本地文件等）
    const char* GetRequestOtherCB(intptr_t hRequest, string name);
    // 获取请求的链接句柄
    intptr_t GetRequestConnCB(intptr_t hRequest);
    // 设置应答头信息
    int SetRespondHeadCB(intptr_t hRequest, string name, string val);
    // 设置应答内容
    int SetRespondBodyCB(intptr_t hRequest, string content);
    // 设置其他应答信息（200或500状态等）
    int SetRespondOtherCB(intptr_t hRequest, string name, string val);
    // 提交应答
    int CommitResponseCB(intptr_t hRequest);
    // 通过错误码判断是否掉线
    bool IsDisconnByErrCodeCB(int errCode);
    // 通过错误码得到错误信息
    const char* GetStatusByErrCodeCB(int errCode);
    // 设置链接为长连接
    int SetKeepaliveCB(intptr_t hConn, bool enable);
    // 设置连接断开事件
    int SetBreakConnCB(intptr_t hConn, intptr_t hInstance, FBreakConnCBFromOwn fcb);
    // 关闭连接
    int CloseConnCB(intptr_t hConn);
    // 得到是否关闭连接
    bool IsCloseConnCB(intptr_t hConn);
    // 重置链接时间
    int ResetConnTimeCB(intptr_t hRequest, unsigned ms);
    // 通过链接发送数据
    int SendByConnCB(intptr_t hRequest, string buf);
    // 通过链接进行Socket发送数据
    int SockSendByConnCB(intptr_t hConn, string buf);
    // 通过链接进行websocket接收数据
    int WscRecvByConnCB(intptr_t hConn, char* buf, unsigned len);
    // post请求，获取请求体后的处理
    static void GetClientBodyHandler(void* rSrc);

protected:
    // 判断是否ssl
    bool IsSSL(TNgxConnection&);
    // 获取服务器配置信息
    const char* GetSrvInfo(string sName);
    // 获取链接信息
    const char* GetConnInfo(TNgxConnection& conn, string sName);
    // 按名称得到请求头
    const char* GetRequestHeader(TNgxRequestData&, string sName) const;
    const char* GetRequesOther(TNgxRequestData&, string sName) const;
    // 获取客户端post输入
    const char* GetClientBlock(TNgxRequestData&) const;
    // 按名称设置响应头（值为空，删除）
    void SetResponseHeader(TNgxRequestData&, const char* name, const char* val);
    // 输出网页内容
    bool AddResponseBody(TNgxRequestData&, const char* buf, int nbyte = 0);
    // 获取链接
    // apr_socket_t& Sock(conn_rec&);
    // 设置长连接
    void SetKeepalive(/*conn_rec&, */bool b);
    // 端口回调
    struct TBreakCB
    {
        void *cdb = nullptr;
        FBreakConnCBFromOwn fBreakCB = nullptr;
    };
    // static apr_status_t DisconCB(TBreakCB* brcb);
    // 设置断开的回调函数
    void SetBreakCB(/*conn_rec&, */void* cdb, FBreakConnCBFromOwn fcb);
    // websocket接收
    int WscRecv(/*request_rec&, */char* buf, unsigned len);

private:
    // nginx根目录、KC框架路径、配置文件、主页目录、虚拟目录、平台根路径、网站或应用根路径
    const string m_NgxPath, m_FxPath, m_CfgFile, m_PgPath, m_VPath, m_PlatformPath, m_WebsitePath;
    // 当前线程的请求数据
    static inline thread_local TNgxRequestData* t_ngxReqData = nullptr;

    // 框架目录
    string FxPath(void) { return m_FxPath; }

public:
    // 代理接口
    typedef LoadWebSrvApi<CNginxWork> TLoadWebSrvApi;
    TLoadWebSrvApi m_load;
    friend TLoadWebSrvApi;
};
extern std::shared_ptr<CNginxWork> g_work;     // 主框架
extern void WriteLog(int lv, string sFlag, string sErr, string sPos);   // 写日志函数
