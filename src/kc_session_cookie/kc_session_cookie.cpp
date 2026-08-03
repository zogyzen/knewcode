#include "kc_session_cookie.h"
#include "ctrl_session.h"
#include "ctrl_local_ctrl.h"

////////////////////////////////////////////////////////////////////////////////
// CKCSessionCookie类
CKCSessionCookie::CKCSessionCookie(const IBundle& bundle)
    : TKCCtrlWork<IKCSessionCookie>(bundle), m_shareMemNameBase([&](){
        // unsigned iSrvID = m_context.GetSrvID();
        // string sSysCust = CUtilFunc::PCharSafeToStr(m_context.GetSysFlag("customer", "MY"));
        // string sFlag = CUtilFunc::PCharSafeToStr(m_context.GetCfgInfo(CCtrlCommon::GetCtrlNodeCtrlName(*this).c_str(), "Flag", ""));
        // string sOwnName = CUtilFunc::PCharSafeToStr(m_context.GetMain().OwnName());
        // string sBuildDt = CUtilFunc::PCharSafeToStr(m_context.BuildDatetime());
        // boost::algorithm::replace_all(sBuildDt, ".", "");
        // return (format("KC_%d_%s_%s_%s_%s_SharedMem") % iSrvID % sOwnName % sSysCust % sFlag % sBuildDt).str();
        string sSysCust = CUtilFunc::PCharSafeToStr(m_context.GetSysFlag("customer", "MY"));
        string sFlag = CUtilFunc::PCharSafeToStr(m_context.GetCfgInfo(CCtrlCommon::GetCtrlNodeCtrlsName(*this).c_str(), "Flag", ""));
        return (format("%s-%s") % sSysCust % sFlag).str();
    }())
    // , m_mtxShareMem(boost::interprocess::open_or_create, m_shareMemNameBase.c_str())
    , m_segment(CreateOrOpenShareMem()), m_mapfile(CreateOrOpenMapFile())
{
    if (m_segment.get() == nullptr || m_mapfile.get() == nullptr)
        cout << "X";
}

// 初始化控制器
void CKCSessionCookie::initAllCtrl(void)
{
    CCtrlCommon::GetAllCtrl(*this, m_pt, m_context.GetCfgFile(), m_ctrls, [&](string sName, property_tree::ptree& v) -> ICtrlSession*
    {
        // 存储类型
        string sCCType = "shared_memory";
        if (v.get_child_optional("<xmlattr>.cctype"))
            sCCType = algorithm::to_lower_copy(v.get<string>("<xmlattr>.cctype"));
        // 按存储类型构建
        if ("mapped_file" == sCCType)           // 映射文件
            return m_mapfile.get() == nullptr ? nullptr : new CCtrlSession<interprocess::managed_mapped_file>(*this, sName, v, false, *m_mapfile);
        else if ("local_ctrl" == sCCType)       // 本地控制器
            return new CCtrlLocalCtrl(*this, sName, v);
        else                                    // 共享内存
            return m_segment.get() != nullptr ? nullptr : new CCtrlSession<interprocess::managed_shared_memory>(*this, sName, v, true, *m_segment);
    });
}

