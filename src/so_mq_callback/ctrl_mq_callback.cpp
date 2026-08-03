#include "ctrl_mq_callback.h"

CCtrlWebMQCallback *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* dir, const char* name)
    {
        if (nullptr == g_ctrl)
            g_ctrl = new CCtrlWebMQCallback(CUtilFunc::PCharSafeToStr(dir), CUtilFunc::PCharSafeToStr(name));
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
CCtrlWebMQCallback::CCtrlWebMQCallback(string sDir, string sName) : m_dir(sDir), m_name(sName)
{
}

// 执行控制器
void CCtrlWebMQCallback::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    IActionData& act = objCtrlD.ActionData();
    try
    {
        // 获取消息队列配置
        const IKCJson& jsonAttach = objCtrlD.JsonAttach();
        string sMQName = jsonAttach.GetStr("MQName", "");
        unsigned iMsgSize = static_cast<unsigned>(jsonAttach.GetVal("MsgSize", 300));
        string sJsonFeild = jsonAttach.GetStr("JsonFeild", "");
        // 发送消息
        if (!sMQName.empty())
        {
            // 获取发送内容
            const IKCJson& jsonContent = objCtrlD.GetParmJson(sJsonFeild.c_str());
            // 发送消息
            MQCallbackHelper<CCtrlWebMQCallback>::SendData(sMQName, jsonContent.ToStr(), iMsgSize, m_dir + "/temp/mq");
        }
    }
    catch (boost::interprocess::interprocess_exception &ex)
    {
        std::cout << typeid(ex).name() << ": " << ex.what() << std::endl;
        act.WriteLogError(ex.what(), __FUNCTION__, typeid(ex).name());
    }
    catch (std::exception& ex)
    {
        std::cout << typeid(ex).name() << ": " << ex.what() << std::endl;
        act.WriteLogError(ex.what(), __FUNCTION__, typeid(ex).name());
    }
    catch (...)
    {
        std::cout << "Unknown Error" << std::endl;
        act.WriteLogError("Unknown Error", __FUNCTION__, KC::CBacktraceSymbols::Get().c_str());
    }
}
