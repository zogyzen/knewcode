#pragma once

#include "ctrl_sse.h"

class CCtrlWS : public CCtrlSSE
{
public:
    CCtrlWS(IKCSSEWS& own, string sName, property_tree::ptree&);

protected:
    // 服务器发送
    void SrvSend(TSendMQInfo& mxm) override;
    // 注册链接
    void RegConn(ICtrlApiData&, map<string, string>&) override;

protected:
    // SSE链接
    class TWSconn : public CCtrlSSE::TSSEconn
    {
    public:
        TWSconn(CCtrlWS&, IKCWebLongConn&, int = 30);

        // 接收信息
        void CALL_TYPE Recv(char*, unsigned) override;
        // 其他指令
        void CALL_TYPE Cmd(const char*, const char*) override;
    };
};
