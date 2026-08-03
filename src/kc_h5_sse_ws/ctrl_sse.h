#pragma once

#include "std.h"

class CCtrlSSE : public IKCController
{
public:
    CCtrlSSE(IKCSSEWS& own, string sName, property_tree::ptree&);
    ~CCtrlSSE() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

public:
    // 宿主
    IKCSSEWS& own(void);
    // 发消息
    int SendMxMessage(int pid, long wcid, int type, string msg);

protected:
    // 宿主
    IKCSSEWS& m_own;
    string m_name;
    // 类型
    string m_type = "sse";
    // 标记（用于队列名称）
    string m_flag = "";
    // 发送消息队列名
    string m_sendMQname;
    // 发送消息队列大小
    int m_sendMQsize = 1024;
    // 中转的session控制器
    string m_sessionCtrl = "global";
    // 锁
    boost::shared_mutex m_mtx;
    // 是否ping
    bool m_runPing = true;
    // 是否运行
    std::atomic_bool m_running;
    // 最大消息编号
    static std::atomic_ullong m_msgMaxID;

protected:
    // 注册链接
    virtual void RegConn(ICtrlApiData&, map<string, string>&);
    // 发送消息
    void SendMsg(ICtrlApiData&, map<string, string>&);
    // 消息队列名称
    string MQname(int pid);
    // 初始化
    bool Init(property_tree::ptree& pt);
    // session接口
    IKCSessionCookie& Session(IServiceReference*&);
    // 守护线程
    boost::thread m_GuardThrd;
    void GuardThrd(void);
    // 消息队列线程
    boost::thread m_MQThrd;
    void MQThrd(void);
    // ping线程
    boost::thread m_PingThrd;
    void PingThrd(void);

protected:
    // send消息
    struct TSendMQInfo
    {
        long m_wcid = 0;
        char m_type = 1;    // 1:文本。9：ping。88：session中转。99：激活。
        char m_size = 0;    // 0~126：表示m_info的数据长度。127表示m_info是数据的session名称
        char m_info[126] = { 0 };
    };
    // 消息描述
    string SMQInfoToStr(TSendMQInfo& mxm, string sPos);
    // 服务器发送
    virtual void SrvSend(TSendMQInfo& mxm);
    // 调用子控制器（取消链接）
    void CallSubCtrl(string, string);

protected:
    // SSE链接
    class TSSEconn : public IKCWebLongConn::IRecvCB
    {
    public:
        TSSEconn(CCtrlSSE&, IKCWebLongConn&, int = 30);
        ~TSSEconn(void) override;

        // 接收信息
        void CALL_TYPE Recv(char*, unsigned) override;
        // 断开链接
        void CALL_TYPE Discon(void) override;
        // 错误日志
        bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const override;
        // 其他指令
        void CALL_TYPE Cmd(const char*, const char*) override;

    public:
        // 链接
        IKCWebLongConn& WC(void);
        // 置为无效
        string SetDisable(void);
        // 调用子控制器（取消或激活链接）
        int CallSubCtrl(string);

    public:
        CCtrlSSE &m_own;
        IKCWebLongConn *m_wc = nullptr;
        const int m_keepalive_secs = 30;
        const long m_wcid = 0;
        const string m_sid;     // sse和websocket的编号
        posix_time::ptime m_pt = posix_time::microsec_clock::local_time();
        enum EWSStatus { ewsInvalid = 0, ewsProcess, ewsValid };
        std::atomic_int m_status;
    };
    // 保存链接
    std::map<long, std::shared_ptr<TSSEconn>> m_wcs;
};