// 创建共享内存
CKCSessionCookie::managed_shared_memory_ptr CKCSessionCookie::CreateOrOpenShareMem(void)
{
    // 返回值
    managed_shared_memory_ptr resPtr;
    // 共享内存大小
    unsigned iShareMemSize = c_KCSessionShareMemSize;
    try
    {
        iShareMemSize = lexical_cast<unsigned>(string(m_context.GetCfgInfo(CCtrlCommon::GetCtrlNodeCtrlsName(*this).c_str(), "ShareMemSize", std::to_string(iShareMemSize / 1024 / 1024).c_str()))) * 1024 * 1024;
    }
    catch (...) {}
    if (iShareMemSize > 0)
    {
        const int ciShareMemSize = iShareMemSize;
        // 共享内存名
        string sShareMemNameInfo = m_shareMemNameBase;
        // 新建或打开
        for (int i = 0; i < 2; ++i, iShareMemSize = ciShareMemSize)
            while (resPtr.get() == nullptr && iShareMemSize >= 1024 * 1024)
                try
                {
                    sShareMemNameInfo = (format("%s-%dM") % m_shareMemNameBase % (iShareMemSize / 1024 / 1024)).str();
                    this->WriteLogInfo(sShareMemNameInfo.c_str(), __CURR_CODE_PLACE_C__);
                    if (!CUtilFunc::TimeoutRun([&](){
                            // boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(m_mtxShareMem);
                            try
                            {
                                // 打开共享内存
                                resPtr.reset(new boost::interprocess::managed_shared_memory(boost::interprocess::open_or_create, sShareMemNameInfo.c_str(), iShareMemSize));
                            }
                            catch (const boost::interprocess::interprocess_exception &ex)
                            {
                                // 打不开，先删除，再创建
                                this->WriteLogWarning((sShareMemNameInfo + "\n" + CUtilFunc::GbkToUtf8(ex.what()) + "\n" + ex.what()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                                try
                                {
                                    interprocess::shared_memory_object::remove(m_shareMemNameBase.c_str());
                                }
                                catch (...) {}
                                // resPtr.reset(new interprocess::managed_shared_memory(interprocess::open_or_create, sShareMemNameInfo.c_str(), iShareMemSize));
                                resPtr.reset(new interprocess::managed_shared_memory(interprocess::create_only, sShareMemNameInfo.c_str(), iShareMemSize));
                            }
                        }, 6666))
                    {
                        // 如果超时，则删除共享内存
                        iShareMemSize /= 2;
                        this->WriteLogError(sShareMemNameInfo.c_str(), __CURR_CODE_PLACE_C__, "time out");
                        // boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(m_mtxShareMem);
                        interprocess::shared_memory_object::remove(m_shareMemNameBase.c_str());
                    }
                }
                catch (std::exception &ex)
                {
                    iShareMemSize /= 2;
                    this->WriteLogWarning((sShareMemNameInfo + "\n" + CUtilFunc::GbkToUtf8(ex.what()) + "\n" + ex.what()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                    // 如果小于1M，仍不能创建或打开，则删除共享内存
                    if (iShareMemSize < 1024 * 1024) interprocess::shared_memory_object::remove(m_shareMemNameBase.c_str());
                }
        // 返回
        // if (resPtr.get() == nullptr)
        //     throw std::runtime_error(this->getHint("Create_Fail_") + sShareMemNameInfo);
    }
    return resPtr;
}
// 创建映射文件
CKCSessionCookie::managed_mapped_file_ptr CKCSessionCookie::CreateOrOpenMapFile(void)
{
    // 返回值
    managed_mapped_file_ptr resPtr;
    // 映射文件大小
    unsigned iMapFileSize = c_KCSessionShareMemSize / 16;
    try
    {
        iMapFileSize = lexical_cast<unsigned>(string(m_context.GetCfgInfo(CCtrlCommon::GetCtrlNodeCtrlsName(*this).c_str(), "MapFileSize", std::to_string(iMapFileSize / 1024 / 1024).c_str()))) * 1024 * 1024;
    }
    catch (...) {}
    if (iMapFileSize > 0)
    {
        // 映射文件
        string sFile = m_bundle.getName() + string(".mapfile");
        string sFileFull = m_bundle.getPath() + string("/") + sFile;
        string sShareMemNameInfo = sFileFull;
        // 新建或打开（如果超时，则删除映射文件）
        for (int i = 0; resPtr.get() == nullptr && i < 3; ++i)
            try
            {
                sShareMemNameInfo = (format("%s (%dM)") % sFileFull % (iMapFileSize / 1024 / 1024)).str();
                this->WriteLogInfo(sShareMemNameInfo.c_str(), __CURR_CODE_PLACE_C__);
                if (!CUtilFunc::TimeoutRun([&](){
                        resPtr.reset(new interprocess::managed_mapped_file(interprocess::open_or_create, sFileFull.c_str(), iMapFileSize));
                    }, 6666))
                {
                    this->WriteLogError(sShareMemNameInfo.c_str(), __CURR_CODE_PLACE_C__, "time out");
                    interprocess::file_mapping::remove(sFileFull.c_str());
                }
            }
            catch (std::exception &ex)
            {
                this->WriteLogWarning((sShareMemNameInfo + "\n" + CUtilFunc::GbkToUtf8(ex.what()) + "\n" + ex.what()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
        // 返回
        // if (resPtr.get() == nullptr)
        // {
        //     //throw std::runtime_error(this->getHint("Create_Fail_") + sShareMemNameInfo);
        //     this->WriteLogError((this->getHint("Create_Fail_") + sShareMemNameInfo).c_str(), __CURR_CODE_PLACE_C__);
        // }
    }
    return resPtr;
}

// session值
bool CKCSessionCookie::IsSession(const char* pName) const
{
    if (nullptr != pName)
    {
        string sName(pName);
        sName = sName.substr(0, sName.find("__"));
        return hasCtrl(sName.c_str());
    }
    return false;
}
const char* CKCSessionCookie::GetSessionVal(const char* pName, IActionData& act)
{
    unsigned iLen = 0;
    return GetSessionVal(pName, iLen, act);
}
const char* CKCSessionCookie::GetSessionVal(const char* pName, unsigned& iLen, IActionData& act)
{
    const char* pResult = nullptr;
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        ICtrlSession *pCtrl = GetSSCtrl(sName, CUtilFunc::PCharSafeToStr(act.GetKCSSID()), CUtilFunc::PCharSafeToStr(act.GetKCCLNID()), sSession);
        if (nullptr != pCtrl)
        {
            static thread_local string sResult;
            sResult = pCtrl->GetSession(sSession, &act);
            if (!sResult.empty())
            {
                iLen = static_cast<unsigned>(sResult.size());
                pResult = sResult.c_str();
            }
        }
    });
    return pResult;
}
const char* CKCSessionCookie::GetSessionVal(const char* pName, const char* pKCSSID, const char* pKCCLNID)
{
    unsigned iLen = 0;
    return GetSessionVal(pName, iLen, pKCSSID, pKCCLNID);
}
const char* CKCSessionCookie::GetSessionVal(const char* pName, unsigned& iLen, const char* pKCSSID, const char* pKCCLNID)
{
    /*
    string sSession;
    auto fExceptInfo = [&](void)
    {
        string sName(nullptr != pName ? pName : "");
        return "\n" + sName + " - \t" + sSession;
    };
    try
    {
        ICtrlSession *pCtrl = GetSSCtrl(CUtilFunc::PCharSafeToStr(pName), CUtilFunc::PCharSafeToStr(pKCSSID), CUtilFunc::PCharSafeToStr(pKCCLNID), sSession);
        if (nullptr != pCtrl)
        {
            static thread_local string sResult;
            sResult = pCtrl->GetSession(sSession);
            if (!sResult.empty())
            {
                iLen = static_cast<unsigned>(sResult.size());
                return sResult.c_str();
            }
        }
    }
    catch (interprocess::interprocess_exception &ex)
    {
        string sErr = ex.what() + fExceptInfo();
        WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        WriteLog(ex);
        throw;
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
    catch (...)
    {
        string sErr = getHint("Unknown_exception") + fExceptInfo();
        WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        throw;
    }
    return nullptr;
    */

    const char* pResult = nullptr;
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        ICtrlSession *pCtrl = GetSSCtrl(sName, CUtilFunc::PCharSafeToStr(pKCSSID), CUtilFunc::PCharSafeToStr(pKCCLNID), sSession);
        if (nullptr != pCtrl)
        {
            static thread_local string sResult;
            sResult = pCtrl->GetSession(sSession);
            if (!sResult.empty())
            {
                iLen = static_cast<unsigned>(sResult.size());
                pResult = sResult.c_str();
            }
        }
    });
    return pResult;
}
void CKCSessionCookie::SetSessionVal(const char* pName, const char* pVal, IActionData& act)
{
    SetSessionVal(pName, pVal, strlen(pVal), act);
}
void CKCSessionCookie::SetSessionVal(const char* pName, const char* pVal, unsigned iLen, IActionData& act)
{
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        sVal = (nullptr != pVal && iLen > 0) ? string(pVal, iLen) : "";
        ICtrlSession *pCtrl = GetSSCtrl(sName, CUtilFunc::PCharSafeToStr(act.GetKCSSID()), CUtilFunc::PCharSafeToStr(act.GetKCCLNID()), sSession);
        if (nullptr != pCtrl)
        {
            if (!sVal.empty())
                pCtrl->SetSession(sSession, sVal, &act);
            else pCtrl->DelSession(sSession, &act);
        }
    });
}
void CKCSessionCookie::SetSessionVal(const char* pName, const char* pVal, const char* pKCSSID, const char* pKCCLNID)
{
    SetSessionVal(pName, pVal, strlen(pVal), pKCSSID, pKCCLNID);
}
void CKCSessionCookie::SetSessionVal(const char* pName, const char* pVal, unsigned iLen, const char* pKCSSID, const char* pKCCLNID)
{
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        sVal = (nullptr != pVal && iLen > 0) ? string(pVal, iLen) : "";
        ICtrlSession *pCtrl = GetSSCtrl(sName, CUtilFunc::PCharSafeToStr(pKCSSID), CUtilFunc::PCharSafeToStr(pKCCLNID), sSession);
        if (nullptr != pCtrl)
        {
            if (!sVal.empty())
                pCtrl->SetSession(sSession, sVal);
            else pCtrl->DelSession(sSession);
        }
    });
}

