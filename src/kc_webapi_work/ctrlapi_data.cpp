#include "ctrlapi_data.h"
#include "action_data.h"

////////////////////////////////////////////////////////////////////////////////
// TBatchGrpInDbSetTmpJson 类
CCtrApilDataBase::TBatchGrpInDbSetTmpJson::TBatchGrpInDbSetTmpJson(CActionData& act) : m_json(new CKCJsonPackRespond(act, "{}"))
{
}

////////////////////////////////////////////////////////////////////////////////
// CRootCtrlApiData 类
CRootCtrlApiData::CRootCtrlApiData(CActionData& act)
    : CCtrApilData(act)
    , m_needToken(m_isRootRun && !m_act.webapi().NeedToken().empty() && GetXmlattrValFul(c_RESTful_needToken, true))
{
    const_cast<bool&>(act.m_needToken) = m_needToken;
    const_cast<bool&>(act.m_jsonCaseSensitive) = m_jsonCaseSensitive;
    const_cast<string&>(act.m_charset) = m_charset;
    cout << "*[Knewcode] Ctrl Charset: " << m_charset << endl;
}

// 创建
std::shared_ptr<CRootCtrlApiData> CRootCtrlApiData::CreatePtr(CActionData& act)
{
    std::shared_ptr<CRootCtrlApiData> objCtrlDPtr(new CRootCtrlApiData(act));
    objCtrlDPtr->m_selfRef = objCtrlDPtr;
    return objCtrlDPtr;
}

////////////////////////////////////////////////////////////////////////////////
// CSubCtrlApiData 类
CSubCtrlApiData::CSubCtrlApiData(CCtrApilDataWeakPtr own, string sSubUrl, string sGetArg, string sKCFilePath, string sSign)
    : CCtrApilData(own, sSubUrl, sGetArg, sKCFilePath, sSign)
    , m_own(own)
{
}
CSubCtrlApiData::CSubCtrlApiData(CCtrApilDataWeakPtr own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut)
    : CCtrApilData(own, btype, sSrv, sSo, sCtrlName, sMethod, sContent, sGoto, sOut)
    , m_own(own)
{
}

