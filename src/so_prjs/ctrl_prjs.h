#pragma once

#include "std.h"

namespace KC
{
    class CCtrlPrjs : public IKCController
    {
    public:
        CCtrlPrjs(string sName);
        ~CCtrlPrjs() override;

        // 执行控制器
        void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

    protected:
        // 创建项目
        void CreatePrj(ICtrlApiData&, map<string, string>&);

    protected:
        // git命令的路径
        string GitExe(string sRootDir);

    private:
        string m_name;
        string m_server;
    };
}
