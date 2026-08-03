#include "std.h"
#include "ctrl_token.h"

CCtrlToken *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* dir, const char* name)
    {
        if (nullptr == g_ctrl) g_ctrl = new CCtrlToken(name);
        return *g_ctrl;
    }
    void CALL_TYPE UninitActor(IKCController&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlToken类
CCtrlToken::CCtrlToken(string sName) : m_name(sName)
{
}

CCtrlToken::~CCtrlToken()
{
}

// 执行控制器
void CCtrlToken::Perform(const char* pLocalFile, const char* pAct, const char* pMethod, const char* pContext, IKCRequestRespond& re, ICtrlNodeData&, IKCController::IAttachParm&)
{
    re.AddCookie("token", "123");
}