// 创建
CCtrApilDataPtr CSubCtrlApiData::CreatePtr(CActionData& act, CCtrApilDataWeakPtr own, string sSubNode, string sSubUrl, string sGetArg, string sKCFilePath)
{
    if (own.expired())
        throw TCtrlException(ecd_ErrCode_KCWebApiWork + 20, __CURR_CODE_PLACE_C__, string(act.GetHint("Null_Point_Data_")) + "Owner. \t\t" + sGetArg + "" + sKCFilePath + "\n" + act.GetSingleInfo("the_request"), typeid(own).name());
    string sSign = string() + own.lock()->Url() + "<" + sSubNode + ">:" + sSubUrl + "?" + (sGetArg.empty() ? "act=main" : sGetArg);
    CCtrApilDataPtr objCtrlDPtr(new CSubCtrlApiData(own, sSubUrl, sGetArg, sKCFilePath, sSign));
    objCtrlDPtr->m_selfRef = objCtrlDPtr;
    return objCtrlDPtr;
}
CCtrApilDataPtr CSubCtrlApiData::CreatePtr(CActionData& act, CCtrApilDataWeakPtr own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut)
{
    if (own.expired())
        throw TCtrlException(ecd_ErrCode_KCWebApiWork + 20, __CURR_CODE_PLACE_C__, string(act.GetHint("Null_Point_Data_")) + "Owner. \t\t" + btype + " / " + sSrv + " " + sSo + " / " + sCtrlName + " / " + sMethod + " / " + sGoto + " / " + sOut + "\n" + act.GetSingleInfo("the_request"), typeid(own).name());
    CCtrApilDataPtr objCtrlDPtr(new CSubCtrlApiData(own, btype, sSrv, sSo, sCtrlName, sMethod, sContent, sGoto, sOut));
    objCtrlDPtr->m_selfRef = objCtrlDPtr;
    return objCtrlDPtr;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrApilData 类
// 根控制器构造
CCtrApilData::CCtrApilData(CActionData& act)
    : CCtrApilDataBase(act, act.re().GetSingleInfo("uri"), act.re().GetGetArgStr(), act.re().GetLocalFilename(), act.re().GetSingleInfo("unparsed_uri"), !act.re().getRe().IsSubCall())
{
}
// 子控制器构造
CCtrApilData::CCtrApilData(CCtrApilDataWeakPtr own, string sSubUrl, string sGetArg, string sKCFilePath, string sSign)
    : CCtrApilDataBase(own.lock()->Act(), sSubUrl, sGetArg, sKCFilePath, sSign, false, own.lock()->m_batchGrpInTmpPrm, own.lock()->m_batchGrpInDbSetTmpJson)
    , m_jsonRespondLast(own.lock()->m_jsonRespond)      // 上一个子控制器的执行结果，作为本子控制器的参数
    , m_innerDbSetLast(&own.lock()->m_innerDbSet)       // 上一个控制器保存的内部数据集
{
}
// 临时调用时的构造函数
CCtrApilData::CCtrApilData(CCtrApilDataWeakPtr own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut)
    : CCtrApilDataBase(own, btype, sSrv, sSo, sCtrlName, sMethod, sContent, sGoto, sOut)
    , m_jsonRespondLast(own.lock()->m_jsonRespond)      // 上一个子控制器的执行结果，作为本子控制器的参数
    , m_innerDbSetLast(&own.lock()->m_innerDbSet)       // 上一个控制器保存的内部数据集
{
}
// 移入移出结果
CKCJsonPackRespond::TCoreParmJsonRespondPtr CCtrApilData::MoveOutResUp(void)        // 向父控制器移出。
{
    return m_jsonRespond.MoveOut();
}
bool CCtrApilData::MoveInRes(CKCJsonPackRespond::TCoreParmJsonRespondPtr resCore)   // 从子控制器移入。（上一个控制器的结果，从父控制器移入，在构造函数里实现）
{
    return m_jsonRespond.MoveIn(resCore);
}

// 执行控制器
int CCtrApilData::WorkAct(const char* sUrl, const char* sName)
{
    const auto resCore = m_act.webapi().SubCallCtrlApi(*this, CUtilFunc::PCharSafeToStr(sUrl), "", CUtilFunc::PCharSafeToStr(sName));
    return resCore.get() != nullptr ? resCore->GetErrCode() : -999;
}
// 临时控制器信息的回调接口
void CCtrApilData::ExecBundle(const char* sSrv, const char* sCtrlName, const char* sMethod, const char* sContent, const char* sGoto)
{
    struct TCtrlDataBackCallSrv : CSubCtrlApiData
    {
        TCtrlDataBackCallSrv(CCtrApilDataWeakPtr own, string sSrv, string sCtrlName, string sMethod, string sContent, string sGoto)
            : CSubCtrlApiData(own, c_RESTful_bundle, sSrv, "", sCtrlName, sMethod, sContent, sGoto, "") {}
    };
    CCtrApilDataPtr objCtrlDPtr(new TCtrlDataBackCallSrv(m_selfRef, sSrv, sCtrlName, sMethod, sContent, sGoto));
    objCtrlDPtr->m_selfRef = objCtrlDPtr;
    m_act.webapi().ExecBundleImpl(*objCtrlDPtr);

    // auto subCtrlDPtr = CSubCtrlApiData::CreatePtr(this->Act(), this->selfRef(), c_RESTful_bundle, sSrv, "", sCtrlName, sMethod, sContent, sGoto, "");
}
void CCtrApilData::ExecBundleSub(const char* sSrv, const char* sCtrlName, const char* sMethod, const char* sContent, const char* sGoto)
{
    CKCWebApiWork& webapi = m_act.webapi();
    auto subCtrlDPtr = CSubCtrlApiData::CreatePtr(this->Act(), this->selfRef(), c_RESTful_bundle, sSrv, "", sCtrlName, sMethod, sContent, sGoto, "");
    webapi.ExecBundleImpl(*subCtrlDPtr);
    webapi.SubCallResult(*this, *subCtrlDPtr);
}
void CCtrApilData::ExecSo(const char* sSoFile, const char* sCtrlName, const char* sMethod, const char* sContent, const char* sGoto)
{
    struct TCtrlDataBackCallSo : CSubCtrlApiData
    {
        TCtrlDataBackCallSo(CCtrApilDataWeakPtr own, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto)
            : CSubCtrlApiData(own, c_RESTful_so, "", sSo, sCtrlName, sMethod, sContent, sGoto, "") {}
    };
    CCtrApilDataPtr objCtrlDPtr(new TCtrlDataBackCallSo(m_selfRef, sSoFile, sCtrlName, sMethod, sContent, sGoto));
    objCtrlDPtr->m_selfRef = objCtrlDPtr;
    m_act.webapi().ExecSoImpl(*objCtrlDPtr);
}
void CCtrApilData::ExecSoSub(const char* sSoFile, const char* sCtrlName, const char* sMethod, const char* sContent, const char* sGoto)
{
    CKCWebApiWork& webapi = m_act.webapi();
    auto subCtrlDPtr = CSubCtrlApiData::CreatePtr(this->Act(), this->selfRef(), c_RESTful_so, "", sSoFile, sCtrlName, sMethod, sContent, sGoto, "");
    webapi.ExecSoImpl(*subCtrlDPtr);
    webapi.SubCallResult(*this, *subCtrlDPtr);
}

// 上一个控制器返回结果json
const IKCJson& CCtrApilData::JsonRespondLast(void) const
{
    try
    {
        if (m_jsonRespondLast.IsValid()) return m_jsonRespondLast.JsonPackRespond();
    }
    catch (...) {}
    return m_act.InvalidJson();
}

// 获取上一个控制器的内部数据集
CSqlFunc::TRecordSetPtr CCtrApilData::DataSetInnerLast(string name)
{
    CSqlFunc::TRecordSetPtr resultPtr;
    if (nullptr != m_innerDbSetLast)
    {
        auto it = m_innerDbSetLast->find(name);
        if (m_innerDbSetLast->end() != it)
            resultPtr = it->second;
    }
    return resultPtr;
}

// 获取批量操作的数组参数json
const IKCJson& CCtrApilData::GetBatchArrayJson(const char* sBatchParm) const
{
    return GetBatchArrayJsonImpl(CUtilFunc::PCharSafeToStr(sBatchParm));
}
// 获取批量操作的数组参数json
const CKCJsonPack& CCtrApilData::GetBatchArrayJsonImpl(string sBatchParm) const
{
    string sBatchParmNode = !sBatchParm.empty() ? sBatchParm : JsonAttach().GetItem(c_RESTful_batchParm).GetStr(c_RESTful_batchValsName, m_act.GetValsName());
    // 1.在上一个控制器的结果里找
    if (m_jsonRespondLast.IsValid())
    {
        auto &jsonRespondLast = m_jsonRespondLast.JsonPackRespond();
        // 1.1.如果请求的节点为空，并且上一个控制器的输出是数组，直接返回
        if (sBatchParm.empty() && jsonRespondLast.IsValid() && jsonRespondLast.GetType() == IKCJson::edtArray)
            return jsonRespondLast;
        // 1.2.在上一个控制器的输出里，找指定的数组字段
        auto arrBatchPrm = const_cast<CKCJsonPackRespond&>(jsonRespondLast).ImplGetItem(sBatchParmNode.c_str(), JsonCaseSensitive());
        if (arrBatchPrm.get() != nullptr && arrBatchPrm->IsValid() && arrBatchPrm->GetType() == IKCJson::edtArray)
            return *arrBatchPrm;
    }
    // 2.如果请求的节点为空，并且请求的输入参数是数组，直接返回
    const CKCJsonPackRoot& jsonRequest = m_act.JsonReq();
    if (sBatchParm.empty() && jsonRequest.IsValid() && jsonRequest.GetType() == IKCJson::edtArray)
        return jsonRequest;
    // 3.在请求的输入参数里，找指定的数组字段
    auto arrBatchPrm = const_cast<CKCJsonPackRoot&>(jsonRequest).ImplGetItem(sBatchParmNode.c_str(), JsonCaseSensitive());
    if (arrBatchPrm.get() != nullptr && arrBatchPrm->IsValid() && arrBatchPrm->GetType() == IKCJson::edtArray)
        return *arrBatchPrm;
    // 没找到
    return m_act.JsonInvalid();
}
// 获取参数默认值json
const IKCJson& CCtrApilData::GetDefParmJson(const char* name) const
{
    const IKCJson& jsonDefVal = JsonAttach().GetItem(c_RESTful_defParm);
    if (!jsonDefVal.IsNull())
    {
        if (nullptr == name || strlen(name) == 0) return jsonDefVal;
        // 1.在当前控制器附加参数里的默认参数里找
        const IKCJson& jsonDefParmCurr = jsonDefVal.GetItem(name);
        if (jsonDefParmCurr.IsValid()) return jsonDefParmCurr;
        // todo: 遍历组控制器嵌套栈

    }
    // 没找到
    return m_act.InvalidJson();
}
// 获取用户输入参数值json
const IKCJson& CCtrApilData::GetUserParmJson(const char* name) const
{
    const IKCJson& jsonRequest = JsonRequest();
    if (!jsonRequest.IsNull())
    {
        if (nullptr == name || strlen(name) == 0) return jsonRequest;
        // 1.在请求的输入参数区域里找
        const IKCJson& jsonInParm = JsonRequest().GetItem(m_nameInParms.c_str()).GetItem(name);
        if (jsonInParm.IsValid()) return jsonInParm;
        // 2.在请求的根区域里找
        const IKCJson& jsonIn = JsonRequest().GetItem(name);
        if (jsonIn.IsValid()) return jsonIn;
    }
    // 没找到
    return m_act.InvalidJson();
}
// 获取参数值json（依次查找上一个控制器的输出、用户输入的参数、临时参数、默认的参数）
const IKCJson& CCtrApilData::GetParmJson(const char* name) const
{
    const bool bNullName = nullptr == name || strlen(name) == 0;
    // 1.从内部数据集生成的临时json
    if (!m_batchGrpInDbSetTmpJson.expired())
    {
        const IKCJson& jsonBatchInTmp = *m_batchGrpInDbSetTmpJson.lock()->m_json.get();
        if (!jsonBatchInTmp.IsNull())
        {
            const IKCJson& jsonBatchInTmpItem = jsonBatchInTmp.GetItem(name);
            if (jsonBatchInTmpItem.IsValid()) return jsonBatchInTmpItem;
        }
    }
    // 2.批量组操作的内部临时参数
    if (!m_batchGrpInTmpPrm.expired())
    try
    {
        const IKCJson& jsonBatchInTmp = *m_batchGrpInTmpPrm.lock();
        if (!jsonBatchInTmp.IsNull())
        {
            if (bNullName) return jsonBatchInTmp;
            const IKCJson& jsonBatchInTmpItem = jsonBatchInTmp.GetItem(name);
            if (jsonBatchInTmpItem.IsValid()) return jsonBatchInTmpItem;
        }
    }
    catch (...) {}
    // 3.到上一个控制器输出里找
    const IKCJson& jsonLast = JsonRespondLast();
    if (!jsonLast.IsNull())
    {
        if (bNullName) return jsonLast;
        // 3.1.到参数区域里找
        const IKCJson& jsonLastParm = jsonLast.GetItem(m_nameInParms.c_str()).GetItem(name);
        if (jsonLastParm.IsValid()) return jsonLastParm;
        // 3.2.到根区域里找
        const IKCJson& jsonLastItem = jsonLast.GetItem(name);
        if (jsonLastItem.IsValid()) return jsonLastItem;
    }
    // 4.针对有依赖的子调用
    if (!m_jsonRespond.IsNull())
    {
        if (bNullName) return m_jsonRespond;
        // 4.1.到结果参数区域里找
        const IKCJson& jsonResParm = m_jsonRespond.GetItem(m_nameInParms.c_str()).GetItem(name);
        if (jsonResParm.IsValid()) return jsonResParm;
        // 4.2.到结果根区域里找
        const IKCJson& jsonRes = m_jsonRespond.GetItem(name);
        if (jsonRes.IsValid()) return jsonRes;
    }
    // 5.在用户输入参数里找
    const IKCJson& jsonInParm = GetUserParmJson(name);
    if (jsonInParm.IsValid()) return jsonInParm;
    // 6.最后，到默认参数里找
    return GetDefParmJson(name);
}
// 获取参数值（依次查找全局数据、上一个控制器的输出、用户输入的参数、临时参数、默认的参数）
const char* CCtrApilData::GetParmVal(const char* name) const
{
    // 全局参数
    if (m_act.IsGlobalVal(name))
    {
        const char* pCfg = m_act.GetGlobalVal(name);
        if (nullptr == pCfg)
            throw TException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, string() + m_act.GetHint("Parm_Error_") + name);
        return pCfg;
    }
    // 用户参数
    else
    {
        const IKCJson& prm = GetParmJson(name);
        if (!prm.IsValid())
            throw TException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, string() + m_act.GetHint("Parm_Error_") + name);
        return prm.GetStr();
    }
}
const char* CCtrApilData::GetParmVal(const char* name, const char* def) const
{
    try
    {
        return GetParmVal(name);
    }
    catch (...) {}
    return def;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrApilDataBase 类
CCtrApilDataBase::CCtrApilDataBase(CActionData& act, string sUrl, string sGetArg, string sKCFilePath, string sSign, bool isRootRun,
                                   CKCJsonPackWeakPtr batchGrpInTmpPrm, TBatchGrpInDbSetTmpJsonWeakPtr batchGrpInDbSet)
    : m_batchGrpInTmpPrm(batchGrpInTmpPrm), m_batchGrpInDbSetTmpJson(batchGrpInDbSet)
    , m_re(act.re()), m_act(act), m_isRootRun(isRootRun), m_url(sUrl), m_kcFilePath(sKCFilePath)
    , m_node([&](){
        // get参数
        CUtilFunc::splitUrlGetParm(m_getParms, sGetArg.c_str());
        // 解析kc文件
        if (!boost::filesystem::exists(m_kcFilePath) || !boost::filesystem::is_regular_file(m_kcFilePath))
        {
            string sErr = string(act.GetHint("Don_t_exists_file_")) + m_url + " \t\t Local File As Follows:\n" + m_kcFilePath + "\r\n" + act.GetSingleInfo("the_request");
            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__);
            throw TCtrlException(ecd_ErrCode_KCWebApiWork + 21, __CURR_CODE_PLACE_C__, sErr, typeid(*this).name());
        }
        try
        {
            // boost::iostreams::mapped_file_source file(m_kcFilePath);
            read_xml(m_kcFilePath, m_pt, boost::property_tree::xml_parser::no_comments);
        }
        catch (const boost::property_tree::xml_parser_error &ex)
        {
            string sErr = (boost::format(act.GetHint("kc_format_error_") + string(" < %s >: %s")) % "KC" % ex.line() % sUrl % ex.message()).str();
            string sOther = (boost::format("<%s> %s\nLocal File As Follows: \n%s\n%s") % typeid(ex).name() % act.GetSingleInfo("the_request") % m_kcFilePath % ex.what()).str();
            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, sOther.c_str());
            throw TCtrlException(ecd_ErrCode_KCWebApiWork + 22, __CURR_CODE_PLACE_C__, sErr, typeid(*this).name());
        }
        catch (std::exception &ex)
        {
            string sErr = string(act.GetHint("Read_file_failure_")) + sUrl + ": " + ex.what();
            string sOther = (boost::format("<%s> %s\nLocal File As Follows: \n%s") % typeid(ex).name() % act.GetSingleInfo("the_request") % m_kcFilePath).str();
            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, sOther.c_str());
            throw TCtrlException(ecd_ErrCode_KCWebApiWork + 22, __CURR_CODE_PLACE_C__, sErr, typeid(*this).name());
        }
        catch (...)
        {
            string sErr = string(act.GetHint("Read_file_failure_")) + sUrl;
            string sOther = (boost::format("<%s> %s\nLocal File As Follows: \n%s") % act.GetHint("Unknown_exception") % act.GetSingleInfo("the_request") % m_kcFilePath).str();
            act.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, sOther.c_str());
            throw TCtrlException(ecd_ErrCode_KCWebApiWork + 22, __CURR_CODE_PLACE_C__, sErr, typeid(*this).name());
        }
        // 控制器根节点
        string sNode = string(c_RESTful_Controllers) + "." + ActName();
        if (!m_pt.get_child_optional(sNode))
        {
            string sErr = string(act.GetHint("No_Controller_")) + sUrl + " [" + sNode + "]. \t" + act.GetSingleInfo("the_request");
            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__);
            throw TCtrlException(ecd_ErrCode_KCWebApiWork + 22, __CURR_CODE_PLACE_C__, sErr, typeid(*this).name());
        }
        return sNode;
    }())
    , m_ctrlSign(GetXmlattrStr(c_RESTful_sign, sSign))
    , m_bundleType([&]() -> string{
        // 首选控制器节点上“type”属性指定的类型
        string sVal = GetXmlattrStr(c_RESTful_type);
        if (!sVal.empty()) return sVal;
        // 如果未指定类型，但控制器节点上包含"srv"、“bundle”或"srvCtrl"属性，则为插件类型
        sVal = GetXmlattrStr(c_RESTful_bundle);
        if (!sVal.empty()) return c_RESTful_bundle;
        sVal = GetXmlattrStr(c_RESTful_srv);
        if (!sVal.empty()) return c_RESTful_bundle;
        sVal = GetXmlattrStr(c_RESTful_srvCtrl);
        if (!sVal.empty()) return c_RESTful_bundle;
        // 如果未指定类型，但控制器节点上包含"file"属性，则为动态库类型
        sVal = GetXmlattrStr(c_RESTful_file);
        if (!sVal.empty()) return c_RESTful_so;
        // 如果未指定类型，但控制器节点上包含"exec"属性，则为组类型
        // sVal = GetXmlattrStr(c_RESTful_exec);
        // if (!sVal.empty()) return c_RESTful_group;
        // 都未指定，取默认类型（插件类型）
        return GetXmlattrStrFul(c_RESTful_type, c_RESTful_bundle);
    }())
    , m_bundleSrv([&]() -> string{
        auto& aliasLs = act.webapi().SrvCtrlAliasLs();
        auto& context = act.webapi().getContext();
        // 1、在api节点里找 srvCtrl 属性
        {
            auto [bHas, sVal] = GetXmlattrStrBase(m_node + "." + c_RESTful_xmlattr + "." + c_RESTful_srvCtrl);
            if (bHas && !sVal.empty())
            {
                auto it = aliasLs.find(sVal);
                if (aliasLs.end() != it && !it->second.m_srv.empty()) return it->second.m_srv;
            }
        }
        // 2、在api节点里找 srv 属性
        {
            auto [bHas, sVal] = GetXmlattrStrBase(m_node + "." + c_RESTful_xmlattr + "." + c_RESTful_srv);
            if (bHas && !sVal.empty()) return sVal;
        }
        // 3、在api节点里找 bundle 属性
        {
            auto [bHas, sVal] = GetXmlattrStrBase(m_node + "." + c_RESTful_xmlattr + "." + c_RESTful_bundle);
            if (bHas && !sVal.empty())
            {
                auto &bundle = context.getBundle(sVal.c_str());
                sVal = CUtilFunc::PCharSafeToPChar(bundle.getFirstServiceAlias());
                if (!sVal.empty()) return sVal;
            }
        }
        m_bundleSrvFromRoot = true;
        // 4、在文件根节点里找 srvCtrl 属性
        {
            auto [bHas, sVal] = GetXmlattrStrBase(c_RESTful_Controllers + string(".") + c_RESTful_xmlattr + "." + c_RESTful_srvCtrl);
            if (bHas && !sVal.empty())
            {
                auto it = aliasLs.find(sVal);
                if (aliasLs.end() != it && !it->second.m_srv.empty()) return it->second.m_srv;
            }
        }
        // 5、在文件根节点里找 srv 属性
        {
            auto [bHas, sVal] = GetXmlattrStrBase(c_RESTful_Controllers + string(".") + c_RESTful_xmlattr + "." + c_RESTful_srv);
            if (bHas && !sVal.empty()) return sVal;
        }
        // 6、在文件根节点里找 bundle 属性
        {
            auto [bHas, sVal] = GetXmlattrStrBase(c_RESTful_Controllers + string(".") + c_RESTful_xmlattr + "." + c_RESTful_bundle);
            if (bHas && !sVal.empty())
            {
                auto &bundle = context.getBundle(sVal.c_str());
                sVal = CUtilFunc::PCharSafeToPChar(bundle.getFirstServiceAlias());
                if (!sVal.empty()) return sVal;
            }
        }
        return "";
    }())
    , m_bundleCtrlName([&]() -> string{
        auto& aliasLs = act.webapi().SrvCtrlAliasLs();
        // 如果未设置服务，则取全局默认的服务控制器别名
        if (m_bundleSrv.empty())
        {
            // 必须是插件类型
            if (c_RESTful_bundle == m_bundleType)
            {
                auto it = aliasLs.find(act.webapi().SrvCtrlAliasDefault());
                if (aliasLs.end() != it && !it->second.m_srv.empty())
                {
                    const_cast<string&>(m_bundleSrv) = it->second.m_srv;
                    return it->second.m_ctrl;
                }
                // 未指定服务控制器
                string sErr = string(act.GetHint("Not_Assigned_Controller_")) + " [" + m_node + "]. \t" + act.GetSingleInfo("the_request");
                act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__);
                throw TCtrlException(ecd_ErrCode_KCWebApiWork + 23, __CURR_CODE_PLACE_C__, sErr, typeid(*this).name());
            }
        }
        // 取设置的控制器名称
        else
        {
            // 1、在api节点里找 srvCtrl 属性
            {
                auto [bHas, sVal] = GetXmlattrStrBase(m_node + "." + c_RESTful_xmlattr + "." + c_RESTful_srvCtrl);
                if (bHas && !sVal.empty())
                {
                    auto it = aliasLs.find(sVal);
                    if (aliasLs.end() != it && !it->second.m_srv.empty()) return it->second.m_ctrl;
                }
            }
            // 2、在api节点里找 ctrl 属性
            {
                auto [bHas, sVal] = GetXmlattrStrBase(m_node + "." + c_RESTful_xmlattr + "." + c_RESTful_ctrl);
                if (bHas && !sVal.empty()) return sVal;
            }
            // 3、在文件根节点里找 srvCtrl 属性
            if (m_bundleSrvFromRoot)
            {
                auto [bHas, sVal] = GetXmlattrStrBase(c_RESTful_Controllers + string(".") + c_RESTful_xmlattr + "." + c_RESTful_srvCtrl);
                if (bHas && !sVal.empty())
                {
                    auto it = aliasLs.find(sVal);
                    if (aliasLs.end() != it && !it->second.m_srv.empty()) return it->second.m_ctrl;
                }
            }
            // 4、在文件根节点里找 ctrl 属性
            if (m_bundleSrvFromRoot)
            {
                auto [bHas, sVal] = GetXmlattrStrBase(c_RESTful_Controllers + string(".") + c_RESTful_xmlattr + "." + c_RESTful_ctrl);
                if (bHas && !sVal.empty()) return sVal;
            }
        }
        return "";
    }())
    , m_bundleSoFile([&](){
        string sFile = GetXmlattrStrFul(c_RESTful_file);
        if (!sFile.empty()) sFile += c_so_ext_name;
        return sFile;
    }())
    , m_method(GetXmlattrStrFul(c_RESTful_method))
    , m_content([&]() -> string {
        string sContent = ReadContentFromXML();
        // 如果api节点内未指定内容
        if (sContent.empty())
        {
            auto [bHasMethod, sValMethod] = GetXmlattrStrBase(m_node + "." + c_RESTful_xmlattr + "." + c_RESTful_method);
            // 如果api节点内未指定内容的同时，也未指定方法类型。则依次取api节点内 select 、 insert 、 update 、 delete 属性的单语句内容，并设置对应的方法类型
            if (!bHasMethod || sValMethod.empty())
            {
                auto fSimContent = [&](string sMothod) -> bool
                {
                    auto [bHas, sVal] = GetXmlattrStrBase(m_node + "." + c_RESTful_xmlattr + "." + sMothod);
                    bool bResult = bHas && !sVal.empty();
                    if (bResult)
                    {
                        const_cast<string&>(m_method) = sMothod;
                        sContent = sVal;
                    }
                    return bResult;
                };
                fSimContent(c_RESTful_select) || fSimContent(c_RESTful_insert) || fSimContent(c_RESTful_update) || fSimContent(c_RESTful_delete);
            }
        }
        return sContent;
    }())
    , m_ptCtrlApi(m_pt.get_child_optional(m_node) ? m_pt.get_child(m_node) : m_pt)
    , m_charset(GetXmlattrStrFul(c_RESTful_Charset, m_act.webapi().Charset()))
    , m_jsonCaseSensitive(GetXmlattrValFul(c_RESTful_jsonCaseSensitive, m_act.webapi().JsonCaseSensitive()))
    , m_attachJson(*this, [&](){
        string sAttach;
        auto fGetAttach = [&](string sAttr)
        {
            if (m_pt.get_child_optional(sAttr))
                sAttach = boost::algorithm::trim_copy(m_pt.get<string>(sAttr));
            return !sAttach.empty();
        };
        fGetAttach(m_node + "." + c_RESTful_attach) || fGetAttach(m_node + ".<xmlattr>." + c_RESTful_attach);
        return sAttach;
    }())
    , m_attachJsonKcFile(*this, [&](){
        string sAttach;
        auto fGetAttach = [&](string sAttr)
        {
            if (m_pt.get_child_optional(sAttr))
                sAttach = boost::algorithm::trim_copy(m_pt.get<string>(sAttr));
            return !sAttach.empty();
        };
        fGetAttach(string(c_RESTful_Controllers) + ".<xmlattr>." + c_RESTful_attach);
        return sAttach;
    }())
    , m_jsonRequest(*this, m_act.JsonReq())
    , m_jsonRespond(*this)
    , m_nameInParms(GetFixParmName(c_RESTful_inParm))
{
    // 控制器初始配置信息
    m_act.WriteLogDebug((boost::format("控制器配置信息\n字符集：%s | 标识：%s") % m_charset % m_ctrlSign).str().c_str(), __CURR_CODE_PLACE_C__);
}
CCtrApilDataBase::CCtrApilDataBase(std::weak_ptr<CCtrApilDataBase> own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut)
    : m_batchGrpInTmpPrm(own.lock()->m_batchGrpInTmpPrm), m_batchGrpInDbSetTmpJson(own.lock()->m_batchGrpInDbSetTmpJson)
    , m_re(own.lock()->Act().re()), m_act(own.lock()->Act()), m_isRootRun(false)
    , m_url("NoUrl"), m_kcFilePath("NoKCFilePath")
    , m_node("NoAct")
    , m_ctrlSign(algorithm::trim_copy(sSrv) + algorithm::trim_copy(sSo))
    , m_bundleType(btype)
    , m_bundleSrv(algorithm::trim_copy(sSrv))
    , m_bundleCtrlName(algorithm::trim_copy(sCtrlName))
    , m_bundleSoFile([&](){
        string sFile = algorithm::trim_copy(sSo);
        if (!sFile.empty()) sFile += c_so_ext_name;
        return sFile;
    }())
    , m_method(sMethod)
    , m_content(algorithm::trim_copy(sContent))
    , m_ptCtrlApi(m_pt)
    , m_attachJson(*this, "")
    , m_attachJsonKcFile(*this, "")
    , m_jsonRequest(*this, m_act.JsonReq())
    , m_jsonRespond(*this)
{
    m_getParms.insert(make_pair(c_RESTful_act, "NoAct"));
    m_getParms.insert(make_pair(c_RESTful_goTo, sGoto));
    m_getParms.insert(make_pair(c_RESTful_out, sOut));
}

