#include "ctrl_prjs.h"

CCtrlPrjs *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* /*dir*/, const char* name)
    {
        if (nullptr == g_ctrl) g_ctrl = new CCtrlPrjs(name);
        return *g_ctrl;
    }
    void CALL_TYPE UninitActor(IKCController&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}


////////////////////////////////////////////////////////////////////////////////
// CCtrlPrjs类
CCtrlPrjs::CCtrlPrjs(string sName) : m_name(sName)
{
}

CCtrlPrjs::~CCtrlPrjs()
{
}

// 执行控制器
void CCtrlPrjs::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    // const char* pLocalFile = objCtrlD.LocalFile();
    // const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    try
    {
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "CreatePrj"));
        // 字符集
        string sAttachCharset = objCtrlD.JsonAttach().GetStr(c_RESTful_Charset, c_RESTful_UTF8);
        algorithm::to_upper(sAttachCharset);
        // 解析控制器
        map<string, string> mpCfgs;
        CCltrFunc::ParseCtrl(mpCfgs, pContext);
        act.WriteLogInfo(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (m_name + "-" + pMethod).c_str());
        // 调用前置的控制器
        int iErrCode = 0;
        if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true)))
            throw TException(iErrCode, __CURR_CODE_PLACE_C__, string("[$before]") + objCtrlD.JsonRespond().GetErrMsg(), act.GetSingleInfo("the_request"));
        // 创建项目（method="CreatePrj"）
        if ("createprj" == sMethod) CreatePrj(objCtrlD, mpCfgs);
        // 删除项目（method="RemovePrj"）
        else if ("removeprj" == sMethod) CreatePrj(objCtrlD, mpCfgs);
        // 调用后置控制器
        if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true)))
            throw TException(iErrCode, __CURR_CODE_PLACE_C__, string("[$after]") + objCtrlD.JsonRespond().GetErrMsg(), act.GetSingleInfo("the_request"));
    }
    catch (std::exception& e)
    {
        act.WriteLogFatal((string("[") + typeid(e).name() + "] " + e.what()).c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        throw;
    }
    catch (...)
    {
        act.WriteLogFatal(act.GetHint("Unknown_exception"), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        throw;
    }
}
