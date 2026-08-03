#include "ctrl_disk.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlDisk类
CCtrlDisk::CCtrlDisk(IKOSFile& own, string sName, property_tree::ptree& pt) : m_own(own), m_name(sName)
{
    if (pt.get_child_optional("<xmlattr>.type"))
        m_type = pt.get<string>("<xmlattr>.type");
}

CCtrlDisk::~CCtrlDisk()
{
}

// 执行控制器
void CCtrlDisk::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    try
    {
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "TraversalDir"));
        // 字符集
        string sAttachCharset = objCtrlD.JsonAttach().GetStr(c_RESTful_Charset, c_RESTful_UTF8);
        algorithm::to_upper(sAttachCharset);
        // 解析控制器
        map<string, string> mpCfgs;
        CCltrFunc::ParseCtrl(mpCfgs, pContext);
        m_own.WriteLogInfo(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (m_type + "-" + pMethod).c_str());
        // 创建目录（method="CreateDir"）
        if ("createdir" == sMethod) CreateDir(objCtrlD, mpCfgs);
        // 删除目录（文件）（method="Remove"）
        else if ("remove" == sMethod) Remove(objCtrlD, mpCfgs);
        // 写入文本文件（method="WriteTxt"）
        else if ("writetxt" == sMethod) WriteTxt(objCtrlD, mpCfgs);
        // 读出文本文件（method="ReadTxt"）
        else if ("readtxt" == sMethod) ReadTxt(objCtrlD, mpCfgs);
        // 拷贝（method="Copy"）
        else if ("copy" == sMethod) Copy(objCtrlD, mpCfgs);
        // 遍历目录（method="TraversalDir"）
        else TraversalDir(objCtrlD, mpCfgs);
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

// 处理
void CCtrlDisk::Deal(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs, std::function<string()> func)
{
    IActionData& act = objCtrlD.ActionData();
    // 调用前置的控制器
    int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
    if (0 != iErrCode)
        throw TKCOSFileException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$before]") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    // 执行
    string sMsg = func();
    // 调用后置控制器
    if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true)))
        throw TKCOSFileException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$after]") + objCtrlD.JsonRespond().GetErrMsg() + "\n" + sMsg, m_own);
}
void CCtrlDisk::DealPath(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs, string sType, std::function<bool(string, system::error_code&)> func)
{
    Deal(objCtrlD, mpCfgs, [&]() -> string {
        // 路径处理
        string sPath = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$path"), objCtrlD);
        system::error_code ec;
        bool bRes = func(sPath, ec);
        // 处理失败
        if (!bRes)
        {
            int iCode = ec.value();
            if (0 != iCode)
                throw TKCOSFileException(iCode, __CURR_CODE_PLACE_C__, CCltrFunc::TranCharset(ec.message() + " - " + sPath, objCtrlD, false), m_own);
            else
                throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, sType + sPath, m_own);
        }
        return sType + sPath;
    });
}

// 创建目录
void CCtrlDisk::CreateDir(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    DealPath(objCtrlD, mpCfgs,  m_own.getHint("Create_Fail_"), [&](string sDir, system::error_code& ec){
        return boost::filesystem::create_directories(sDir, ec);
    });
}

// 删除目录（文件）
void CCtrlDisk::Remove(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    DealPath(objCtrlD, mpCfgs,  m_own.getHint("Remove_Fail_"), [&](string sPath, system::error_code& ec){
        return boost::filesystem::remove_all(sPath, ec);
    });
}

// 写入文本文件
void CCtrlDisk::WriteTxt(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    Deal(objCtrlD, mpCfgs, [&]() -> string{
        // 文件
        string sFile = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
        // 内容
        string sContent = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$content"), objCtrlD);
        // 保存文件
        CUtilFunc::SaveFile(sFile, sContent);
        return "Write - " + sFile;
    });
}

// 读出文本文件
void CCtrlDisk::ReadTxt(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    Deal(objCtrlD, mpCfgs, [&]() -> string{
        // 文件
        string sFile = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
        // 读取文件
        string sContent = CUtilFunc::ReadFile(sFile);
        // 设置输出
        objCtrlD.JsonRespond().SetStr("content", sContent.c_str());
        return "Read - " + sFile;
    });
}

// 拷贝文件（目录）
void CCtrlDisk::Copy(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    DealPath(objCtrlD, mpCfgs,  m_own.getHint("Request_Failed_"), [&](string sPath, system::error_code& ec){
        string sTarget = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$target"), objCtrlD);
        boost::filesystem::copy(sPath, sTarget, boost::filesystem::copy_options::recursive | boost::filesystem::copy_options::overwrite_existing, ec);
        return !ec;
    });
}

// 遍历目录
void CCtrlDisk::TraversalDir(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    DealPath(objCtrlD, mpCfgs,  m_own.getHint("Request_Failed_"), [&](string sDir, system::error_code& /*ec*/){
        if (sDir.empty())
            throw TKCOSFileException(ecd_ErrCode_KCOSFile + 6, __CURR_CODE_PLACE_C__, m_own.getHint("Don_t_exists_field_") + string("[$path]"), m_own);
        // 过滤扩展名
        string sExtName = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$ext"), objCtrlD);
        if (sExtName.empty()) sExtName = "*";
        // 设置输出
        IKCJson& jsn = objCtrlD.JsonRespond().AddItem("dir", false);
        // 遍历目录
        TraversalDir(objCtrlD, jsn, sDir, sExtName);
        return true;
    });
}
void CCtrlDisk::TraversalDir(ICtrlApiData& objCtrlD, IKCJson& jsn, boost::filesystem::path dir, string extName, int layer, string layerDir)
{
    for (const auto& iter : boost::filesystem::directory_iterator(dir))
    {
        bool isDir = boost::filesystem::is_directory(iter.path());
        string sExt = CCltrFunc::TranCharset(iter.path().filename().extension().string(), objCtrlD, false);
        // 如果是文件，需根据扩展名筛选
        if (isDir || "*" == extName || sExt == extName)
        {
            // 文件名
            string sPthName = CCltrFunc::TranCharset(iter.path().filename().string(), objCtrlD, false);
            // 从主目录开始的目录层级名称
            string sLayName = layerDir + sPthName;
            // 添加json节点
            IKCJson& jsnSub = jsn.AddItem(sPthName.c_str(), false);
            jsnSub.SetVal("type", isDir ? 1 : 0);           // 类型：0文件；1目录
            jsnSub.SetVal("layer", layer);                  // 目录层级：0为主目录
            jsnSub.SetStr("pathLayer", sLayName.c_str());   // 从主目录开始的目录层级名称
            jsnSub.SetStr("pathFull", CCltrFunc::TranCharset(iter.path().string(), objCtrlD, false).c_str());     // 完全的目录名称
            // 如果是目录，则递归遍历
            if (isDir)
            {
                IKCJson& jsnSubs = jsnSub.AddItem("subs", false);
                TraversalDir(objCtrlD, jsnSubs, iter.path(), extName, layer + 1, sLayName + "/");
            }
        }
    }
}
