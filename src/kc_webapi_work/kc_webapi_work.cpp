#include "kc_webapi_work.h"
#include "action_data.h"
#include "ctrlapi_data.h"

////////////////////////////////////////////////////////////////////////////////
// CKCWebApiWork::TKCAliveObj类
CKCWebApiWork::TKCAliveObj::TKCAliveObj(CKCWebApiWork& own, string id) : m_own(own), m_aliveID(id)
{
}
CKCWebApiWork::TKCAliveObj::~TKCAliveObj(void)
{
    for (auto it : m_actObj)
    {
        try
        {
            it.second->Release();
            it.second = nullptr;
        }
        catch (...) {}
    }
    m_actObj.clear();
}
const char* CKCWebApiWork::TKCAliveObj::AliveID(void)
{
    return m_aliveID.c_str();
}
// 活动对象
void CKCWebApiWork::TKCAliveObj::AddActObj(const char* sName, IActionData::TActObj* pObj)
{
    if (nullptr == pObj)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork_NullPointActObj, __CURR_CODE_PLACE_C__, string(m_own.getHint("Null_Point_Data_")) + sName, m_own);
    boost::unique_lock<boost::shared_mutex> lck(m_mtx);
    auto it = m_actObj.find(sName);
    if (m_actObj.end() != it)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork_RepeatActObj, __CURR_CODE_PLACE_C__, string(m_own.getHint("Repeat_Register_Data_")) + sName, m_own);
    m_actObj.insert(make_pair(string(sName), pObj));
}
void CKCWebApiWork::TKCAliveObj::DelActObj(const char* sName)
{
    boost::unique_lock<boost::shared_mutex> lck(m_mtx);
    auto it = m_actObj.find(sName);
    if (m_actObj.end() != it)
    {
        if (nullptr != it->second) it->second->Release();
        m_actObj.erase(it);
    }
}
IActionData::TActObj* CKCWebApiWork::TKCAliveObj::GetActObj(const char* sName)
{
    boost::shared_lock<boost::shared_mutex> lck(m_mtx);
    auto it = m_actObj.find(sName);
    if (m_actObj.end() == it) return nullptr;
    else return it->second;
}

////////////////////////////////////////////////////////////////////////////////
// CKCWebApiWork类
CKCWebApiWork::CKCWebApiWork(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle)
    , m_sessionRef(*m_context.takeServiceReference(c_KCSessionCookieSrvGUID))
    , m_cfgMod(string("Config.Modules.") + m_bundle.getName()), m_cfgModCtrl(m_cfgMod + "." + c_RESTful_Controllers)
    , m_NeedToken(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), c_RESTful_needToken))
    , m_innerSessionCtrl(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), "innerSessionCtrl"))
    , m_GrpBranchCtrl(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), c_RESTful_grpBranchCtrl, m_context.GetSysFlag("customer", "KC")))
    , m_jsonCaseSensitive(algorithm::to_lower_copy(string(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), c_RESTful_jsonCaseSensitive))) == "true")
    , m_backSessionID(algorithm::to_lower_copy(string(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), "backSessionID"))) == "true")
    , m_charset(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), c_RESTful_Charset, c_RESTful_UTF8))
    , m_jsonLibrary(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), c_RESTful_jsonLibrary, c_RESTful_cJSON))
    , m_keepAliveTimeout(atoi(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), c_RESTful_keepAliveTimeout, "600")))
    , m_recursionLayerMax(atoi(m_context.GetCfgInfo(m_cfgModCtrl.c_str(), c_RESTful_recursionLayerMax, "128")))
    , m_initAct(m_context.GetCfgInfo((m_cfgModCtrl + ".init").c_str(), c_RESTful_uri))
    , m_checkTokenAct(m_context.GetCfgInfo((m_cfgModCtrl + ".checkToken").c_str(), c_RESTful_uri))
    , m_prepAct(m_context.GetCfgInfo((m_cfgModCtrl + ".prep").c_str(), c_RESTful_uri))
    , m_postpAct(m_context.GetCfgInfo((m_cfgModCtrl + ".postp").c_str(), c_RESTful_uri))
    , m_apiUriLs([&](){
        // auto &wrk = m_context.GetMain();
        map<string, TApiUri> setRes;
        // 授权执行webapi的uri路径。如果未指定，则所有的uri路径都可执行webapi
        string sLogInfo = "GrantUriPath:\n";
        string sGrantUriNode = m_cfgMod + ".GrantUriPath";
        for (int i = 0, c = m_context.GetCfgSubCount(sGrantUriNode.c_str()); i < c; ++i)
        {
            string sName = m_context.GetCfgSubName(sGrantUriNode.c_str(), i);
            if (c_RESTful_xmlattr != sName)
            {
                string sUri = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(m_context.GetCfgSubInfo(sGrantUriNode.c_str(), i, c_RESTful_uri)));
                string sLocalDir = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(m_context.GetCfgSubInfo(sGrantUriNode.c_str(), i, "localDir")));
                string sLocalDirFull = m_context.transCfgPathToFullPath(sLocalDir.c_str());
                bool bExists = boost::filesystem::exists(sLocalDirFull);
                sLogInfo += (bExists ? "v\t" : " \t") + sUri + " \t\t" + sLocalDir + " (" + sLocalDirFull + ")\n";
                bool bGrant = CUtilFunc::StrToLower(CUtilFunc::PCharSafeToStr(m_context.GetCfgSubInfo(sGrantUriNode.c_str(), i, "grant", "true"))) == "true";
                if (!sUri.empty())
                {
                    auto it = setRes.find(sUri);
                    if (setRes.end() == it)
                        setRes.insert(std::make_pair(sUri, TApiUri(sUri, sLocalDirFull, bGrant)));
                }
            }
        }
        WriteLogDebug(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
        return setRes;
    }())
    , m_srvCtrlAliasDefault(m_context.GetSysFlag("customer", "KC"))
    , m_srvCtrlAliasLs([&](){
        map<string, TSrvCtrlAlias> setRes;
        string sSrvCtrl = m_cfgMod + ".SrvCtrl";
        // 默认的返回数据集的名称
        auto srvDefaultValsName = m_context.GetCfgInfo(sSrvCtrl.c_str(), "valsName");
        if (nullptr != srvDefaultValsName && strlen(srvDefaultValsName) > 0)
            const_cast<string&>(m_defaultValsName) = srvDefaultValsName;
        // 默认的控制器别名
        auto srvCtrlAliasDefault = m_context.GetCfgInfo(sSrvCtrl.c_str(), "default");
        if (nullptr != srvCtrlAliasDefault && strlen(srvCtrlAliasDefault) > 0)
            const_cast<string&>(m_srvCtrlAliasDefault) = srvCtrlAliasDefault;
        string sLogInfo = "SrvCtrlAliasList, [" + m_srvCtrlAliasDefault + "]:\n";
        // 循环添加控制器别名
        for (int i = 0, c = m_context.GetCfgSubCount(sSrvCtrl.c_str()); i < c; ++i)
        {
            string sName = m_context.GetCfgSubName(sSrvCtrl.c_str(), i);
            if (c_RESTful_xmlattr != sName)
            {
                string sBundle = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(m_context.GetCfgSubInfo(sSrvCtrl.c_str(), i, c_RESTful_bundle)));
                string sSrv = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(m_context.GetCfgSubInfo(sSrvCtrl.c_str(), i, c_RESTful_srv)));
                string sCtrl = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(m_context.GetCfgSubInfo(sSrvCtrl.c_str(), i, c_RESTful_ctrl)));
                sLogInfo += " \t\t" + sName + " \t=> " + sBundle + "::" + sSrv + "." + sCtrl + "\n";
                if (sSrv.empty() && !sBundle.empty())
                {
                    auto &bundle = m_context.getBundle(sBundle.c_str());
                    sSrv = CUtilFunc::PCharSafeToPChar(bundle.getFirstServiceAlias());
                }
                if (!sSrv.empty() && !sCtrl.empty())
                {
                    auto it = setRes.find(sName);
                    if (setRes.end() == it) setRes.insert(std::make_pair(sName, TSrvCtrlAlias(sSrv, sCtrl)));
                }
            }
        }
        WriteLogDebug(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
        if (setRes.end() == setRes.find(m_srvCtrlAliasDefault))
        {
            string sErr = "Not Exists SrvCtrlAlias - " + m_srvCtrlAliasDefault;
            WriteLogWarning(sErr.c_str(), __CURR_CODE_PLACE_C__);
            cout << sErr << endl;
        }
        return setRes;
    }())
    , m_filePartFlag(m_cfgMod + ".FilePartFlag")
    , m_FilePartFlagB(m_context.GetCfgInfo(m_filePartFlag.c_str(), "begin", "------part::%s{{"))
    , m_FilePartFlagE(m_context.GetCfgInfo(m_filePartFlag.c_str(), "end", "------}}part::~%s"))
    , m_taskTimeExec(m_cfgMod + ".TaskTimeExec")
    , m_TaskExecManaFlag(m_context.GetCfgInfo(m_taskTimeExec.c_str(), "manaFlag", "KC__SESSION_global__taskManaFlag"))
    , m_TaskExecGet(m_context.GetCfgInfo((m_taskTimeExec + ".GetTasks").c_str(), c_RESTful_uri))
    , m_TaskExecFinish(m_context.GetCfgInfo((m_taskTimeExec + ".FinishTask").c_str(), c_RESTful_uri))
    , m_TaskExecInterval(atoi(m_context.GetCfgInfo(m_taskTimeExec.c_str(), "interval", "60")))
    , m_guardThrdEnd(false)
    , m_GuardThrd(&CKCWebApiWork::GuardThrd, this)
{
    // ExceptBacktrace::SetExceptFilter(m_context.getPath());
    // 记录函数地址
    WriteLogDebug((string("kc_webapi_work:")
                            + "\nCKCWebApiWork::RenderREST: \t\t" + CUtilFunc::GetFuncAddr(&CKCWebApiWork::RenderREST)
                            + "\nCKCWebApiWork::Work: \t\t" + CUtilFunc::GetFuncAddr(&CKCWebApiWork::Work)
                            + "\nCKCWebApiWork::ExecCtrlApi: \t\t" + CUtilFunc::GetFuncAddr(&CKCWebApiWork::ExecCtrlApi)
                            + "\nCKCWebApiWork::SubCallCtrlApi: \t\t" + CUtilFunc::GetFuncAddr(&CKCWebApiWork::SubCallCtrlApi)
                       ).c_str(), __CURR_CODE_PLACE_C__);
    // 系统初始信息
    WriteLogInfo((boost::format("系统配置信息\n默认字符集：%s | 系统标识：%s") % m_charset % m_GrpBranchCtrl).str().c_str(), __CURR_CODE_PLACE_C__);
}