// 执行session操作
void CKCSessionCookie::TrySession(std::function<void(string&, string&, string&)> func)
{
    string sSession, sName, sVal;
    auto fExceptInfo = [&](void)
    {
        return "\n" + sName + "=" + sVal + " - \t" + sSession + "\n" + typeid(func).name();
    };
    try
    {
        func(sSession, sName, sVal);
    }
    catch (interprocess::interprocess_exception &ex)
    {
        string sErr = ex.what() + fExceptInfo();
        WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        WriteLog(ex);
        throw;
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
    catch (...)
    {
        string sErr = getHint("Unknown_exception") + fExceptInfo();
        WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        throw;
    }
}

// 得到session控制器
ICtrlSession* CKCSessionCookie::GetSSCtrl(string sName, string sKCSSID, string sKCCLNID, string &sSession)
{
    ICtrlSession *pCtrl = nullptr;
    if (!sName.empty())
    {
        unsigned iPos = static_cast<unsigned>(sName.find("__"));
        string sCtrl = sName.substr(0, iPos);
        if (hasCtrl(sCtrl.c_str()) && (pCtrl = dynamic_cast<ICtrlSession*>(&getCtrl(sCtrl.c_str()))) != nullptr)
            sSession = pCtrl->PrefixName(sKCSSID, sKCCLNID) + sName.substr(iPos + 2);
    }
    return pCtrl;
}

void CKCSessionCookie::SetSessionExpire(const char* pName, const char* pExpire, IActionData& act)
{
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        if (!sName.empty())
        {
            auto iPos = sName.find("__");
            if (string::npos != iPos)
            {
                string sCtrl = sName.substr(0, iPos);
                ICtrlSession *pCtrl = nullptr;
                if (hasCtrl(sCtrl.c_str()) && (pCtrl = dynamic_cast<ICtrlSession*>(&getCtrl(sCtrl.c_str()))) != nullptr)
                {
                    sSession = pCtrl->PrefixName(CUtilFunc::PCharSafeToStr(act.GetKCSSID()), CUtilFunc::PCharSafeToStr(act.GetKCCLNID())) + sName.substr(iPos + 2);
                    pCtrl->SetExpire(sSession, pExpire, &act);
                }
            }
        }
    });
}
void CKCSessionCookie::SetSessionExpire(const char* pName, const char* pExpire, const char* pKCSSID, const char* pKCCLNID)
{
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        if (!sName.empty())
        {
            auto iPos = sName.find("__");
            if (string::npos != iPos)
            {
                string sCtrl = sName.substr(0, iPos);
                ICtrlSession *pCtrl = nullptr;
                if (hasCtrl(sCtrl.c_str()) && (pCtrl = dynamic_cast<ICtrlSession*>(&getCtrl(sCtrl.c_str()))) != nullptr)
                {
                    sSession = pCtrl->PrefixName(CUtilFunc::PCharSafeToStr(pKCSSID), CUtilFunc::PCharSafeToStr(pKCCLNID)) + sName.substr(iPos + 2);
                    pCtrl->SetExpire(sSession, pExpire);
                }
            }
        }
    });
}