// 读取控制器正文
string CCtrApilDataBase::ReadContentFromXML(void)
{
    // string sContent;
    // string sContNode = m_node + "." + c_RESTful_content;
    // if (m_pt.get_child_optional(sContNode))
    //     sContent = m_pt.get<string>(sContNode);
    // else sContent = m_pt.get<string>(m_node);
    // return algorithm::trim_copy(sContent);

    string sContent;
    auto fGetContent = [&](string sAttr)
    {
        if (m_pt.get_child_optional(sAttr))
            sContent = boost::algorithm::trim_copy(m_pt.get<string>(sAttr));
        return !sContent.empty();
    };
    fGetContent(m_node + "." + c_RESTful_content) || fGetContent(m_node + ".<xmlattr>." + c_RESTful_content) || fGetContent(m_node);
    return sContent;

}

// 抛一个c++异常
void CCtrApilDataBase::Throw(const char* ex, const char* place) const
{
    throw TActionException(ecd_ErrCode_KCWebApiWork + 98, place, ex, typeid(*this).name());
}
// 获取本地化提示信息
const char* CCtrApilDataBase::GetHint(const char* key) const
{
    return m_act.GetHint(key);
}
// 各单个请求信息
const char* CCtrApilDataBase::GetSingleInfo(const char* key, const char* def) const
{
    return m_act.GetSingleInfo(key, def);
}
// Json库
std::string CCtrApilDataBase::JsonLibrary(void) const
{
    return m_act.JsonLibrary();
}
// 是否区分大小写
bool CCtrApilDataBase::JsonCaseSensitive(void) const
{
    return m_jsonCaseSensitive;
}
// 字符集
const char* CCtrApilDataBase::GetCharset(void) const
{
    return m_charset.c_str();
}
// 无效的json
IKCJson& CCtrApilDataBase::InvalidJson(void)
{
    return m_act.InvalidJson();
}
// 得到配置的日志等级
int CCtrApilDataBase::GetCfgLogLevel(void) const
{
    return m_act.webapi().getContext().GetCfgLogLevel();
}