CKCWebApiWork::~CKCWebApiWork()
{
}

// 初始化/释放
bool CKCWebApiWork::init(void)
{
    // 添加响应头
    auto fAddHeader = [&](string key, string val)
    {
        if (!key.empty() && !val.empty())
        {
            auto iter = m_CfgHeader.find(key);
            if (m_CfgHeader.end() == iter)
                m_CfgHeader.insert(make_pair(key, val));
            else iter->second = val;
        }
    };
    // 配置中的响应头信息
    string sHeaderNode = m_cfgMod + ".Header";
    for (int i = 0, c = m_context.GetCfgSubCount(sHeaderNode.c_str()); i < c; ++i)
        if (m_context.IsCfgSubValid(sHeaderNode.c_str(), i))
            fAddHeader(m_context.GetCfgSubInfo(sHeaderNode.c_str(), i, "key"), m_context.GetCfgSubInfo(sHeaderNode.c_str(), i, "value"));
    return true;
}
bool CKCWebApiWork::free(void)
{
    try
    {
        m_guardThrdEnd = true;
        GC();
        IServiceReference *srvRef = nullptr;
        for (auto &itCtrl : m_refs)
        {
            srvRef = itCtrl.second;
            m_context.freeServiceReference(srvRef);
        }
        srvRef = &m_sessionRef;
        m_context.freeServiceReference(srvRef);
        // 终止守护线程
        if (m_GuardThrd.joinable()) m_GuardThrd.interrupt();
        if (m_GuardThrd.joinable()) m_GuardThrd.timed_join(boost::posix_time::milliseconds(500));
        // 清除持久对象
        m_aliveObjMap.clear();
        m_aliveObjList.clear();
    }
    catch (...) {}
    return true;
}

// 垃圾回收
void CKCWebApiWork::GC(void)
{
    try
    {
        m_dlls.clear();
    }
    catch (...) {}
}

// session接口
IKCSessionCookie& CKCWebApiWork::Session(void)
{
    return dynamic_cast<IServiceReferenceEx*>(&m_sessionRef)->getServiceSafe<IKCSessionCookie>();
}

// 得到服务特征码
const char* CKCWebApiWork::getGUID(void) const
{
    return c_KCWebApiWorkSrvGUID;
}

// 对应的模块
const IBundle& CKCWebApiWork::getBundle(void) const
{
    return m_bundle;
}