const char* CKCSessionCookie::GetSessionNextVal(const char* pName, const char* pBegin, const char* pKCSSID, const char* pKCCLNID)
{
    const char* pResult = nullptr;
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        sVal = CUtilFunc::PCharSafeToStr(pBegin);
        ICtrlSession *pCtrl = GetSSCtrl(sName, CUtilFunc::PCharSafeToStr(pKCSSID), CUtilFunc::PCharSafeToStr(pKCCLNID), sSession);
        if (nullptr != pCtrl)
        {
            static thread_local string sResult;
            sResult = pCtrl->GetSessionNextVal(sSession, sVal);
            pResult = sResult.c_str();
        }
    });
    return pResult;
}
const char* CKCSessionCookie::GetSessionNextVal(const char* pName, const char* pBegin, IActionData& act)
{
    const char* pResult = nullptr;
    TrySession([&](string& sName, string& sVal, string& sSession){
        sName = CUtilFunc::PCharSafeToStr(pName);
        sVal = CUtilFunc::PCharSafeToStr(pBegin);
        ICtrlSession *pCtrl = GetSSCtrl(sName, CUtilFunc::PCharSafeToStr(act.GetKCSSID()), CUtilFunc::PCharSafeToStr(act.GetKCCLNID()), sSession);
        if (nullptr != pCtrl)
        {
            static thread_local string sResult;
            sResult = pCtrl->GetSessionNextVal(sSession, sVal, &act);
            pResult = sResult.c_str();
        }
    });
    return pResult;
}
