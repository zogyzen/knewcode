#pragma once

namespace KC
{
    class CCtrlSoap : public IKCController
    {
    public:
        CCtrlSoap(string sName);
        CALL_TYPE ~CCtrlSoap() override;

        // 执行控制器
        void CALL_TYPE Perform(const char*, const char*, const char*, const char*, IKCRequestRespond& re, ICtrlNodeData&, IKCController::IAttachParm&) override;

    private:
        string m_name;
        string m_server;
    };
}