// 初始化系统
void CKCWebApiWork::InitSys(void)
{
    int iID = -1;
    string sJson, sParm;
    auto fExecMsg = [&](string stt, string msg, string msg2 = "")
    {
        return iID < 0  ? (boost::format("<System Initial> %s.\n\n%s\n%s\n%s\n") % stt % sJson % msg % msg2).str()
                        : (boost::format("<System Initial> %s.\nid = %d\nreq = %s\npar = %s\n\n%s\n%s\n%s\n") % stt % iID % m_initAct % sParm % sJson % msg % msg2).str();
    };
    // 执行系统初始化控制器
    if (!m_initAct.empty())
    try
    {
        const unsigned iSrvID = getContext().GetSrvID();
        const int iProcID = this_process::get_id();
        string sFlag = (format("%d-%d-%d") % iSrvID % iProcID % CUtilFunc::GetCurrentStampS()).str();
        // 控制器调用参数
        string sCookie = (boost::format("KCCLNID=%1%; KCSSID=%1%") % c_RESTful_KCGlobalKCSSID).str();
        sParm = (boost::format(R"({"SrvID":"%d", "ProcID":"%d", "Flag":"%s"})") % iSrvID % iProcID % sFlag).str();
        // 读取符合执行的任务列表
        sJson = CCltrFunc::CallCtrl(*this, m_initAct, sCookie, sParm);
        cout << sJson << endl;
        this->WriteLogDebug(("<System Initial> - " + sFlag).c_str(), __CURR_CODE_PLACE_C__);
    }
    catch(TException& ex)
    {
        this->WriteLogFatal(fExecMsg("<System Initial> Failed", ex.error_info()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch(std::exception& ex)
    {
        this->WriteLogFatal(fExecMsg("<System Initial> Failed", ex.what()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        this->WriteLogFatal(fExecMsg("<System Initial> Failed", this->getHint("Unknown_exception")).c_str(), __CURR_CODE_PLACE_C__);
    }
}
void CKCWebApiWork::FreeSys(void)
{

}

// 获取uri的本地地址
const char* CKCWebApiWork::GetApiUrlLocalDir(const char* uri)
{
    auto it = m_apiUriLs.find(uri);
    if (m_apiUriLs.end() != it) return it->second.m_localDir.c_str();
    return nullptr;
}
// 得到本地完整文件名
const char* CKCWebApiWork::GetUrlLocalPath(const char* uri)
{
    string sUri = CUtilFunc::PCharSafeToStr(uri);
    // 从配置的虚拟路径列表里获取本地路径
    string sLocalFile = CUtilFunc::TransVPathToLocal(m_apiUriLs, sUri);
    if (!sLocalFile.empty())
    {
        static thread_local string sResult;
        // sResult = CUtilFunc::ToAbsPath(sLocalFile, m_context.getWebsiteRootPath());
        sResult = CUtilFunc::PCharSafeToStr(m_context.transCfgPathToFullPath(sLocalFile.c_str()));
        // 判断本地文件是否存在
        if (boost::filesystem::exists(sResult)) return sResult.c_str();
        {
            string sErr = (boost::format("%s %s\n%s (%s)") % this->getHint("Don_t_exists_file_") % sUri % sResult % sLocalFile).str();
            this->WriteLogWarning(sErr.c_str(), __CURR_CODE_PLACE_C__);
        }
    }
    // 返回空
    return "";
}
// 得到网络根路径
const char* CKCWebApiWork::GetUrlPageRootPath(const char* uri)
{
    static thread_local string sResult;
    // sResult = CUtilFunc::PCharSafeToStr(uri);
    // for (auto it = m_apiUriLs.find(sResult); m_apiUriLs.end() == it; it = m_apiUriLs.find(sResult))
    // {
    //     sResult = boost::filesystem::path(sResult).parent_path().string();
    //     if ("/" == sResult || sResult.empty()) break;
    // }
    sResult = CUtilFunc::GetUrlRootByVPath(m_apiUriLs, CUtilFunc::PCharSafeToStr(uri));
    return sResult.c_str();
}

// 处理请求
int CKCWebApiWork::RenderREST(IKCRequestRespondData& re)
{
    // 执行
    string sJsonContentType = c_DefaultResponseContentType, sErrNoName = c_RESTful_errCode, sErrMsgName = c_RESTful_errMsg;
    try
    {
        // 本次活动数据
        CActionData actData(*this, re);
        // 打卡持久对象
        GetAliveObjNotCreate(actData.GetAliveID());
        // 执行
        auto resCore = Work(actData, [&](CRootCtrlApiData& objCtrlD){
            auto &parmRes = objCtrlD.JsonRes().Kernel().m_parm;
            sJsonContentType = parmRes.JsonContentType();
            sErrNoName = parmRes.m_nameErrCode;
            sErrMsgName = parmRes.m_nameErrMsg;
        });
        if (resCore.get() == nullptr || (resCore->Parm().IsJson() && !resCore->Json().IsValid()) || (!resCore->Parm().IsJson() && resCore->Parm().GetContentLength() == 0))
            throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork_InvalidRespondRoot, __CURR_CODE_PLACE_C__, string(this->getHint("Request_Failed_")) + actData.GetSingleInfo("the_request"), *this);
        // 设置返回类型
        re.SetResponseContentType(resCore->Parm().GetContentType());
        // json格式的返回
        if (resCore->Parm().IsJson())
        {
            string sContext = resCore->Json().ToStr(this->getContext().GetCfgLogLevel() <= 1);
            re.AddResponseHeader(c_WebHeader_ContentLength, std::to_string(sContext.size()).c_str());
            re.AddResponseBody(resCore->Json().ToStr(this->getContext().GetCfgLogLevel() <= 1));
        }
        // 其他类型数据的返回
        else
        {
            // 输出头
            auto &header = resCore->m_parm.Header();
            for (auto &itm : header)
                re.AddResponseHeader(itm.first.c_str(), itm.second.c_str());
            // 输出内容
            const char* pContent = resCore->Parm().GetContent();
            unsigned iContentLength = resCore->Parm().GetContentLength();
            re.AddResponseHeader(c_WebHeader_ContentLength, std::to_string(iContentLength).c_str());
            re.AddResponseBody(pContent, iContentLength);
        }
        return resCore->GetErrCode();
    }
    catch(TException& ex)
    {
        this->WriteLogError((this->getHint("Action_page_") + ex.error_info()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        re.SetResponseContentType(sJsonContentType.c_str());
        re.AddResponseBody(CCtrlCommon::GetRespondJson(ex.error_id(), string(this->getHint("Action_page_")) + ex.what(), sErrNoName, sErrMsgName));
    }
    catch(std::exception& e)
    {
        this->WriteLogFatal((this->getHint("Action_page_") + string(e.what())).c_str(), __CURR_CODE_PLACE_C__, typeid(e).name());
        re.SetResponseContentType(sJsonContentType.c_str());
        re.AddResponseBody(CCtrlCommon::GetRespondJson(ecd_ErrCode_KCWebApiWork_RequestFail, string(this->getHint("Action_page_")) + e.what(), sErrNoName, sErrMsgName));
    }
    catch (...)
    {
        this->WriteLogFatal(this->getHint("Action_page_"), __CURR_CODE_PLACE_C__, this->getHint("Unknown_exception"));
        re.SetResponseContentType(sJsonContentType.c_str());
        re.AddResponseBody(CCtrlCommon::GetRespondJson(ecd_ErrCode_KCWebApiWork_RequestFail, string(this->getHint("Action_page_")) + this->getHint("Unknown_exception"), sErrNoName, sErrMsgName));
    }
    return -10;
}

// 执行请求
CKCJsonPackRespond::TCoreParmJsonRespondPtr CKCWebApiWork::Work(CActionData& act, std::function<void(CRootCtrlApiData&)> fcbNewCtrl)
{
    // 退出时，输出配置的动态响应头信息
    CAutoRelease autoRun([&]()
    {
        try
        {
            for (auto &h: m_CfgHeader)
            {
                string sKeyName = h.first;
                string sValName = h.second;
                string sValue = act.GetGlobalVal(sValName.c_str(), sValName.c_str());
                act.re().AddResponseHeader(sKeyName.c_str(), sValue.c_str());
            }
        }
        catch (...) {}
    });
    // 异常
    auto fExceptDeal = [&](TException& ex, int errCode, string sOther = "")
    {
        string sOth = (boost::format("\n  Current Flag: %s\n  Root Flag: %s\n  Root Local KC: %s")
                       % act.GetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_sign).c_str())
                       % act.GetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_rootSign).c_str())
                       % act.re().getRe().GetLocalFilename()).str();
        ex.OtherInfo() += sOth;
        TLogInfo log(ex);
        log.m_place = __CURR_CODE_PLACE_C__;
        log.m_other = sOther;
        this->WriteLog(log);
        //string sPos = "\r\n" + lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(boost::this_thread::get_id()) + "\r\n";
        // CUtilFunc::GetRespondJson(act, errCode, ex.CurrPosInfo() + ex.what());
        throw TKCWebApiWorkException(errCode, __CURR_CODE_PLACE_C__, ex.CurrPosInfo() + ex.what(), *this);
    };
    try
    {
        // 授权执行webapi的uri路径。如果未指定，则所有的uri路径都可执行webapi
        if (!m_apiUriLs.empty() && !act.re().getRe().IsSubCall())
        {
            // string sUrlRoot = act.re().GetUrlPageRootPath();
            string sUrlRoot = this->GetUrlPageRootPath(act.re().GetUriFilename());
            auto it = m_apiUriLs.find(sUrlRoot);
            if (m_apiUriLs.end() == it || !it->second.m_grant)
                throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork_ApiNoAccess, __CURR_CODE_PLACE_C__, string(this->getHint("No_Access_")) + "[" + sUrlRoot + "]  " + act.GetSingleInfo("the_request"), *this);
        }
        // 根控制器api数据接口
        auto rootCtrlDPtr = CRootCtrlApiData::CreatePtr(act);
        auto &objCtrlD = *rootCtrlDPtr;
        fcbNewCtrl(objCtrlD);
        // 根控制器的预先操作
        if (objCtrlD.IsRootRun())
        {
            const property_tree::ptree &pt = objCtrlD.Pt();
            string sAttr = objCtrlD.NodeName() + string(".<xmlattr>.");
            // 根控制器标识
            act.SetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_rootSign).c_str(), objCtrlD.SignName());
            // 是否保持长链接
            string sKeepAlive = "off";
            if (pt.get_child_optional(sAttr + c_RESTful_keepAlive)) sKeepAlive = pt.get<string>(sAttr + c_RESTful_keepAlive);
            act.SetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_keepAlive).c_str(), sKeepAlive.c_str());
            // 判断有无权限执行控制器
            CheckToken(objCtrlD, pt, sAttr + c_RESTful_needToken, objCtrlD.NodeName());
        }
        // 执行控制器api
        ExecCtrlApi(objCtrlD);
        // 结果
        auto &resKernel = objCtrlD.JsonRes().Kernel();
        resKernel.SaveErrInfo();
        const int errCode = resKernel.GetErrCode();
        string errMsg = resKernel.GetErrMsg();
        if (0 != errCode)
            this->WriteLogWarning(("🚨 " + std::to_string(errCode) + "-" + errMsg).c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        else
            this->WriteLogTrace(("✅ " + std::to_string(errCode) + "-" + errMsg).c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        // 回传“活动数据”中的ClientID和SessionID
        if (m_backSessionID)
        {
            string sClnID = act.JsonRes().GetStr(c_RESTful_KCClientID, "");
            string sSSID = act.JsonRes().GetStr(c_RESTful_KCSessoinID, "");
            if (!sClnID.empty()) objCtrlD.JsonRes().SetStr(c_RESTful_KCClientID, sClnID.c_str());
            if (!sSSID.empty()) objCtrlD.JsonRes().SetStr(c_RESTful_KCSessoinID, sSSID.c_str());
        }
        // 只输出一个参数
        objCtrlD.JsonRes().OutParmSubItem(boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(objCtrlD.GetGetArg(c_RESTful_out))));
        // 移出
        auto coreMov = objCtrlD.MoveOutResUp();
        if (coreMov.get() == nullptr)
            throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork_InvalidRespondRoot, __CURR_CODE_PLACE_C__, string(this->getHint("Null_Point_Data_")) + act.GetSingleInfo("the_request"), *this);
        // 根控制器api的结果，转移到“活动数据”中
        act.JsonRes().MoveIn(coreMov);
        // 返回
        return coreMov;
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        fExceptDeal(ex, ex.error_id());
    }
    catch(property_tree::file_parser_error& e)
    {
        TException ex(ecd_ErrCode_KCWebApiWork_KCFileFmtErr, __CURR_CODE_PLACE_C__, e.message() + "\n" + e.filename() + " - " + std::to_string(e.line()));
        ex.LineCode() = __LINE__;
        ex.ExceptType() = typeid(e).name();
        fExceptDeal(ex, ecd_ErrCode_KCWebApiWork_KCFileFmtErr, CBacktraceSymbols::Get());
    }
    catch(std::exception& e)
    {
        TException ex(ecd_ErrCode_KCWebApiWork_RequestFail, __CURR_CODE_PLACE_C__, e.what());
        ex.LineCode() = __LINE__;
        ex.ExceptType() = typeid(e).name();
        fExceptDeal(ex, ecd_ErrCode_KCWebApiWork_RequestFail, CBacktraceSymbols::Get());
    }
    catch(const char* e)
    {
        TException ex(ecd_ErrCode_KCWebApiWork_RequestFail, __CURR_CODE_PLACE_C__, e);
        ex.LineCode() = __LINE__;
        ex.ExceptType() = "const char*";
        fExceptDeal(ex, ecd_ErrCode_KCWebApiWork_RequestFail, CBacktraceSymbols::Get());
    }
    catch(int e)
    {
        TException ex(e, __CURR_CODE_PLACE_C__, std::to_string(e).c_str());
        ex.LineCode() = __LINE__;
        ex.ExceptType() = "int";
        fExceptDeal(ex, e, CBacktraceSymbols::Get());
    }
    catch(...)
    {
        TException ex(ecd_ErrCode_KCWebApiWork_RequestFail, __CURR_CODE_PLACE_C__, this->getHint("Unknown_exception"));
        ex.LineCode() = __LINE__;
        ex.ExceptType() = "Unknown";
        fExceptDeal(ex, ecd_ErrCode_KCWebApiWork_RequestFail, CBacktraceSymbols::Get());
    }
    throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork_RequestFail, __CURR_CODE_PLACE_C__, this->getHint("Unknown_exception"), *this);
}
// *** 执行控制器（通用入口） ***
void CKCWebApiWork::ExecCtrlApi(CCtrApilData &objCtrlD)
{
    auto &act = objCtrlD.Act();
    // 控制器标识
    act.SetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_sign).c_str(), objCtrlD.SignName());
    // 控制器根节点和各属性根名称
    const property_tree::ptree &pt = objCtrlD.Pt();
    string sAttr = objCtrlD.NodeName() + string(".<xmlattr>.");
    // 根调用控制器（客户端直接调用的控制器）
    if (objCtrlD.IsRootRun())
    {
        // todo: 调用配置文件中配置的前置控制器（m_prepAct）
    }
    // 子控制器
    else
    {
    }
    // 区分大小写
    // act.m_jsonCaseSensitive = JsonCaseSensitive(pt, sAttr + c_RESTful_jsonCaseSensitive, JsonCaseSensitive());
    // 执行依赖
    string sActNode = string(c_RESTful_Controllers) + "." + objCtrlD.ActName();
    string sDepend = sActNode + "." + c_RESTful_depend;
    if (pt.get_child_optional(sDepend))
    {
        ExecGroup(objCtrlD, sDepend);
        // 有错误，退出
        if (0 != objCtrlD.JsonRespond().GetVal(c_RESTful_errCode, 0))
            return;
    }
    // 类型
    string sType = objCtrlD.Type();
    // 控制器正文引用（用于替换）
    string &sContent = objCtrlD.ContentRef();
    // 替换包含的脚本
    for (unsigned i = 0; i < m_recursionLayerMax && CScriptFunc::ReplaceInclude(objCtrlD, sContent, m_FilePartFlagB, m_FilePartFlagE, m_recursionLayerMax) > 0; ++i);
    // 替换包含的参数
    CScriptFunc::ReplaceParm(objCtrlD, sContent);
    // 插件
    if (c_RESTful_bundle == sType) ExecBundle(objCtrlD);
    // 动态库
    else if (c_RESTful_so == sType) ExecSo(objCtrlD);
    // 组操作
    else if (c_RESTful_group == sType)
        ExecGroup(objCtrlD, objCtrlD.NodeName(), objCtrlD.Method());
    // 类型错误
    else
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 15, __CURR_CODE_PLACE_C__, string(this->getHint("Controller_Type_Error_")) + act.GetSingleInfo("the_request") + " [Controllers." + objCtrlD.ActName() + "](" + sType + ") " + objCtrlD.LocalFile(), *this);
    // 执行善后
    string sComplete = sActNode + "." + c_RESTful_complete;
    if (pt.get_child_optional(sComplete))
    {
        ExecGroup(objCtrlD, sComplete);
        // 有错误，退出
        if (0 != objCtrlD.JsonRespond().GetVal(c_RESTful_errCode, 0))
            return;
    }
    // 根调用控制器（客户端直接调用的控制器）
    if (objCtrlD.IsRootRun())
    {
        // todo: 调用配置文件中配置的后置控制器（m_postpAct）
    }
    // 子控制器
    else
    {
    }
}
// 可供其他第三方模块调用
CKCJsonPackRespond::TCoreParmJsonRespondPtr CKCWebApiWork::SubCallCtrlApi(CCtrApilData& objCtrlD, string sUrl, string sActNode, string sName, bool upResult)
{
    CActionData &act = objCtrlD.Act();
    // 分解子项
    vector<string> urlVec;
    split(urlVec, sUrl, is_any_of("?"));
    // 子控制器完整本地文件名（默认与主调用控制器相同）
    string sSubFilePath = act.re().GetLocalFilename();
    // 子控制器url文件
    string sSubFileUrl = algorithm::trim_left_copy(urlVec[0]);
    if (!sSubFileUrl.empty())
    {
        sSubFilePath = CUtilFunc::PCharSafeToStr(objCtrlD.transItemUrlToFullPath(sSubFileUrl.c_str()));
        if (!boost::filesystem::exists(sSubFilePath))
            throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 16, __CURR_CODE_PLACE_C__, string(this->getHint("Don_t_exists_file_")) + sSubFileUrl, *this, sSubFilePath + "\n" + act.re().GetSingleInfo("the_request") + " [" + sName + ": " + sUrl + "]");
    }
    else sSubFileUrl = CUtilFunc::PCharSafeToStr(objCtrlD.Url());
    // else throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 17, __CURR_CODE_PLACE_C__, string(this->getHint("Not_Assigned_Controller_")) + "\r\n" + re.GetSingleInfo("the_request") + " [" + sName + ": " + sUrl + "]", *this);
    // 子控制器的get参数
    const string sUrlArg = urlVec.size() >= 2 ? urlVec[1] : "";
    // 子控制器api数据接口
    auto subCtrlDPtr = CSubCtrlApiData::CreatePtr(act, objCtrlD.selfRef(), sActNode + "." + sName, sSubFileUrl, sUrlArg, sSubFilePath);
    CCtrApilData &subObjCtrlD = *subCtrlDPtr;
    // 执行控制器api
    ExecCtrlApi(subObjCtrlD);
    // 执行结果
    return SubCallResult(objCtrlD, subObjCtrlD, sUrl, upResult);
}
CKCJsonPackRespond::TCoreParmJsonRespondPtr CKCWebApiWork::SubCallResult(CCtrApilData& objCtrlD, CCtrApilData& subObjCtrlD, string sFlag, bool upResult)
{
    CActionData &act = objCtrlD.Act();
    auto &resKernel = subObjCtrlD.JsonRes().Kernel();
    resKernel.SaveErrInfo();
    if (resKernel.Parm().IsJson())
    {
        const int errCode = resKernel.GetErrCode();
        string errMsg = resKernel.GetErrMsg();
        if (0 != errCode)
            this->WriteLogDebug((boost::format("⚠️ %d-%s\n%s ") % errCode % errMsg % sFlag).str().c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
        else
            this->WriteLogTrace((boost::format("✅ %d-%s\n%s ") % errCode % errMsg % sFlag).str().c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
    }
    // 只输出一个参数
    subObjCtrlD.JsonRes().OutParmSubItem(boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(subObjCtrlD.GetGetArg(c_RESTful_out))));
    // 移出
    auto coreMov = subObjCtrlD.MoveOutResUp();
    if (coreMov.get() == nullptr)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork_InvalidRespondRoot, __CURR_CODE_PLACE_C__, string(this->getHint("Null_Point_Data_")) + act.GetSingleInfo("the_request"), *this);
    // 子控制器api的结果，上传到父控制器api中的结果
    if (upResult) objCtrlD.MoveInRes(coreMov);
    // 子控制器与父控制器交换内部数据集
    objCtrlD.SwapInnerDbSet(subObjCtrlD);
    // 返回
    return coreMov;
}