// 获取属性值
std::tuple<bool, string> CCtrApilDataBase::GetXmlattrStrBase(string sAttrFul)
{
    bool bResult = false;
    string sResult;
    if (m_pt.get_child_optional(sAttrFul))
    {
        bResult = true;
        sResult = boost::algorithm::trim_copy(m_pt.get<string>(sAttrFul));
    }
    return std::make_tuple(bResult, sResult);
}
string CCtrApilDataBase::GetXmlattrStr(string sAttr, string sDef)
{
    // string sVal;
    // sAttr = m_node + ".<xmlattr>." + sAttr;
    // if (m_pt.get_child_optional(sAttr)) sVal = m_pt.get<string>(sAttr);
    // else sVal = sDef;
    // return algorithm::trim_copy(sVal);
    auto [bHas, sVal] = GetXmlattrStrBase(m_node + ".<xmlattr>." + sAttr);
    if (!bHas) sVal = sDef;
    return sVal;
}
// 获取属性值（包括文件全局配置）
bool CCtrApilDataBase::GetXmlattrValFul(string sAttr, bool bDef)
{
    bool bResult = bDef;
    auto fSetBool = [&](string sAttrFul)
    {
        // if (m_pt.get_child_optional(sAttrFul))
        // {
        //     string sVal = boost::algorithm::trim_copy(m_pt.get<string>(sAttrFul));
        //     algorithm::to_lower(sVal);
        //     if ("true" == sVal || "false" == sVal)
        //         bResult = "true" == sVal;
        // }
        auto [bHas, sVal] = GetXmlattrStrBase(sAttrFul);
        algorithm::to_lower(sVal);
        if (bHas && ("true" == sVal || "false" == sVal)) bResult = "true" == sVal;
    };
    // 整个文件配置
    fSetBool((boost::format("%s.%s.%s") % c_RESTful_Controllers % c_RESTful_xmlattr % sAttr).str());
    // 控制器配置
    fSetBool(m_node + ".<xmlattr>." + sAttr);
    return bResult;
}
// 获取字符串类型属性值（包括文件全局配置）
string CCtrApilDataBase::GetXmlattrStrFul(string sAttr, string sDef)
{
    string sResult = sDef;
    auto fSetStr = [&](string sAttrFul)
    {
        // if (m_pt.get_child_optional(sAttrFul))
        // {
        //     string sVal = boost::algorithm::trim_copy(m_pt.get<string>(sAttrFul));
        //     if (!sVal.empty()) sResult = sVal;
        // }

        auto [bHas, sVal] = GetXmlattrStrBase(sAttrFul);
        bool bResult = bHas && !sVal.empty();
        if (bResult) sResult = sVal;
        return bResult;
    };
    fSetStr(m_node + ".<xmlattr>." + sAttr) || fSetStr((boost::format("%s.%s.%s") % c_RESTful_Controllers % c_RESTful_xmlattr % sAttr).str());
    return sResult;
}

