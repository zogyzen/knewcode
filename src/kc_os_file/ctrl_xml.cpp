#include "ctrl_xml.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlXML类
CCtrlXML::CCtrlXML(IKOSFile& own, string sName, property_tree::ptree& pt) : m_own(own), m_name(sName)
{
    if (pt.get_child_optional("<xmlattr>.type"))
        m_type = pt.get<string>("<xmlattr>.type");
}

CCtrlXML::~CCtrlXML()
{
}

// 执行控制器
void CCtrlXML::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    try
    {
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "list"));
        // 字符集
        string sAttachCharset = objCtrlD.JsonAttach().GetStr(c_RESTful_Charset, c_RESTful_UTF8);
        algorithm::to_upper(sAttachCharset);
        // 解析控制器
        map<string, string> mpCfgs;
        CCltrFunc::ParseCtrl(mpCfgs, pContext);
        m_own.WriteLogInfo(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (m_type + "-" + pMethod).c_str());
        // 调用前置的控制器
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
        if (0 != iErrCode)
            throw TKCOSFileException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$before]") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
        string sMsg;
        // 读取节点信息（method="get"）
        if ("get" == sMethod) sMsg = GetNode(objCtrlD, mpCfgs);
        // 写入节点信息（method="set"）
        else if ("set" == sMethod) sMsg = SetNode(objCtrlD, mpCfgs);
        // 删除节点（method="del"）
        else if ("del" == sMethod) sMsg = DelNode(objCtrlD, mpCfgs);
        // 清除节点（method="clear"）
        else if ("clear" == sMethod) sMsg = ClearNode(objCtrlD, mpCfgs);
        // 读取子节点列表（method="list"）
        else sMsg = ListSub(objCtrlD, mpCfgs);
        // 调用后置控制器
        if (0 != (iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true)))
            throw TKCOSFileException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$after]") + objCtrlD.JsonRespond().GetErrMsg() + "\n" + sMsg, m_own);
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

// 读取子节点列表
string CCtrlXML::ListSub(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    IActionData& act = objCtrlD.ActionData();
    // 参数
    string sFile = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
    string sNode = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$node"), objCtrlD);
    string sAttrs = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$attrs"), objCtrlD);
    // 读配置文件
    boost::property_tree::ptree pt;
    if (!boost::filesystem::exists(sFile))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_file_") + sFile, m_own);
    read_xml(sFile, pt);
    // 属性列表
    vector<string> vctAttr;
    boost::algorithm::split(vctAttr, sAttrs, boost::is_any_of(","));
    // 设置输出
    IKCJson& jsonNodes = objCtrlD.JsonRespond().AddItem("nodes", false);
    // 子节点列表
    BOOST_FOREACH(auto &pv, pt.get_child(sNode))
    {
        if ("<xmlcomment>" != pv.first)
        {
            IKCJson& jsonNode = jsonNodes.AddItem(pv.first.c_str(), false);
            for (auto &attr : vctAttr)
            {
                string strNodeAttr = "<xmlattr>." + attr;
                if (pv.second.get_child_optional(strNodeAttr))
                {
                    string sVal = pv.second.get<string>(strNodeAttr);
                    jsonNode.AddStr(attr.c_str(), sVal.c_str());
                }
            }
        }
    }
    return "ListSub - " + sFile + " / " + sNode;
}

// 读取节点信息
string CCtrlXML::GetNode(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    IActionData& act = objCtrlD.ActionData();
    // 参数
    string sFile = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
    string sNode = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$node"), objCtrlD);
    string sAttrs = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$attrs"), objCtrlD);
    // 读配置文件
    boost::property_tree::ptree pt;
    if (!boost::filesystem::exists(sFile))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_file_") + sFile, m_own);
    read_xml(sFile, pt);
    // 设置输出
    IKCJson& jsonNode = objCtrlD.JsonRespond().AddItem("node", false);
    // 节点属性列表
    vector<string> vctAttr;
    boost::algorithm::split(vctAttr, sAttrs, boost::is_any_of(","));
    for (auto &attr : vctAttr)
    {
        string strNodeAttr = sNode + ".<xmlattr>." + attr;
        if (pt.get_child_optional(strNodeAttr))
        {
            string sVal = pt.get<string>(strNodeAttr);
            jsonNode.AddStr(attr.c_str(), sVal.c_str());
        }
    }
    return "GetNode - " + sFile + " / " + sNode + " / " + sAttrs;
}