// 持久对象
IActionData::TAliveObj& CKCWebApiWork::GetAliveObj(string sName)
{
    // 读取
    TKCAliveObj *pAliveObj = GetAliveObjNotCreate(sName);
    // 创建
    if (nullptr == pAliveObj)
    {
        // 创建持久对象及智能指针和弱指针
        pAliveObj = new TKCAliveObj(*this, sName);
        boost::shared_ptr<TKCAliveObj> ptr(pAliveObj);
        boost::weak_ptr<TKCAliveObj> wptr(ptr);
        // 插入弱指针
        {
            boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtxMap);
            auto it = m_aliveObjMap.find(sName);
            if (m_aliveObjMap.end() != it) it->second = wptr;
            else
            {
                boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                m_aliveObjMap.insert(make_pair(sName, wptr));
            }
        }
        // 插入智能指针
        boost::thread thrdInsetPtr([=](){
            boost::unique_lock<boost::shared_mutex> lck(this->m_mtxList);
            this->m_aliveObjList.push_back(ptr);
        });
        // 日志
        WriteLogDebug("Create Alive Object", __CURR_CODE_PLACE_C__, sName.c_str());
    }
    return *pAliveObj;
}
CKCWebApiWork::TKCAliveObj* CKCWebApiWork::GetAliveObjNotCreate(string sName)
{
    TKCAliveObj *pAliveObj = nullptr;
    boost::shared_lock<boost::shared_mutex> lck(m_mtxMap);
    auto it = m_aliveObjMap.find(sName);
    if (m_aliveObjMap.end() != it && !it->second.expired())
    {
        pAliveObj = it->second.lock().get();
        if (nullptr != pAliveObj) pAliveObj->m_ptime = posix_time::microsec_clock::local_time();
    }
    return pAliveObj;
}