// 当前请求上下文信息
IKCRequestRespond& CCtrApilDataBase::GetRequestRespond(void) const
{
    return m_re;
}
// 本次执行中的整体活动数据接口
IActionData& CCtrApilDataBase::ActionData(void) const
{
    return m_re.ActionData();
}

// 控制器标识名
const char* CCtrApilDataBase::SignName(void) const
{
    return m_ctrlSign.c_str();
}

// 控制器的链接
const char* CCtrApilDataBase::Url(void) const
{
    return m_url.c_str();
}

// 本地文件（KC文件）
const char* CCtrApilDataBase::LocalFile(void) const
{
    return m_kcFilePath.c_str();
}

// 子项路径转换到本地完整路径
const char* CCtrApilDataBase::transItemUrlToFullPath(const char* url) const
{
    try
    {
        static thread_local string sUrl;
        sUrl = CUtilFunc::PCharSafeToStr(url);
        // 如果不是从根目录开始，则转换为当前页面的相对目录
        if ('/' != sUrl[0])
            sUrl = m_re.GetUrlPagePath() + string("/") + sUrl;
        // 规范化路径（去除“..”和“.”）
        //sSubFileUrl = boost::filesystem::canonical(sSubFileUrl).string();
        sUrl = m_re.CanonicalUrl(sUrl.c_str());
        // 转换为本地路径
        sUrl = m_re.GetLocalFilename(sUrl.c_str());
        return sUrl.c_str();
    }
    catch (TException &ex)
    {
        throw TCtrlException(ex.error_id(), ex.error_place(), (boost::format("[%s (Subitem)] %s") % this->SignName() % ex.what()).str(), this->m_bundleCtrlName);
    }
}

