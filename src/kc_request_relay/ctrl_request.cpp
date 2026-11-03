#include "ctrl_request.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlRelay类
CCtrlRelay::CCtrlRelay(IKCRelayREST& own, string sName, boost::property_tree::ptree& pt)
    : m_own(own), m_name(sName)
{
    // 初始化
    try
    {
        string sErr = m_requestLib.Load(m_own, m_own.getContext().getPath() + string("/so_web_request"));
        if (!sErr.empty()) throw std::runtime_error(sErr);
    }
    catch (std::exception &ex)
    {
        string sExcept = (boost::format("Load `so_web_request` Failed \n<%s> %s") % typeid(ex).name() % ex.what()).str();
        cout << "*[knewcode] " << sExcept << endl;
        m_own.WriteLogError(sExcept.c_str(), __CURR_CODE_PLACE_C__);
        throw;
    }
    catch (...)
    {
        string sExcept = "*[knewcode] Load `so_web_request` Failed.";
        cout << sExcept << endl;
        m_own.WriteLogError(sExcept.c_str(), __CURR_CODE_PLACE_C__);
        throw;
    }
}

CCtrlRelay::~CCtrlRelay()
{
    try
    {
        m_requestLib.UnLoad();
    }
    catch (...) {}
}

// 执行控制器
void CCtrlRelay::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sApiUrl;
    auto fExceptInfo = [&](void)
    {
        return (boost::format("\n (Relay-> [act=%s]) ") % pAct).str();
    };
    try
    {
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "post"));
        // 解析控制器
        map<string, string> mpCfgs;
        CCltrFunc::ParseCtrl(mpCfgs, CUtilFunc::PCharSafeToStr(pContext));
        // 调用前置的控制器
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
        if (0 != iErrCode)
            throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$Before] " + objCtrlD.JsonRespond().GetErrMsg(), m_own);
        // GET请求（method="get"）
        if ("get" == sMethod)
            sApiUrl = m_requestLib.Inf().PostGet(objCtrlD, mpCfgs);
        // POST请求（method="post"）
        else
            sApiUrl = m_requestLib.Inf().PostGet(objCtrlD, mpCfgs, true);
        string sErrMsg = objCtrlD.JsonRespond().GetErrMsg();
        // 如果失败，则退出
        if (0 != (iErrCode = objCtrlD.JsonRespond().GetErrCode()))
            //throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$request - " + sApiUrl + "]" + sErrMsg + "\n", m_own);
            throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$request]" + sErrMsg + "\n", m_own);
        // 调用后置的控制器
        if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true)))
            throw TKCRelayRESTException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + fExceptInfo() + "[$after]" + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    }
    catch (TKCRelayRESTException &ex)
    {
        string sErr = m_own.getHint("Request_Failed_") + sApiUrl + "\n" + ex.error_info();
        m_own.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, (act.GetSingleInfo("the_request") + string("\n") + pLocalFile).c_str());
        //throw;
        objCtrlD.SetJsonRespond(ex.error_id(), sErr.c_str());
    }
    catch (std::exception &ex)
    {
        string sErr = m_own.getHint("Request_Failed_") + sApiUrl + "\n" + ex.what();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, (act.GetSingleInfo("the_request") + fExceptInfo() + string("\n") + pLocalFile).c_str());
        //throw TKCRelayRESTException(ecd_ErrCode_KCRequestRelay + 3, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + ex.what() + fExceptInfo(), m_own);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCRequestRelay + 3, ex.what());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Request_Failed_") + sApiUrl + "\n" + m_own.getHint("Unknown_exception");
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, (act.GetSingleInfo("the_request") + string("\n") + pLocalFile).c_str());
        //throw TKCRelayRESTException(ecd_ErrCode_KCRequestRelay + 4, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Unknown_exception") + fExceptInfo(), m_own);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCRequestRelay + 4, m_own.getHint("Unknown_exception"));
    }
}