// 守护线程（定时垃圾回收等）
void CKCWebApiWork::GuardThrd(void)
{
    posix_time::ptime ptLastAliveList = posix_time::microsec_clock::local_time();
    posix_time::ptime ptLastAliveMap = posix_time::microsec_clock::local_time();
    posix_time::ptime ptLastTaskExec = posix_time::microsec_clock::local_time();
    int keepAliveTimeout = m_keepAliveTimeout;
    WriteLogDebug("Begin Guard Thread.", __CURR_CODE_PLACE_C__, (boost::format("[PID = %d, TaskExecInterval = %d]") % boost::this_process::get_id() % m_TaskExecInterval).str().c_str());
    while (!m_guardThrdEnd)
    {
        boost::this_thread::interruption_point();
        boost::this_thread::sleep(boost::posix_time::milliseconds(888));
        posix_time::ptime ptNow = posix_time::microsec_clock::local_time();
        // 执行定时任务
        if (m_TaskExecInterval > 0 && (ptNow - ptLastTaskExec).total_seconds() > m_TaskExecInterval)
        {
            ptLastTaskExec = ptNow;
            ExecScheduledTask();
        }
        // 清除超时的持久对象
        if ((ptNow - ptLastAliveList).total_seconds() > keepAliveTimeout)
        {
            ptLastAliveList = ptNow;
            try
            {
                string sInfo = "";
                // 循环
                {
                    boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtxList);
                    for(auto it = m_aliveObjList.begin(); m_aliveObjList.end() != it;)
                        if ((ptNow - (*it)->m_ptime).total_seconds() > keepAliveTimeout)
                        {
                            sInfo += (*it)->m_aliveID + ";";
                            boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                            it = m_aliveObjList.erase(it);
                        }
                        else ++it;
                }
                // 日志
                if (!sInfo.empty())
                    WriteLogDebug("Release Alive Object", __CURR_CODE_PLACE_C__, sInfo.c_str());
            }
            catch (...) {}
        }
        // 清除无效的弱指针
        if (ptNow.date().day() != ptLastAliveMap.date().day())
        {
            ptLastAliveMap = ptNow;
            try
            {
                boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtxMap);
                for(auto it = m_aliveObjMap.begin(); m_aliveObjMap.end() != it;)
                    if (it->second.expired())
                    {
                        boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                        it = m_aliveObjMap.erase(it);
                    }
                    else ++it;
            }
            catch (...) {}
        }
    }
    WriteLogDebug("End Guard Thread.", __CURR_CODE_PLACE_C__, (boost::format("[PID = %d, TaskExecInterval = %d]") % boost::this_process::get_id() % m_TaskExecInterval).str().c_str());
}
// 执行定时任务
void CKCWebApiWork::ExecScheduledTask(void)
{
    int iID = -1;
    string sJson, sReq, sPar;
    auto fExecMsg = [&](string stt, string msg, string msg2 = "")
    {
        return iID < 0  ? (boost::format("<Scheduled Task> %s.\n\n%s\n%s\n%s\n") % stt % sJson % msg % msg2).str()
                        : (boost::format("<Scheduled Task> %s.\nid = %d\nreq = %s\npar = %s\n\n%s\n%s\n%s\n") % stt % iID % sReq % sPar % sJson % msg % msg2).str();
    };
    try
    {
        // 定时任务标记，全局session
        if (!m_TaskExecGet.empty() && !m_TaskExecFinish.empty() && m_TaskExecManaFlag.substr(0, sizeof(c_RESTful_KCSession) - 1) == c_RESTful_KCSession)
        {
            auto &sess = Session();
            const unsigned iSrvID = getContext().GetSrvID();
            const int iProcID = this_process::get_id();
            string sSessionName = m_TaskExecManaFlag.substr(sizeof(c_RESTful_KCSession) - 1);
            // 设置独占标记
            auto fSetAlone = [&](void) -> string
            {
                string sFlag = (format("%d-%d-%d") % iSrvID % iProcID % CUtilFunc::GetCurrentStampS()).str();
                sess.SetSessionVal(sSessionName.c_str(), sFlag.c_str());
                return sFlag;
            };
            // 判断是否有权限独占执行
            auto fCheckAlone = [&](void) -> bool
            {
                string sFlag = boost::trim_copy(CUtilFunc::PCharSafeToStr(sess.GetSessionVal(sSessionName.c_str())));
                // 独占标记为空时，设置独占标记
                if (sFlag.empty()) return true;
                // 解析独占标记
                std::vector<std::string> flagVec;
                boost::algorithm::split(flagVec, sFlag, boost::is_any_of("-"));
                // 独占标记格式错误时，设置独占标记
                if (flagVec.size() != 3) return true;
                try
                {
                    auto ptm = CUtilFunc::GetPTimeByStampS(boost::lexical_cast<long long>(flagVec[2]));
                    // 独占时间超过6分钟未刷新时，抢夺独占标记
                    if ((boost::posix_time::second_clock::local_time() - ptm).total_seconds() > m_TaskExecInterval * 6) return true;
                    // 判断是否本进程的独占标记
                    return iSrvID == boost::lexical_cast<unsigned>(flagVec[0]) && iProcID == boost::lexical_cast<int>(flagVec[1]);
                }
                catch (...)
                {
                    // 独占标记格式错误时，设置独占标记
                    return true;
                }
            };
            // 执行定时任务（由一个进程独占执行）
            auto fExecTasks = [&](void)
            {
                string sFlag = fSetAlone();
                // 控制器调用参数
                string sCookie = (boost::format("KCCLNID=%1%; KCSSID=%1%") % c_RESTful_KCGlobalKCSSID).str();
                string sParm = (boost::format(R"({"SrvID":"%d", "ProcID":"%d", "Flag":"%s"})") % iSrvID % iProcID % sFlag).str();
                // 读取符合执行的任务列表
                sJson = CCltrFunc::CallCtrl(*this, m_TaskExecGet, sCookie, sParm);
                cout << sJson << endl;
                this->WriteLogDebug(("<Scheduled Task> Begin Execution - " + sFlag).c_str(), __CURR_CODE_PLACE_C__);
                //  解析返回值
                cJSON *pJson = cJSON_Parse(sJson.c_str());
                if (nullptr != pJson)
                {
                    // 获取json整数值
                    auto fGetJsonInt = [&](cJSON *pJson, string sName) -> int
                    {
                        cJSON *pSubJson = cJSON_GetObjectItem(pJson, sName.c_str());
                        if (nullptr != pSubJson && !cJSON_IsNull(pSubJson))
                            return static_cast<int>(cJSON_GetNumberValue(pSubJson));
                        return 0;
                    };
                    // 获取json字符串值
                    auto fGetJsonStr = [&](cJSON *pJson, string sName) -> string
                    {
                        cJSON *pSubJson = cJSON_GetObjectItem(pJson, sName.c_str());
                        if (nullptr != pSubJson && !cJSON_IsNull(pSubJson))
                            return cJSON_GetStringValue(pSubJson);
                        return "";
                    };
                    // 获取执行队列
                    pJson = cJSON_GetObjectItem(pJson, m_defaultValsName.c_str());
                    if (nullptr != pJson)
                    {
                        unsigned iTskCnt = cJSON_GetArraySize(pJson);
                        if (iTskCnt > 0)
                            this->WriteLogDebug((boost::format("Run Task ~=> [Count = %d]") % iTskCnt).str().c_str(), __CURR_CODE_PLACE_C__);
                        for (unsigned i = 0; i < iTskCnt; ++i)
                        {
                            try
                            {
                                // 获取任务控制器
                                cJSON *pJsonCtrl = cJSON_GetArrayItem(pJson, i);
                                iID = fGetJsonInt(pJsonCtrl, "id");
                                sReq = fGetJsonStr(pJsonCtrl, "req");
                                sPar = fGetJsonStr(pJsonCtrl, "par");
                                if (!sReq.empty() && iID > 0)
                                {
                                    // 执行任务控制器
                                    sJson = CCltrFunc::CallCtrl(*this, sReq, sCookie, sPar);
                                    sFlag = fSetAlone();
                                    cout << iID << " - " << sReq << endl << sJson << endl;
                                    this->WriteLogDebug(fExecMsg("Execution Success", sFlag).c_str(), __CURR_CODE_PLACE_C__);
                                    // 设置任务执行完成
                                    sJson = CCltrFunc::CallCtrl(*this, m_TaskExecFinish, sCookie, (boost::format(R"({"id":%d,"%s":%s})") % iID % c_RESTful_inParm % sJson).str());
                                    cout << m_TaskExecFinish << endl << sJson << endl;
                                }
                            }
                            catch (std::exception &ex)
                            {
                                cout << ex.what() << endl << sJson << endl;
                                this->WriteLogFatal(fExecMsg("Execution Failed", ex.what(), cJSON_GetErrorPtr()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                            }
                            catch (...)
                            {
                                cout << "Unknown Error" << endl << sJson << endl;
                                this->WriteLogFatal(fExecMsg("Execution Failed", this->getHint("Unknown_exception"), cJSON_GetErrorPtr()).c_str(), __CURR_CODE_PLACE_C__);
                            }
                            iID = -1;
                            sReq = sPar = "";
                        }
                    }
                }
            };
            // 判断权限，并执行
            if (fCheckAlone()) fExecTasks();
        }
    }
    catch(TException& ex)
    {
        this->WriteLogFatal(fExecMsg("Execution Failed", ex.error_info()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch(std::exception& ex)
    {
        this->WriteLogFatal(fExecMsg("Execution Failed", ex.what()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        this->WriteLogFatal(fExecMsg("Execution Failed", this->getHint("Unknown_exception")).c_str(), __CURR_CODE_PLACE_C__);
    }
}

// 检查是否需要登录
void CKCWebApiWork::CheckToken(CRootCtrlApiData &objCtrlD, const property_tree::ptree& /*pt*/, string /*sNodeToken*/, string sActNode)
{
    auto &act = objCtrlD.Act();
    // 需要验证登录
    if (objCtrlD.NeedToken())
    {
        // 得到登录标识
        string sToken = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(act.GetGlobalVal(m_NeedToken.c_str())));
        // 未登录
        if (sToken.empty())
            throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork, __CURR_CODE_PLACE_C__, this->getHint("No_Login"), *this);
        // 其他权限（执行检查权限的控制器）
        if (!m_checkTokenAct.empty())
        {
            // 执行控制器
            auto resCore = SubCallCtrlApi(objCtrlD, m_checkTokenAct, sActNode, "checkToken", false);
            if (resCore.get() == nullptr)
                throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 17, __CURR_CODE_PLACE_C__, string(this->getHint("Null_Point_Data_")) + act.re().GetSingleInfo("the_request") + " [checkToken - " + m_checkTokenAct + "]", *this);
            if (!resCore->Parm().IsJson())
                throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 17, __CURR_CODE_PLACE_C__, string(this->getHint("Request_Failed_")) + act.re().GetSingleInfo("the_request") + " [checkToken - " + m_checkTokenAct + "]", *this);
            int errCode = resCore->GetErrCode();
            string errMsg = resCore->GetErrMsg();
            // 有错误则退出
            if (0 != errCode)
            {
                if (errMsg.empty()) errMsg = string(this->getHint("No_Access_")) + act.GetSingleInfo(c_RESTful_RootActUri);
                throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 18, __CURR_CODE_PLACE_C__, errMsg, *this);
            }
        }
    }
}

// json是否区分大小写
bool CKCWebApiWork::JsonCaseSensitive(const property_tree::ptree& pt, string sJsonCaseSensitive, bool bDef)
{
    bool bResult = bDef;
    auto fSetBool = [&](string sVal)
    {
        algorithm::to_lower(sVal);
        if ("true" == sVal || "false" == sVal)
            bResult = "true" == sVal;
    };
    // 整个文件是否配置为区分大小写
    string sRootCaseSS = string(c_RESTful_Controllers) + ".<xmlattr>." + c_RESTful_jsonCaseSensitive;
    if (pt.get_child_optional(sRootCaseSS)) fSetBool(pt.get<string>(sRootCaseSS));
    // 控制器是否配置为区分大小写
    if (pt.get_child_optional(sJsonCaseSensitive)) fSetBool(pt.get<string>(sJsonCaseSensitive));
    return bResult;
}

// 执行组操作
void CKCWebApiWork::ExecGroup(CCtrApilData& objCtrlD, string sActNode, string sMethod)
{
    const property_tree::ptree &pt = objCtrlD.Pt();
    // 处理组操作
    auto fGrpWork = [&](void) -> bool
    {
        // 循环每个子项
        for(const property_tree::ptree::value_type &vt: pt.get_child(sActNode))
            if (!ExecGroupSub(objCtrlD, vt, sActNode))
                return false;
        return true;
    };
    // 批量处理
    if (c_RESTful_batch == sMethod)
    {
        // 附加参数
        const int iBatchMethod = static_cast<int>(objCtrlD.JsonAttach().GetItem(c_RESTful_batchParm).GetVal(c_RESTful_batchMethod, 1));
        string sBatchParm = objCtrlD.JsonAttach().GetItem(c_RESTful_batchParm).GetStr(c_RESTful_batchValsName, "");
        string sSrcDbSetName = !sBatchParm.empty() ? sBatchParm : objCtrlD.ActionData().GetValsName();

        // 从内部数据集里取
        CSqlFunc::TRecordSetPtr lastInnerDbSet = objCtrlD.DataSetInnerLast(sSrcDbSetName);
        if (lastInnerDbSet.get() != nullptr)
        {
            // 字段
            CSqlFunc::TFields vecFields;
            // 循环内部数据集的每行数据
            lastInnerDbSet->DataSetToOut(vecFields,
                [&](){
                }, [&](int, int) -> bool {
                    // 行数据生成json
                    CCtrApilDataBase::TBatchGrpInDbSetTmpJsonPtr batchGrpInDbSetTmpJson(new CCtrApilDataBase::TBatchGrpInDbSetTmpJson(objCtrlD.Act()));
                    objCtrlD.m_batchGrpInDbSetTmpJson = batchGrpInDbSetTmpJson;
                    auto &tmpJson = *batchGrpInDbSetTmpJson->m_json.get();
                    // 添加行记录
                    for (auto &fFeild : vecFields)
                    {
                        CSqlFunc::TKcValVariant val = lastInnerDbSet->GetVal(*fFeild);
                        if (val.empty())
                            tmpJson.AddNull(fFeild->m_name.c_str());
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<int>(val))
                            tmpJson.AddVal(fFeild->m_name.c_str(), *ptr);
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<long long>(val))
                            tmpJson.AddVal(fFeild->m_name.c_str(), static_cast<double>(*ptr));
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<double>(val))
                            tmpJson.AddVal(fFeild->m_name.c_str(), *ptr);
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<string>(val))
                            tmpJson.AddStr(fFeild->m_name.c_str(), ptr->c_str());
                        else
                            tmpJson.AddStr(fFeild->m_name.c_str(), BoostAnyToStdString(val).c_str());
                    }
                    // 执行
                    bool isSucc = fGrpWork();
                    return isSucc || 1 == iBatchMethod;
                });
        }
        // 从json批量数组参数里取
        else
        {
            // json批量数组
            const CKCJsonPack& arrBatchPrm = objCtrlD.GetBatchArrayJsonImpl(sBatchParm);
            // 循环每个数组元素
            for (unsigned i = 0, c = arrBatchPrm.GetItemCount(); i < c; ++i)
            {
                // 本条数据
                auto ptrSubItem = const_cast<CKCJsonPack&>(arrBatchPrm).ImplGetItem(i);
                objCtrlD.m_batchGrpInTmpPrm = ptrSubItem;
                // objCtrlD.m_batchGrpInTmpPrm = const_cast<CKCJsonPack&>(arrBatchPrm).ImplGetItem(i);
                CAutoRelease _auto([&](){ objCtrlD.m_batchGrpInTmpPrm.reset(); });
                // 执行
                bool isSucc = fGrpWork();
                if (!isSucc && 1 != iBatchMethod) break;
            }
        }
    }
    // 单次处理
    else fGrpWork();
}
bool CKCWebApiWork::ExecGroupSub(CCtrApilData& objCtrlD, property_tree::ptree::value_type vt, string sActNode)
{
    CActionData &act = objCtrlD.Act();
    string sAttr = objCtrlD.NodeName() + string(".") + c_RESTful_xmlattr + ".";
    string sExec = c_RESTful_order;
    const property_tree::ptree &pt = objCtrlD.Pt();
    if (pt.get_child_optional(sAttr + c_RESTful_exec)) sExec = pt.get<string>(sAttr + c_RESTful_exec);
    const string sSubActName = vt.first;
    if ("<xmlattr>" != sSubActName && vt.second.get_child_optional(string(c_RESTful_xmlattr) + "." + c_RESTful_url))
    {
        // 分支控制器链接
        string sUrl = vt.second.get<string>(string(c_RESTful_xmlattr) + "." + c_RESTful_url);
        // 顺序结构
        if (c_RESTful_order == sExec)
        {
            // 条件判断
            auto fCheckCondition = [&](void)
            {
                // 执行子控制器
                const auto resCore = SubCallCtrlApi(objCtrlD, sUrl, sActNode, sSubActName, false);
                if (resCore.get() == nullptr)
                    throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 17, __CURR_CODE_PLACE_C__, string(this->getHint("Null_Point_Data_")) + act.re().GetSingleInfo("the_request") + " [" + sSubActName + " - " + sUrl + "]", *this);
                if (!resCore->Parm().IsJson())
                    throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 17, __CURR_CODE_PLACE_C__, string(this->getHint("Request_Failed_")) + act.re().GetSingleInfo("the_request") + " [" + sSubActName + " - " + sUrl + "]", *this);
                return 0 == resCore->GetErrCode();
            };
            // 条件分支
            if (c_RESTful_order_if == sSubActName)
            {
                // “成功”：执行then分支
                if (fCheckCondition())
                {
                    if (vt.second.get_child_optional(c_RESTful_order_then))
                        for(property_tree::ptree::value_type &vtThen: vt.second.get_child(c_RESTful_order_then))
                            if (!ExecGroupSub(objCtrlD, vtThen, sActNode))
                                return false;
                }
                // “失败”：执行else分支
                else
                {
                    if (vt.second.get_child_optional(c_RESTful_order_else))
                        for(property_tree::ptree::value_type &vtElse: vt.second.get_child(c_RESTful_order_else))
                            if (!ExecGroupSub(objCtrlD, vtElse, sActNode))
                                return false;
                }
            }
            // 条件循环
            else if (c_RESTful_order_while == sSubActName)
            {
                while (fCheckCondition())
                    // “成功”：开始执行循环体
                    for(property_tree::ptree::value_type &vtWhile: vt.second)
                        if (!ExecGroupSub(objCtrlD, vtWhile, sActNode))
                            return false;
            }
            // todo: 跳转
            else if (c_RESTful_goTo == sSubActName)
            {
            }
            // todo: 执行内置函数
            else if (c_RESTful_order_exec == sSubActName)
            {
            }
            // 子控制器
            else
            {
                // 执行控制器
                const auto resCore = SubCallCtrlApi(objCtrlD, sUrl, sActNode, sSubActName);
                // 忽略错误选项
                bool ignoreError = false;
                if (vt.second.get_child_optional(string("<xmlattr>.") + c_RESTful_ignoreError))
                    ignoreError = boost::algorithm::to_lower_copy(vt.second.get<string>(string("<xmlattr>.") + c_RESTful_ignoreError)) == "true";
                // 有错误且未忽略，则退出
                if (!ignoreError && (resCore.get() == nullptr || (resCore->Parm().IsJson() && 0 != resCore->GetErrCode())))
                {
                    this->WriteLogDebug((boost::format("🔚 *** Group️ Exec Break ***: %s [%s.%s]") % sUrl % sActNode % sSubActName).str().c_str(), __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request"));
                    return false;
                }
            }
        }
        // 分支结构
        else if (c_RESTful_branch == sExec)
        {
            string sGrpBranchCtrl = act.GetGlobalVal((string(c_RESTful_KCAct) + c_RESTful_grpBranchCtrl).c_str());
            // 分支筛选条件
            if (sGrpBranchCtrl == sSubActName)
            {
                // 执行控制器
                SubCallCtrlApi(objCtrlD, sUrl, sActNode, sSubActName);
                return false;
            }
        }
        // // 并列结构（废弃，导致kc_chai_script插件的this指针不确定，多个并行子控制器返回的结果和内部数据集冲突）
        // else if (c_RESTful_parallel == sExec)
        // {
        //     // todo：启动多个线程，同时执行各子控制器，子控制器结果不上传
        // }
        // 类型错误
        else
        {
            this->WriteLogError((string(this->getHint("Controller_Type_Error_")) + act.GetSingleInfo("the_request") + " [Controllers." + CUtilFunc::PCharSafeToStr(objCtrlD.ActName()) + "." + vt.first + "](" + sExec + ") " + CUtilFunc::PCharSafeToStr(objCtrlD.LocalFile())).c_str(), __CURR_CODE_PLACE_C__);
            throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 20, __CURR_CODE_PLACE_C__, string(this->getHint("Controller_Type_Error_")) + act.GetSingleInfo("the_request") + " [Controllers." + CUtilFunc::PCharSafeToStr(objCtrlD.ActName()) + "." + vt.first + "](" + sExec + ") " + CUtilFunc::PCharSafeToStr(objCtrlD.Url()), *this);
        }
    }
    return true;
}