// 活动控制器名称
const char* CCtrApilDataBase::ActName(void) const
{
    const char* pAct = GetGetArg(c_RESTful_act);
    return nullptr != pAct && strlen(pAct) > 0 ? pAct : c_RESTful_main;
}

// 控制器xml全节点名
const char* CCtrApilDataBase::NodeName(void) const
{
    return m_node.c_str();
}

// 插件类型（服务、动态库、组等）
const char* CCtrApilDataBase::Type(void) const
{
    return m_bundleType.c_str();
}

// 插件对应服务名（服务类型）
const char* CCtrApilDataBase::Srv(void) const
{
    if (m_bundleSrv.empty())
        throw TCtrlException(ecd_ErrCode_KCWebApiWork + 23, __CURR_CODE_PLACE_C__, (boost::format("%s %s.<xmlattr>.%s\n%s") % m_re.ActionData().GetHint("Don_t_exists_field_") % m_node % c_RESTful_srv % m_re.GetSingleInfo("the_request")).str(), typeid(*this).name());
    return m_bundleSrv.c_str();
}

// 插件对应动态库文件（动态库类型）
const char* CCtrApilDataBase::SoFile(void) const
{
    if (m_bundleSoFile.empty())
        throw TCtrlException(ecd_ErrCode_KCWebApiWork + 24, __CURR_CODE_PLACE_C__, (boost::format("%s %s.<xmlattr>.%s\n%s") % m_re.ActionData().GetHint("Don_t_exists_field_") % m_node % c_RESTful_file % m_re.GetSingleInfo("the_request")).str(), typeid(*this).name());
    return m_bundleSoFile.c_str();
}

