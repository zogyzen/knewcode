#include "ctrl_helloworld.h"

CCtrlHelloWorld *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* /*dir*/, const char* name)
    {
        if (nullptr == g_ctrl) g_ctrl = new CCtrlHelloWorld(name);
        return *g_ctrl;
    }
    void CALL_TYPE UninitActor(IKCController&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlHelloWorld类
CCtrlHelloWorld::CCtrlHelloWorld(string sName) : m_name(sName)
{
}

// 执行控制器
void CCtrlHelloWorld::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    IActionData& act = objCtrlD.ActionData();
    try
    {
        objCtrlD.JsonRespond().SetStr("content", "Hello World");
    }
    catch(std::exception& e)
    {
        act.WriteLogError(act.GetHint("Action_page_"), __FUNCTION__, e.what());
    }
    catch (...)
    {
        act.WriteLogError(act.GetHint("Action_page_"), __FUNCTION__);
    }
}