// 执行插件
void CKCWebApiWork::ExecBundle(CCtrApilData& objCtrlD)
{
    ExecBundleImpl(objCtrlD);
}
void CKCWebApiWork::ExecBundleImpl(CCtrApilData& objCtrlD)
{
    CActionData& act = objCtrlD.Act();
    string sSrvAlias = CUtilFunc::PCharSafeToStr(objCtrlD.Srv());
    string sCtrlName = CUtilFunc::PCharSafeToStr(objCtrlD.CtrlName());
    string sAct = CUtilFunc::PCharSafeToStr(objCtrlD.ActName());
    string sKCFilePath = CUtilFunc::PCharSafeToStr(objCtrlD.LocalFile());
    // 服务
    string sSrv = m_context.GetSrvGUIDByAlias(sSrvAlias.c_str());
    IServiceReference *refCtrl = nullptr;
    auto itCtrl = m_refs.find(sSrv);
    if (m_refs.end() == itCtrl)
    {
        refCtrl = m_context.takeServiceReference(sSrv.c_str());
        m_refs.insert(make_pair(sSrv, refCtrl));
    }
    else refCtrl = itCtrl->second;
    if (nullptr == refCtrl)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 19, __CURR_CODE_PLACE_C__, string(this->getHint("The_service_don_t_exist_")) + act.GetSingleInfo("the_request") + " [Controllers." + sAct + "](" + sSrvAlias + ") " + sKCFilePath, *this);
    // 附加参数
    TAttachParmForCpp<CCtrApilData> attachArg(objCtrlD);
    // 控制器
    auto& iSrv = dynamic_cast<IServiceReferenceEx*>(refCtrl)->getServiceSafe<IKCCtrlWork>();
    if (!iSrv.hasCtrl(sCtrlName.c_str()))
    {
        this->WriteLogError((string(this->getHint("The_service_don_t_exist_")) + sCtrlName + "\n" + act.GetSingleInfo("the_request") + " [Controllers." + sAct + "](" + sSrvAlias + ") " + sKCFilePath).c_str(), __CURR_CODE_PLACE_C__);
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 20, __CURR_CODE_PLACE_C__, string(this->getHint("The_service_don_t_exist_")) + sCtrlName + "\n" + act.GetSingleInfo("the_request") + " [Controllers." + sAct + "](" + sSrvAlias + ")", *this);
    }
    IKCController& wbCtrl = iSrv.getCtrl(sCtrlName.c_str());
    // 🚀 执行
    wbCtrl.Perform(objCtrlD, attachArg);
    // 执行结果
    objCtrlD.SetPerformResult();
}