// 对应服务控制器名称
const char* CCtrApilDataBase::CtrlName(void) const
{
    return m_bundleCtrlName.c_str();
}

// 控制器方法类型
const char* CCtrApilDataBase::Method(void) const
{
    return m_method.c_str();
}

// 控制器正文
const char* CCtrApilDataBase::Content(void) const
{
    return m_content.c_str();
}

// 是否根调用
bool CCtrApilDataBase::IsRootRun(void) const
{
    return m_isRootRun;
}

// 获取GET参数
const char* CCtrApilDataBase::GetGetArg(const char* key, const char* def) const
{
    auto it = m_getParms.find(CUtilFunc::PCharSafeToStr(key));
    if (m_getParms.end() != it) return it->second.c_str();
    else return def;
}
// 获取POST参数
const char* CCtrApilDataBase::GetPostStr(void) const
{
    return m_re.GetPostArgStr();
}

// 得到子节点信息
const char* CCtrApilDataBase::GetSubInfo(const char* sNode, const char* sAttr, const char* sDefault) const
{
    static thread_local string sResult;
    sResult = CUtilFunc::PCharSafeToStr(sDefault);
    string strNode = m_node + "." + CUtilFunc::PCharSafeToStr(sNode);
    string strAttr = CUtilFunc::PCharSafeToStr(sAttr);
    if (!strNode.empty() && !m_pt.empty())
    {
        string strNodeAttr = strNode;
        if (!strAttr.empty())
            strNodeAttr = strNode + ".<xmlattr>." + strAttr;
        if (m_pt.get_child_optional(strNodeAttr))
            sResult = m_pt.get<string>(strNodeAttr);
    }
    return sResult.c_str();
}
// 得到其他信息
const char* CCtrApilDataBase::GetOtherInfo(const char* name, const char* sDefault) const
{
    string sName = CUtilFunc::PCharSafeToStr(name);
    if ("FilePartFlag/begin" == sName)
        return m_act.webapi().FilePartFlagB();
    else if ("FilePartFlag/end" == sName)
        return m_act.webapi().FilePartFlagE();
    return sDefault;
}