// 删除节点
string CCtrlXML::DelNode(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    IActionData& act = objCtrlD.ActionData();
    // 参数
    string sFile = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
    string sNode = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$node"), objCtrlD);
    string sSub = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$sub"), objCtrlD);
    // 读配置文件
    boost::property_tree::ptree pt;
    if (!boost::filesystem::exists(sFile))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_file_") + sFile, m_own);
    read_xml(sFile, pt, boost::property_tree::xml_parser::trim_whitespace, std::locale());
    // 节点
    if (!pt.get_child_optional(sNode))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 6, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_field_") + sNode, m_own);
    auto &ptNode = pt.get_child(sNode);
    ptNode.erase(sSub);
    // 写配置文件
    boost::property_tree::xml_parser::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
    write_xml(sFile, pt, std::locale(), settings);
    return "DelNode - " + sFile + " / " + sNode;
}

// 清除节点
string CCtrlXML::ClearNode(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    IActionData& act = objCtrlD.ActionData();
    // 参数
    string sFile = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
    string sNode = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$node"), objCtrlD);
    // 读配置文件
    boost::property_tree::ptree pt;
    if (!boost::filesystem::exists(sFile))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_file_") + sFile, m_own);
    read_xml(sFile, pt, boost::property_tree::xml_parser::trim_whitespace, std::locale());
    // 节点
    if (!pt.get_child_optional(sNode))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 6, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_field_") + sNode, m_own);
    auto &ptNode = pt.get_child(sNode);
    ptNode.clear();
    // 写配置文件
    boost::property_tree::xml_parser::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
    write_xml(sFile, pt, std::locale(), settings);
    return "ClearNode - " + sFile + " / " + sNode;
}

// 写入节点信息
string CCtrlXML::SetNode(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    IActionData& act = objCtrlD.ActionData();
    // 参数
    string sFile = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$file"), objCtrlD);
    string sNode = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$node"), objCtrlD);
    string sSub = CCltrFunc::TranCharset(CCltrFunc::GetParm(objCtrlD, mpCfgs, "$sub"), objCtrlD);
    // 读配置文件
    boost::property_tree::ptree pt;
    if (!boost::filesystem::exists(sFile))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 5, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_file_") + sFile, m_own);
    read_xml(sFile, pt, boost::property_tree::xml_parser::trim_whitespace, std::locale());
    // 节点
    if (!pt.get_child_optional(sNode))
        throw TKCOSFileException(ecd_ErrCode_KCOSFile + 6, __CURR_CODE_PLACE_C__, string("[") + act.GetSingleInfo("the_request") + "] " + m_own.getHint("Don_t_exists_field_") + sNode, m_own);
    auto &pNode = pt.get_child(sNode);
    IKCJson& jsonSubs = act.MakeJson(sSub.c_str());
    // 遍历设置子节点
    SetNode(pNode, jsonSubs);
    // 写配置文件
    boost::property_tree::xml_parser::xml_writer_settings<boost::property_tree::ptree::key_type> settings('\t', 1);
    write_xml(sFile, pt, std::locale(), settings);
    return "SetNode - " + sFile + " / " + sNode;
}
void CCtrlXML::SetNode(boost::property_tree::ptree &pt, const IKCJson& json)
{
    for (int i = 0, c = json.GetItemCount(); i < c; ++i)
    {
        const IKCJson& jsonSub = json.GetItem(i);
        string sName = jsonSub.GetName();
        int iCnt = jsonSub.GetItemCount();
        if (iCnt > 0)
        {
            auto &ptSub = pt.put(sName, "");
            SetNode(ptSub, jsonSub);
        }
        else
            pt.put("<xmlattr>." + sName, jsonSub.GetStr());
    }
}
