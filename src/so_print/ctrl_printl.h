#pragma once

#include "std.h"

namespace KC
{
    class CCtrlPrint : public IKCController
    {
    public:
        CCtrlPrint(string sPth, string sName);
        ~CCtrlPrint() override;

        // 执行控制器
        void CALL_TYPE Perform(ICtrlData&, IKCController::IAttachParm&) override;

    private:
        string m_path;
        int m_argc = 1;
        char *m_argv = nullptr;
        QCoreApplication m_qtApp;
        std::thread m_thrdQtExec;
    };
}
