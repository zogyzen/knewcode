#include "ctrl_updown.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlUpDown类
CCtrlUpDown::CCtrlUpDown(IKOSFile& own, string sName, property_tree::ptree& pt) : m_own(own), m_name(sName)
{
    if (pt.get_child_optional("<xmlattr>.type"))
        m_type = pt.get<string>("<xmlattr>.type");
}

CCtrlUpDown::~CCtrlUpDown()
{
}

// 执行控制器
void CCtrlUpDown::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    try
    {
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "upfile"));
        // 解析控制器
        map<string, string> mpCfgs;
        CCltrFunc::ParseCtrl(mpCfgs, pContext);
        m_own.WriteLogInfo(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (m_type + "-" + pMethod).c_str());
        // 下载文件（method="downfile"）
        if ("downfile" == sMethod) downfile(objCtrlD, sMethod, mpCfgs);
        // 上传文件（method="upfile"）
        else upfile(objCtrlD, sMethod, mpCfgs);
    }
    catch (TKCOSFileException &ex)
    {
        m_own.WriteLogError(ex.error_info().c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        throw;
    }
    catch (std::exception &ex)
    {
        m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 3, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + ex.what(), m_own);
    }
    catch (...)
    {
        m_own.WriteLogFatal(m_own.getHint("Unknown_exception"), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 4, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Unknown_exception"), m_own);
    }
}

// 上传文件
void CCtrlUpDown::upfile(ICtrlApiData& objCtrlD, string sMethod, map<string, string>& mpCfgs)
{
    ICtrlApiDataX *pObjCtrlDX = dynamic_cast<ICtrlApiDataX*>(&objCtrlD);
    if (nullptr == pObjCtrlDX)
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, m_own.getHint("Null_Point_Data_"), m_own);
    IKCRequestRespond& re = pObjCtrlDX->GetRequestRespond();
    IActionData& act = objCtrlD.ActionData();
    string sLocalDir, sFilename;
    bool bHasFile = false;
    int iFrmCount = re.GetMultiFormDataCount();
    // 按文件数量循环
    for (int i = 0;  i < iFrmCount; ++i)
        // 上传文件
        if ("upfile" == sMethod)
        try
        {
            // 设置当前文件位置
            re.SetCurrentMultiFormData(i);
            // 获取文件流数据
            IMultiFormData& mfd = re.GetMultiFormData();
            if (mfd.isFile())
            {
                bHasFile = true;
                // 调用前置的控制器
                int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
                if (0 != iErrCode)
                    throw TKCOSFileException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$before]") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
                // 保存的目录
                string sUrlDir = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$path"), objCtrlD);
                // 本地目录
                sLocalDir = re.GetLocalPath(sUrlDir.c_str());
                if (!boost::filesystem::exists(sLocalDir)) boost::filesystem::create_directories(sLocalDir);
                if (!boost::filesystem::exists(sLocalDir))
                    throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, m_own.getHint("Create_Fail_") + sLocalDir, m_own);
                // 保存的文件
                sFilename = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
                // 本地完整文件名
                string sLocalFile = sLocalDir + "/" + sFilename;
                m_own.WriteLogInfo("Save File", __CURR_CODE_PLACE_C__, sLocalFile.c_str());
                // 文件二进制流
                unsigned iSize = 0;
                const char* pBuf = mfd.GetBody(iSize);
                string sBuf(pBuf, iSize);
                // 保存文件
                CUtilFunc::SaveFile(sLocalFile, sBuf);
                if (!boost::filesystem::exists(sLocalFile))
                    throw TKCOSFileException(ecd_ErrCode_KCOSFile + 6, __CURR_CODE_PLACE_C__, m_own.getHint("Create_Fail_") + sLocalFile, m_own);
                // 调用后置的控制器
                if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true)))
                    throw TKCOSFileException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$after][") + sLocalDir + "][" + sFilename + "] " + objCtrlD.JsonRespond().GetErrMsg(), m_own);
            }
        }
        catch (TKCOSFileException&)
        {
            throw;
        }
        catch (std::exception &ex)
        {
            throw TKCOSFileException(ecd_ErrCode_KCOSFile + 7, __CURR_CODE_PLACE_C__, string() + ex.what() + "\r\n[" + typeid(ex).name() + "]" + "[" + sLocalDir + "][" + sFilename + "]", m_own);
        }
        catch (...)
        {
            throw TKCOSFileException(ecd_ErrCode_KCOSFile + 8, __CURR_CODE_PLACE_C__, string() + m_own.getHint("Unknown_exception") + "\r\n[" + sLocalDir + "][" + sFilename + "]", m_own);
        }
    if (!bHasFile)
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 9, __CURR_CODE_PLACE_C__, (format("Not Exists File [FormData Count = %d]") % iFrmCount).str().c_str(), m_own);
}

// 下载文件
void CCtrlUpDown::downfile(ICtrlApiData& objCtrlD, string, map<string, string>& mpCfgs)
{
    auto &act = objCtrlD.ActionData();
    // 获取文件内容
    auto fGetContent = [&]()
    {
        string sContent;
        // url
        string sUrl = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$url");
        // 读取url文件内容
        if (!sUrl.empty())
        {
            string sLocal = act.GetUrlLocalPath(sUrl.c_str());
            if (!boost::filesystem::exists(sLocal) || !boost::filesystem::is_regular_file(sLocal))
            {
                m_own.WriteLogError((boost::format("%s %s \t\t Local File As Follows:\n%s\n") % m_own.getHint("Don_t_exists_file_") % sUrl % sLocal).str().c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
                throw TKCOSFileException(ecd_ErrCode_KCOSFile + 10, __CURR_CODE_PLACE_C__, (m_own.getHint("Don_t_exists_file_") + sUrl).c_str(), m_own);
            }
            sContent = CUtilFunc::ReadFile(sLocal);
        }
        // 文件内容
        else
        {
            sContent = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$content");
            string sCharset = CUtilFunc::PCharSafeToStr(objCtrlD.GetCharset());
            if (c_RESTful_GBK == sCharset) sContent = CUtilFunc::Utf8ToGbk(sContent);
        }
        return sContent;
    };
    // 调用前置的控制器
    int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
    if (0 != iErrCode)
        throw TKCOSFileException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$before]") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    // 文件名
    string sFileName = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$filename");
    // 流类型
    string sType = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$type");
    if (sType.empty()) sType = c_OctetStreamResponseContentType;
    // 在头中返回json
    string sResultJson = (boost::format(R"({"filename":"%s"})") % sFileName).str();
    // 文件内容
    string sContent = fGetContent();
    if (sContent.empty())
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 11, __CURR_CODE_PLACE_C__, ("Empty File: " + sFileName).c_str(), m_own);
    // 输出
    auto &parmRespond = objCtrlD.JsonRespond().ParmInOut();
    parmRespond.SetHeader("Content-Disposition", ("attachment; filename=" + sFileName).c_str());
    parmRespond.SetContent(sContent.c_str(), static_cast<unsigned>(sContent.size()));
    // parmRespond.SetContentType(c_DownTxtFileResponseContentType);
    parmRespond.SetContentType(sType.c_str());
    parmRespond.SetHeader(c_RespondHeaderResultJson, sResultJson.c_str());
}
