#pragma once

#include "std.h"

class KCWebSrvProxy;

// Web链接
class TKCWebConn : public IKCWebLongConn
{
public:
    TKCWebConn(KCWebSrvProxy&, IWSProxyRequestCB&);
    ~TKCWebConn(void) override;

    // 链接编号
    long CALL_TYPE GetID(void) override;
    const char* CALL_TYPE GetUniqueID(void) override;
    // 设置接收消息回调接口
    void CALL_TYPE SetRecvIF(IKCWebLongConn::IRecvCB* = nullptr) override;
    // 请求应答上下文
    IBaseRequestRespond& CALL_TYPE MakeRe(void) override;
    void CALL_TYPE ReleaseRe(IBaseRequestRespond&) override;
    // 启动
    void CALL_TYPE Start(void) override;

public:
    // 关闭前锁住
    void LockForClose(void);

protected:
    // 断开链接的回调函数
    struct TDisconData
    {
        TKCWebConn *pWebConn = nullptr;
        TDisconData(TKCWebConn *p) : pWebConn(p) {}
    };
    TDisconData *m_pCdb = nullptr;
    static bool DisconCB(TDisconData* cdb);
    // 返回状态
    IKCWebLongConn::EWebConnStatus GetStatus(int, string, string = "");

protected:
    KCWebSrvProxy& m_own;
    IWSProxyConnectCB& m_connCB;
    string m_uniqueID;
    IKCWebLongConn::IRecvCB *m_recvCB = nullptr;
    char m_recvBuf[c_KCMaxParmBufSize + 1] = { 0 };
    // 关闭前的锁和信号量
    boost::mutex m_mtxForClose;
    boost::condition_variable_any m_condForClose;
};

// websocket链接
class TWebSrvConnWebsocket : public TKCWebConn
{
public:
    TWebSrvConnWebsocket(KCWebSrvProxy&, IWSProxyRequestCB&);
    ~TWebSrvConnWebsocket(void) override;

    // 启动
    void CALL_TYPE Start(void) override;
    // 发消息
    IKCWebLongConn::EWebConnStatus CALL_TYPE Send(const char*, unsigned) override;

protected:
    // 接收数据的线程
    static void RecvInfoThrd(boost::weak_ptr<IKCWebLongConn>);
    // 接收数据
    bool RecvData(void);

protected:
    // 接收线程
    boost::shared_ptr<boost::thread> m_recvThrd;
};

// SSE链接
class TWebSrvConnSSE : public TKCWebConn
{
public:
    TWebSrvConnSSE(KCWebSrvProxy&, IWSProxyRequestCB&);

    // 发消息
    IKCWebLongConn::EWebConnStatus CALL_TYPE Send(const char*, unsigned) override;
};
