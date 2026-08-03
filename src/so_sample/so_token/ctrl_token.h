#pragma once

namespace KC
{
    class CCtrlToken : public IKCController
    {
    public:
        CCtrlToken(string sName);
        CALL_TYPE ~CCtrlToken() override;

        // 执行控制器
        void CALL_TYPE Perform(const char*, const char*, const char*, const char*, IKCRequestRespond& re, ICtrlNodeData&, IKCController::IAttachParm&) override;

    private:
        string m_name;
        string m_server;
    };
}
