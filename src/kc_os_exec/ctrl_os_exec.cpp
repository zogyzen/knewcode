#include "ctrl_os_exec.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlOSExec类
CCtrlOSExec::CCtrlOSExec(IKCOSExec& own, string sName, property_tree::ptree& /*pt*/)
    : m_own(own), m_name(sName)
{
}

CCtrlOSExec::~CCtrlOSExec(void)
{
}

// 执行控制器
void CCtrlOSExec::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    auto fExceptInfo = [&](void)
    {
        string sAct(nullptr != pAct ? pAct : "");
        return "\n" + sAct + " - \t" + objCtrlD.GetPostStr();
    };
    try
    {
        // 解析控制器
        map<string, string> mpCfgs;
        CCltrFunc::ParseCtrl(mpCfgs, pContext);
        // 调用前置的控制器
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
        if (0 != iErrCode)
            throw TKCOSExecException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$before]") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
        // 返回值变量
        iErrCode = 0;
        string sErrMsg;
        // 命令
        string sCmd = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$cmd");
        // 方式
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "console"));
        // 执行信息
        auto fExecInfo = [&](string sHead)
        {
            return sHead + sCmd + "\n" + sErrMsg + "\n" + act.GetSingleInfo("client_host_port") + "\n" + act.GetSingleInfo("Cookie");
        };
        // 执行进程（method="process"）
        if ("process" == sMethod)
        {
            iErrCode = CUtilFunc::GetCmdResult(sCmd, sErrMsg);
            m_own.WriteLogInfo(fExecInfo("[Exec Process] > ").c_str(), __CURR_CODE_PLACE_C__);
        }
        // 进程间（命名管道）通讯（method="pipe"）
        else if ("pipe" == sMethod)
        {
            // 创建一个子进程，并捕获其标准输出
            boost::process::ipstream pipe_stream;
            boost::process::child c(sCmd, boost::process::std_out > pipe_stream);
            // 读取并显示输出
            std::string line;
            while (std::getline(pipe_stream, line))
            {
                sErrMsg += line;
            }
            // 等待子进程结束
            c.wait();
        }
        // 进程间（网络）通讯（method="socket"）
        else if ("socket" == sMethod)
        {
        }
        // 脚本文件（method="cmdFile"）
        else if ("cmdfile" == sMethod)
        {
            string sFile = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file");
            // 当前kc文件路径
            std::string sCurrDir = boost::filesystem::path(pLocalFile).parent_path().string();
            // 脚本文件绝对路径
            std::string sLocalFile = CUtilFunc::ToAbsPath(sFile, sCurrDir);
            if (!boost::filesystem::exists(sLocalFile))
                throw std::runtime_error("cmdFile Error. Not Exists - " + sFile);
            // 执行脚本
            string sParm = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$parm");
            sCmd = "\"" + sLocalFile + "\" " + sParm;
            sErrMsg = CUtilFunc::GetCmdResult(sCmd);
            m_own.WriteLogInfo(fExecInfo("[Exec CmdFile] > ").c_str(), __CURR_CODE_PLACE_C__);
        }
        // 执行脚本（method="console"）
        else
        {
            sErrMsg = CUtilFunc::GetCmdResult(sCmd);
            m_own.WriteLogInfo(fExecInfo("[Exec Cmd] > ").c_str(), __CURR_CODE_PLACE_C__);
        }
        // 设置错误返回信息
        objCtrlD.JsonRespond().SetVal(c_RESTful_errCode, iErrCode);
        objCtrlD.JsonRespond().SetStr(c_RESTful_errMsg, sErrMsg.c_str());
        // 设置获取到的信息
        CCltrFunc::SetParm(objCtrlD, mpCfgs, "$result", sErrMsg);
        // 如果失败，则退出
        if (0 != iErrCode)
            throw TKCOSExecException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$cmd]") + sErrMsg + "\n", m_own);
        // 调用后置的控制器
        if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true)))
            throw TKCOSExecException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$after]") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    }
    catch (interprocess::interprocess_exception &ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ex.get_error_code(), ex.what());
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.WriteLog(ex);
        objCtrlD.SetJsonRespond(ex.error_id(), ex.what());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCOSExec + 3, ex.what());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCOSExec + 4, (string("Session ") + m_own.getHint("Unknown_exception")).c_str());
    }
}