// 控制器返回结果json
IKCJson& CCtrApilDataBase::JsonRespond(void)
{
    return this->JsonRes();
}
// 控制器返回结果json
void CCtrApilDataBase::SetJsonRespond(int errNo, const char* errMsg)
{
    IKCJson& jsonRespond = JsonRespond();
    jsonRespond.SetVal(c_RESTful_errCode, errNo);
    jsonRespond.SetStr(c_RESTful_errMsg, errMsg);
    cout << "*[Knewcode] Respond Msg: " << errMsg << endl;
}
// 控制器附加参数的json
const IKCJson& CCtrApilDataBase::JsonAttach(void) const
{
    return this->m_attachJson;
}
const IKCJson& CCtrApilDataBase::JsonAttachByKcFile(void) const
{
    return this->m_attachJsonKcFile;
}
// 输入参数的json
const IKCJson& CCtrApilDataBase::JsonRequest(void) const
{
    return m_act.JsonRequest();
}

// 保存内部数据集
void CCtrApilDataBase::DataSetSaveToInner(CSqlFunc::TRecordSetPtr dbset)
{
    auto it = m_innerDbSet.find(dbset->m_name);
    if (m_innerDbSet.end() != it) it->second = dbset;
    else m_innerDbSet.insert(make_pair(dbset->m_name, dbset));
}

// 获取固定字段的名称
std::string CCtrApilDataBase::GetFixParmName(std::string prmName) const
{
    return m_attachJson.GetItem(c_RESTful_fixedParmName).GetStr(prmName.c_str(), prmName.c_str());
}

// 执行结果
void CCtrApilDataBase::SetPerformResult(void)
{
    // 在应答的json里，添加字符集
    if (m_charset != c_RESTful_UTF8 && JsonAttach().GetItem(c_RESTful_outParm).HasItem(c_RESTful_Charset) && !JsonRespond().HasItem(c_RESTful_Charset))
        JsonRespond().SetStr(c_RESTful_Charset, m_charset.c_str());
    // 日志
    if (JsonRespond().HasItem(c_RESTful_logMsg))
    {
        string sLogMsg = JsonRespond().GetStr(c_RESTful_logMsg, "");
        if (!sLogMsg.empty()) Act().WriteLogInfo((boost::format("ExecBundle logMsg: %s - %s\n%s") % CUtilFunc::PCharSafeToStr(LocalFile()) % CUtilFunc::PCharSafeToStr(ActName()) % sLogMsg).str().c_str(), __CURR_CODE_PLACE_C__);
    }
}