// 执行动态库
void CKCWebApiWork::ExecSo(CCtrApilData& objCtrlD)
{
    ExecSoImpl(objCtrlD);
}
void CKCWebApiWork::ExecSoImpl(CCtrApilData& objCtrlD)
{
    CActionData& act = objCtrlD.Act();
    string sAct = CUtilFunc::PCharSafeToStr(objCtrlD.ActName());
    string sKCFilePath = CUtilFunc::PCharSafeToStr(objCtrlD.LocalFile());
    string sSoFileUri = objCtrlD.SoFile();
    // 本地完整路径
    string sSoFile = act.GetLocalFullPath(sSoFileUri.c_str());
    if (!boost::filesystem::exists(sSoFile))
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 22, __CURR_CODE_PLACE_C__, this->getHint("Don_t_exists_file_") + objCtrlD.GetXmlattrStr(c_RESTful_file), *this, sSoFile + "\n" + act.GetSingleInfo("the_request") + " [Controllers." + objCtrlD.ActName() + "] " + sKCFilePath);
    // 加载动态库
    TSoCtrl *pSoCtrl = nullptr;
    auto it = m_dlls.find(sSoFile);
    if (m_dlls.end() != it) pSoCtrl = it->second.get();
    else
    {
        pSoCtrl = new TSoCtrl(*this);
        std::shared_ptr<TSoCtrl> SoCtrlPtr(pSoCtrl);
        pSoCtrl->m_ctrl = SoInitActor(pSoCtrl->m_lib, sSoFile, sSoFileUri);
        m_dlls.insert(make_pair(sSoFile, SoCtrlPtr));
    }
    // 附加参数
    TAttachParmForCpp<CCtrApilData> attachArg(objCtrlD);
    // 🚀 执行控制器
    if (nullptr == pSoCtrl || nullptr == pSoCtrl->m_ctrl)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 23, __CURR_CODE_PLACE_C__, string(this->getHint("Null_Point_Data_")) + sSoFile, *this);
    pSoCtrl->m_ctrl->Perform(objCtrlD, attachArg);
    // 执行结果
    objCtrlD.SetPerformResult();
}
// 动态库的控制器
IKCController* CKCWebApiWork::SoInitActor(dll::shared_library& lib, string sSoFile, string sSoFileUri)
{
    // 加载动态库
    if (!lib.is_loaded())
    {
        system::error_code ec;
        lib.load(sSoFile, ec, dll::load_mode::rtld_now);
        if (ec)
            throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 24, __CURR_CODE_PLACE_C__, string(this->getHint("Load_library_failure_")) + sSoFileUri, *this, sSoFile + "\n" + ec.message());
    }
    // 初始化函数
    if (!lib.has("InitActor"))
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 25, __CURR_CODE_PLACE_C__, string(this->getHint("Can_t_get_the_function_")) + sSoFileUri + "::InitActor", *this, sSoFile);
    auto _Init = lib.get<IKCController&(const char*, const char*)>("InitActor");
    if (nullptr == _Init)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 26, __CURR_CODE_PLACE_C__, string(this->getHint("Can_t_get_the_function_")) + sSoFileUri + "::InitActor", *this, sSoFile);
    // 执行函数
    boost::filesystem::path _path(sSoFile);
    string sDir = _path.parent_path().string();
    sDir = CUtilFunc::FormatPath(sDir);
    return &_Init(sDir.c_str(), sSoFileUri.c_str());
}
void CKCWebApiWork::SoUninitActor(dll::shared_library& lib, IKCController*& ctrl)
{
    if (lib.is_loaded())
    {
        // 卸载函数
        if (lib.has("UninitActor"))
        {
            auto _Uninit = lib.get<void(IKCController&)>("UninitActor");
            _Uninit(*ctrl);
        }
        ctrl = nullptr;
        // 卸载动态库
        lib.unload();
    }
}
