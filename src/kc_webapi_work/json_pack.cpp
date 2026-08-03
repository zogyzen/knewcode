#include "json_pack.h"

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPackRoot::TCoreParmJsonRoot 类
CKCJsonPackRoot::TCoreParmJsonRoot::TCoreParmJsonRoot(string jsonLib, string json, string nameErrCode, string nameErrMsg, string charset, bool jsonCaseSensitive)
    : m_json(NewJsonCore(jsonLib, json))
    , m_parm(*new  CKCCtrlParmInOut(*this, nameErrCode, nameErrMsg, charset, jsonCaseSensitive))
{
}
CKCJsonPackRoot::TCoreParmJsonRoot::TCoreParmJsonRoot(IKCJsonCore::IJsonCorePtr json, string nameErrCode, string nameErrMsg, string charset, bool jsonCaseSensitive)
    : m_json(json)
    , m_parm(*new  CKCCtrlParmInOut(*this, nameErrCode, nameErrMsg, charset, jsonCaseSensitive))
{
}
CKCJsonPackRoot::TCoreParmJsonRoot::~TCoreParmJsonRoot(void)
{
    delete &m_parm;
    m_json.reset();
}

// 通过json字符串生成json
IKCJsonCore* CKCJsonPackRoot::TCoreParmJsonRoot::NewJsonCore(string jsonLib, string json)
{
    // 使用yyjson库
    if ("yyjson" == jsonLib) return new CKCcJsonRoot(json);
    // 默认使用cJSON库
    else return new CKCcJsonRoot(json);
}

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPackRoot 类
CKCJsonPackRoot::CKCJsonPackRoot(IJsonCallBack& cb) : CKCJsonPack(cb)
{
}

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPackAttach 类
CKCJsonPackAttach::CKCJsonPackAttach(IJsonCallBack& cb, string json)
    : CKCJsonPackRoot(cb), m_core(cb.JsonLibrary(), json, c_RESTful_errCode, c_RESTful_errMsg, cb.GetCharset(), cb.JsonCaseSensitive())
{
}

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPackRequest 类
CKCJsonPackRequest::CKCJsonPackRequest(IJsonCallBack& cb, string json)
    : CKCJsonPackRoot(cb), m_core(CKCJsonPackRoot::NewCore(cb, json))
{
}
CKCJsonPackRequest::CKCJsonPackRequest(IJsonCallBack& cb, const CKCJsonPackRequest& req)
    : CKCJsonPackRoot(cb), m_core(req.CloneOut())
{
}

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPackRespond 类
CKCJsonPackRespond::CKCJsonPackRespond(IJsonCallBack& cb, string json)
    : CKCJsonPackRoot(cb), m_core(CKCJsonPackRespond::NewCore(cb, json))
{
}
CKCJsonPackRespond::CKCJsonPackRespond(IJsonCallBack& cb, TCoreParmJsonRespondPtr core)
    : CKCJsonPackRoot(cb), m_core(core)
{
}
CKCJsonPackRespond::CKCJsonPackRespond(IJsonCallBack& cb, IKCJsonCore::IJsonCorePtr json)
    : CKCJsonPackRoot(cb), m_core(CKCJsonPackRespond::NewCore(cb, json))
{
}

// 用字符串重置
bool CKCJsonPackRespond::TCoreParmJsonRespond::ResetByStr(string jsonLib, string json)
{
    m_json.reset(NewJsonCore(jsonLib, json));
    return m_json->IsValid();
}
bool CKCJsonPackRespond::ResetByStr(const char* json)
{
    GC();
    return IsValid() ? m_core->ResetByStr(m_cb.JsonLibrary(), CUtilFunc::PCharSafeToStr(json)) : false;
}

// 用其他根项重置（只针对根）
bool CKCJsonPackRespond::ResetByRoot(const IKCJson& jsn)
{
    CKCJsonPackRespond* pRt = dynamic_cast<CKCJsonPackRespond*>(const_cast<IKCJson*>(&jsn));
    if (nullptr != pRt) return MoveIn(pRt->MoveOut());
    else return false;
}

// 移入移出（用于在控制器实例里转移核心参数和Json）
CKCJsonPackRespond::TCoreParmJsonRespondPtr CKCJsonPackRespond::MoveOut(void)
{
    GC();
    auto rem = m_core;
    m_core.reset(CKCJsonPackRespond::NewCore(m_cb));
    return rem;
}
bool CKCJsonPackRespond::MoveIn(TCoreParmJsonRespondPtr core)
{
    GC();
    m_core.reset();
    m_core = core;
    return IsValid();
}

// 只输出一个参数
bool CKCJsonPackRespond::OutParmSubItem(string sPath, string sSplit)
{
    bool bResult = !sPath.empty();
    if (bResult)
    {
        const auto &jsonPth = GetItem(sPath.c_str(), sSplit.c_str());
        if (jsonPth.IsValid())
        {
            const CKCJsonPackSub* subJsn = dynamic_cast<const CKCJsonPackSub*>(&jsonPth);
            if (nullptr != subJsn && subJsn->IsValid())
            {
                auto nwrt = subJsn->PromoteToRoot(true);
                if (nwrt.get() != nullptr && nwrt->IsValid()) bResult = MoveIn(nwrt->MoveOut());
            }
        }
    }
    return bResult;
}

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPackSub 类
CKCJsonPackSub::CKCJsonPackSub(IJsonCallBack& cb, IKCJsonCore::IJsonCorePtr core, const CKCJsonPack& own)
    : CKCJsonPack(cb), m_core(core, own.Core().Parm()), m_own(own)
{
}

// 子项提升为根项
CKCJsonPackRespondPtr CKCJsonPackSub::PromoteToRoot(bool delFromOwn) const
{
    return CKCJsonPackRespondPtr(PromoteToRootNeedDel(delFromOwn));
}
CKCJsonPackRespond* CKCJsonPackSub::PromoteToRootNeedDel(bool delFromOwn) const
{
    CKCJsonPackRespond* pRes = nullptr;
    if (IsValid())
    {
        if (delFromOwn)
        {
            pRes = new CKCJsonPackRespond(m_own.m_cb, const_cast<CKCJsonPack&>(m_own).Core().Json().Detach2Root(*m_core.m_json));
            const_cast<CKCJsonPackSub*>(this)->m_core.m_json.reset();
        }
        else pRes = new CKCJsonPackRespond(m_cb, Core().Json().Clone2Root());
    }
    return pRes;
}

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPackSub::TCoreParmJsonSub 类
CKCJsonPackSub::TCoreParmJsonSub::TCoreParmJsonSub(IKCJsonCore::IJsonCorePtr json, const IKCCtrlParmInOut& parm)
    : m_json(json), m_parm(parm)
{
}
CKCJsonPackSub::TCoreParmJsonSub::~TCoreParmJsonSub(void)
{
    m_json.reset();
}

////////////////////////////////////////////////////////////////////////////////
// CKCJsonPack 类
CKCJsonPack::CKCJsonPack(IJsonCallBack& cb) : m_cb(cb)
{
}
CKCJsonPack::~CKCJsonPack(void)
{
    GCImpl();
}

// 实现
CKCJsonPack::CKCJsonPackPtr CKCJsonPack::ImplGetItem(unsigned id)
{
    CKCJsonPackPtr ptr;
    if (this->IsValid() && id < this->GetItemCount())
    {
        auto it = m_mapSubItm_inArr.find(id);
        if (m_mapSubItm_inArr.end() != it) return it->second;
        IKCJsonCore::IJsonCorePtr core = Core().Json().GetItem(id);
        // core->AddItem("", true, m_cb.JsonCaseSensitive());
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_inArr.insert(make_pair(id, ptr));
            m_stepSub = ptr;
        }
    }
    return ptr;
}
CKCJsonPack::CKCJsonPackPtr CKCJsonPack::ImplGetItem(const char* name, bool caseSensitive)
{
    string sName = CUtilFunc::PCharSafeToStr(name);
    CKCJsonPackPtr ptr;
    if (this->IsValid() && this->HasItem(sName.c_str(), caseSensitive))
    {
        auto it = m_mapSubItm_obj.find(sName);
        if (m_mapSubItm_obj.end() != it) return it->second;
        IKCJsonCore::IJsonCorePtr core = Core().Json().GetItem(sName, caseSensitive);
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_obj.insert(make_pair(sName, ptr));
        }
    }
    return ptr;
}
CKCJsonPack::CKCJsonPackPtr CKCJsonPack::ImplGetItemNext(void)
{
    CKCJsonPackPtr ptr;
    if (this->IsValid())
    {
        if (m_stepSub.get() == nullptr || !m_stepSub->IsValid())
            m_stepSub = ptr = ImplGetItem(0);
        else
        {
            IKCJsonCore::IJsonCorePtr core = m_stepSub->Core().Json().GetItemNext();
            if (core.get() != nullptr)
            {
                ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
                m_mapSubItm_obj.insert(make_pair(ptr->GetName(), ptr));
                m_stepSub = ptr;
            }
            else m_stepSub.reset();
        }
    }
    return ptr;
}
CKCJsonPack::CKCJsonPackPtr CKCJsonPack::ImplAddItem(const char* name, bool repeat, bool caseSensitive)
{
    string sName = CUtilFunc::PCharSafeToStr(name);
    CKCJsonPackPtr ptr;
    if (this->IsValid())
    {
        if (!repeat)
        {
            CKCJsonPackPtr jsonRes = this->ImplGetItem(name, caseSensitive);
            if (jsonRes.get() != nullptr && jsonRes->IsValid()) return jsonRes;
        }
        IKCJsonCore::IJsonCorePtr core = Core().Json().AddItem(sName, repeat, caseSensitive);
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_obj.insert(make_pair(sName, ptr));
        }
    }
    return ptr;
}
CKCJsonPack::CKCJsonPackPtr CKCJsonPack::ImplAddJson(const char* name, const char* json, bool repeat, bool caseSensitive)
{
    string sName = CUtilFunc::PCharSafeToStr(name);
    string sJson = CUtilFunc::PCharSafeToStr(json);
    CKCJsonPackPtr ptr;
    if (this->IsValid() && !sJson.empty())
    {
        if (!repeat)
        {
            const IKCJson& jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.IsValid()) this->DelItem(name);
        }
        IKCJsonCore::IJsonCorePtr core = Core().Json().AddJson(sName, sJson, repeat, caseSensitive);
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_obj.insert(make_pair(sName, ptr));
        }
    }
    return ptr;
}
CKCJsonPack::CKCJsonPackPtr CKCJsonPack::ImplAddArray(const char* name, bool repeat, bool caseSensitive)
{
    string sName = CUtilFunc::PCharSafeToStr(name);
    CKCJsonPackPtr ptr;
    if (this->IsValid())
    {
        if (!repeat)
        {
            CKCJsonPackPtr jsonRes = this->ImplGetItem(name, caseSensitive);
            if (jsonRes.get() != nullptr && jsonRes->IsValid()) return jsonRes;
        }
        IKCJsonCore::IJsonCorePtr core = Core().Json().AddArray(sName, repeat, caseSensitive);
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_obj.insert(make_pair(sName, ptr));
        }
    }
    return ptr;
}

// key名字
const char* CKCJsonPack::GetName(void) const
{
    if (IsValid())
    {
        static thread_local string sStr;
        sStr = Core().Json().GetName();
        return sStr.c_str();
    }
    return "";
}

// 数值类型
IKCJson::EDataType CKCJsonPack::GetType(void) const
{
    return IsValid() ? Core().Json().GetType() : IKCJson::edtNull;
}

// 父项
const IKCJson& CKCJsonPack::Own(void) const
{
    throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name());
}
bool CKCJsonPack::IsSub(void) const
{
    return false;
}

// 子项
bool CKCJsonPack::HasItem(const char* name, bool caseSensitive) const
{
    return IsValid() ? Core().Json().HasItem(CUtilFunc::PCharSafeToStr(name), caseSensitive) : false;
}
bool CKCJsonPack::HasItem(const char* name) const
{
    return HasItem(name, m_cb.JsonCaseSensitive());
}
unsigned CKCJsonPack::GetItemCount(void) const
{
    return IsValid() ? Core().Json().GetItemCount() : 0;
}
IKCJson& CKCJsonPack::GetItem(unsigned id)
{
    auto ptr = ImplGetItem(id);
    if (ptr.get() != nullptr) return *ptr;
    return m_cb.InvalidJson();
}
const IKCJson& CKCJsonPack::GetItem(unsigned id) const
{
    return const_cast<CKCJsonPack&>(*this).GetItem(id);
}
IKCJson& CKCJsonPack::GetItem(const char* name, bool caseSensitive)
{
    auto ptr = ImplGetItem(name, caseSensitive);
    if (ptr.get() != nullptr) return *ptr;
    return m_cb.InvalidJson();
}
const IKCJson& CKCJsonPack::GetItem(const char* name, bool caseSensitive) const
{
    return const_cast<CKCJsonPack&>(*this).GetItem(name, caseSensitive);
}
IKCJson& CKCJsonPack::GetItem(const char* name)
{
    return GetItem(name, m_cb.JsonCaseSensitive());
}
const IKCJson& CKCJsonPack::GetItem(const char* name) const
{
    return GetItem(name, m_cb.JsonCaseSensitive());
}
IKCJson& CKCJsonPack::GetItem(const char* name, const char* nameSplit, bool caseSensitive)
{
    // string sActName = CUtilFunc::PCharSafeToStr(name);
    // string sNameSplit = CUtilFunc::PCharSafeToStr(nameSplit);
    // vector<string> subVec;
    // algorithm::split_regex(subVec, sActName, boost::regex(sNameSplit));
    // IKCJson *pJson = this;
    // for (auto &sub : subVec)
    // {
    //     if (sub.empty() || nullptr == pJson || !pJson->IsValid()) break;
    //     try
    //     {
    //         if (pJson->GetType() == IKCJson::edtArray)
    //         {
    //             pJson = &pJson->GetItem(lexical_cast<int>(sub));
    //             continue;
    //         }
    //     }
    //     catch (...) {}
    //     pJson = &pJson->GetItem(sub.c_str(), caseSensitive);
    // }
    // if (nullptr == pJson) pJson = &m_cb.InvalidJson();
    // return *pJson;
    return CCtrlCommon::GetSubItem(*this, CUtilFunc::PCharSafeToStr(name), CUtilFunc::PCharSafeToStr(nameSplit), caseSensitive);
}
const IKCJson& CKCJsonPack::GetItem(const char* name, const char* nameSplit, bool caseSensitive) const
{
    return const_cast<CKCJsonPack&>(*this).GetItem(name, nameSplit, caseSensitive);
}
IKCJson& CKCJsonPack::GetItem(const char* name, const char* nameSplit)
{
    return GetItem(name, nameSplit, m_cb.JsonCaseSensitive());
}
const IKCJson& CKCJsonPack::GetItem(const char* name, const char* nameSplit) const
{
    return const_cast<CKCJsonPack&>(*this).GetItem(name, nameSplit);
}
void CKCJsonPack::InitItemNextStep(void) const
{
    m_stepSub.reset();
}
IKCJson& CKCJsonPack::GetItemNext(void)
{
    auto ptr = ImplGetItemNext();
    if (ptr.get() != nullptr) return *ptr;
    return m_cb.InvalidJson();
}
const IKCJson& CKCJsonPack::GetItemNext(void) const
{
    return const_cast<CKCJsonPack&>(*this).GetItemNext();
}

double CKCJsonPack::GetVal(const char* name, double def, bool caseSensitive) const
{
    return IsValid() ? Core().Json().GetVal(CUtilFunc::PCharSafeToStr(name), def, caseSensitive) : def;
}
double CKCJsonPack::GetVal(const char* name, double def) const
{
    return GetVal(name, def, m_cb.JsonCaseSensitive());
}
const char* CKCJsonPack::GetStr(const char* name, const char* def, bool caseSensitive, const char* charset) const
{
    if (IsValid())
    {
        static thread_local string sStr;
        sStr = Core().Json().GetStr(CUtilFunc::PCharSafeToStr(name), def, caseSensitive);
        if (CUtilFunc::PCharSafeToStr(charset) == c_RESTful_GBK) sStr = CUtilFunc::Utf8ToGbk(sStr);
        return sStr.c_str();
    }
    return def;
}
const char* CKCJsonPack::GetStr(const char* name, const char* def) const
{
    return GetStr(name, def, m_cb.JsonCaseSensitive(), m_cb.GetCharset());
}
bool CKCJsonPack::IsNull(const char* name, bool caseSensitive) const
{
    return IsValid() ? Core().Json().IsNull(CUtilFunc::PCharSafeToStr(name), caseSensitive) : true;
}
bool CKCJsonPack::IsNull(const char* name) const
{
    return IsNull(name, m_cb.JsonCaseSensitive());
}
IKCJson& CKCJsonPack::AddItem(const char* name, bool repeat, bool caseSensitive)
{
    auto ptr = ImplAddItem(name, repeat, caseSensitive);
    return ptr.get() != nullptr ? *ptr : m_cb.InvalidJson();
}
IKCJson& CKCJsonPack::AddItem(const char* name, bool repeat)
{
    return AddItem(name, repeat, m_cb.JsonCaseSensitive());
}
IKCJson& CKCJsonPack::AddItem(unsigned id, bool bReplace)
{
    CKCJsonPackPtr ptr;
    if (this->IsValid())
    {
        IKCJsonCore::IJsonCorePtr core = Core().Json().AddItem(id, bReplace);
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_obj.insert(make_pair(std::to_string(id), ptr));
        }
    }
    return ptr.get() != nullptr ? *ptr : m_cb.InvalidJson();
}
IKCJson& CKCJsonPack::AddArray(const char* name, bool repeat, bool caseSensitive)
{
    auto ptr = ImplAddArray(name, repeat, caseSensitive);
    return ptr.get() != nullptr ? *ptr : m_cb.InvalidJson();
}
IKCJson& CKCJsonPack::AddArray(const char* name, bool repeat)
{
    return AddArray(name, repeat, m_cb.JsonCaseSensitive());
}
IKCJson& CKCJsonPack::AddArray(unsigned id, bool bReplace)
{
    CKCJsonPackPtr ptr;
    if (this->IsValid())
    {
        IKCJsonCore::IJsonCorePtr core = Core().Json().AddArray(id, bReplace);
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_obj.insert(make_pair(std::to_string(id), ptr));
        }
    }
    return ptr.get() != nullptr ? *ptr : m_cb.InvalidJson();
}
IKCJson& CKCJsonPack::AddJson(const char* name, const char* json, bool repeat, bool caseSensitive)
{
    auto ptr = ImplAddJson(name, json, repeat, caseSensitive);
    return ptr.get() != nullptr ? *ptr : m_cb.InvalidJson();
}
IKCJson& CKCJsonPack::AddJson(const char* name, const char* json, bool repeat)
{
    return AddJson(name, json, repeat, m_cb.JsonCaseSensitive());
}
bool CKCJsonPack::AddJson(const char* name, IKCJson& sub, bool repeat, bool caseSensitive)
{
    if (IsValid())
    {
        CKCJsonPack* pck = dynamic_cast<CKCJsonPack*>(&sub);
        if (nullptr != pck) return Core().Json().AddJson(CUtilFunc::PCharSafeToStr(name), pck->Core().Json(), repeat, caseSensitive);
    }
    return false;
}
bool CKCJsonPack::AddJson(const char* name, IKCJson& sub, bool repeat)
{
    return AddJson(name, sub, repeat, m_cb.JsonCaseSensitive());
}
IKCJson& CKCJsonPack::AddJson(unsigned id, const char* json, bool bReplace)
{
    CKCJsonPackPtr ptr;
    if (this->IsValid())
    {
        IKCJsonCore::IJsonCorePtr core = Core().Json().AddJson(id, CUtilFunc::PCharSafeToStr(json, "{}"), bReplace);
        if (core.get() != nullptr)
        {
            ptr.reset(new CKCJsonPackSub(m_cb, core, *this));
            m_mapSubItm_obj.insert(make_pair(std::to_string(id), ptr));
        }
    }
    return ptr.get() != nullptr ? *ptr : m_cb.InvalidJson();
}
bool CKCJsonPack::AddNull(const char* name)
{
    return IsValid() ? Core().Json().AddNull(CUtilFunc::PCharSafeToStr(name)) : false;
}
bool CKCJsonPack::AddVal(const char* name, double val)
{
    return IsValid() ? Core().Json().AddVal(CUtilFunc::PCharSafeToStr(name), val) : false;
}
bool CKCJsonPack::AddStr(const char* name, const char* str, const char* charset)
{
    if (IsValid())
    {
        string sStr = CUtilFunc::PCharSafeToStr(str);
        if (CUtilFunc::PCharSafeToStr(charset) == c_RESTful_GBK) sStr = CUtilFunc::Utf8ToGbk(sStr);
        return Core().Json().AddStr(CUtilFunc::PCharSafeToStr(name), sStr);
    }
    return false;
}
bool CKCJsonPack::AddStr(const char* name, const char* str)
{
    return AddStr(name, str, m_cb.GetCharset());
}
bool CKCJsonPack::SetNull(const char* name, bool caseSensitive)
{
    return IsValid() ? Core().Json().SetNull(CUtilFunc::PCharSafeToStr(name), caseSensitive) : false;
}
bool CKCJsonPack::SetNull(const char* name)
{
    return SetNull(name, m_cb.JsonCaseSensitive());
}
bool CKCJsonPack::SetNull(unsigned id)
{
    return IsValid() ? Core().Json().SetNull(id) : false;
}
bool CKCJsonPack::SetVal(const char* name, double val, bool caseSensitive)
{
    return IsValid() ? Core().Json().SetVal(CUtilFunc::PCharSafeToStr(name), val, caseSensitive) : 0;
}
bool CKCJsonPack::SetVal(const char* name, double val)
{
    return SetVal(name, val, m_cb.JsonCaseSensitive());
}
bool CKCJsonPack::SetVal(unsigned id, double val)
{
    return IsValid() ? Core().Json().SetVal(id, val) : 0;
}
bool CKCJsonPack::SetStr(const char* name, const char* str, bool caseSensitive, const char* charset)
{
    if (IsValid())
    {
        string sStr = CUtilFunc::PCharSafeToStr(str);
        if (CUtilFunc::PCharSafeToStr(charset) == c_RESTful_GBK) sStr = CUtilFunc::Utf8ToGbk(sStr);
        return Core().Json().SetStr(CUtilFunc::PCharSafeToStr(name), sStr, caseSensitive);
    }
    return false;
}
bool CKCJsonPack::SetStr(const char* name, const char* str)
{
    return SetStr(name, str, m_cb.JsonCaseSensitive(), m_cb.GetCharset());
}
bool CKCJsonPack::SetStr(unsigned id, const char* str)
{
    if (IsValid())
    {
        string sStr = CUtilFunc::PCharSafeToStr(str);
        if (CUtilFunc::PCharSafeToStr(m_cb.GetCharset()) == c_RESTful_GBK) sStr = CUtilFunc::Utf8ToGbk(sStr);
        return Core().Json().SetStr(id, sStr);
    }
    return false;
}
bool CKCJsonPack::SetJson(const char* name, const char* json, bool caseSensitive)
{
    return IsValid() ? Core().Json().SetJson(CUtilFunc::PCharSafeToStr(name), CUtilFunc::PCharSafeToStr(json), caseSensitive) : false;
}
bool CKCJsonPack::SetJson(const char* name, const char* json)
{
    return SetJson(name, json, m_cb.JsonCaseSensitive());
}
bool CKCJsonPack::SetJson(const char* name, IKCJson& sub, bool caseSensitive)
{
    if (IsValid())
    {
        CKCJsonPack* pck = dynamic_cast<CKCJsonPack*>(&sub);
        if (nullptr != pck) return Core().Json().SetJson(CUtilFunc::PCharSafeToStr(name), pck->Core().Json(), caseSensitive);
    }
    return false;
}
bool CKCJsonPack::SetJson(const char* name, IKCJson& sub)
{
    return SetJson(name, sub, m_cb.JsonCaseSensitive());
}
bool CKCJsonPack::SetJson(unsigned id, IKCJson& sub)
{
    if (IsValid())
    {
        CKCJsonPack* pck = dynamic_cast<CKCJsonPack*>(&sub);
        if (nullptr != pck) return Core().Json().SetJson(id, pck->Core().Json());
    }
    return false;
}
bool CKCJsonPack::DelItem(unsigned id)
{
    return IsValid() ? Core().Json().DelItem(id) : false;
}
bool CKCJsonPack::DelItem(const char* name, bool caseSensitive)
{
    return IsValid() ? Core().Json().DelItem(CUtilFunc::PCharSafeToStr(name), caseSensitive) : false;
}
bool CKCJsonPack::DelItem(const char* name)
{
    return DelItem(name, m_cb.JsonCaseSensitive());
}

// 读取
double CKCJsonPack::GetVal(double def) const
{
    return IsValid() ? Core().Json().GetVal(def) : def;
}
const char* CKCJsonPack::GetStrSelf(const char* def, const char* charset) const
{
    if (IsValid())
    {
        static thread_local string sStr;
        sStr = Core().Json().GetStr(def);
        if (CUtilFunc::PCharSafeToStr(charset) == c_RESTful_GBK) sStr = CUtilFunc::Utf8ToGbk(sStr);
        return sStr.c_str();
    }
    return def;
}
const char* CKCJsonPack::GetStr(const char* def) const
{
    return GetStrSelf(def, m_cb.GetCharset());
}
bool CKCJsonPack::IsNull(void) const
{
    return IsValid() ? Core().Json().IsNull() : true;
}

// 写入
bool CKCJsonPack::SetVal(double val)
{
    return IsValid() ? Core().Json().SetVal(val) : false;
}
bool CKCJsonPack::SetStrSelf(const char* str, const char* charset)
{
    if (IsValid())
    {
        string sStr = CUtilFunc::PCharSafeToStr(str);
        if (CUtilFunc::PCharSafeToStr(charset) == c_RESTful_GBK) sStr = CUtilFunc::Utf8ToGbk(sStr);
        return Core().Json().SetStr(sStr);
    }
    return false;
}
bool CKCJsonPack::SetStr(const char* str)
{
    return SetStrSelf(str, m_cb.GetCharset());
}

// 判断
bool CKCJsonPack::EqualTo(const IKCJson& rv, bool caseSensitive) const
{
    const CKCJsonPack* pck = dynamic_cast<const CKCJsonPack*>(&rv);
    if (IsValid() && nullptr != pck) return Core().Json().EqualTo(pck->Core().Json(), caseSensitive);
    else return false;
}
bool CKCJsonPack::EqualTo(const IKCJson& rv) const
{
    return EqualTo(rv, m_cb.GetCharset());
}

// 用其他根项重置（只针对根）
bool CKCJsonPack::ResetByRoot(const IKCJson&)
{
    throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name());
}

// 用字符串重置json（只针对根）
bool CKCJsonPack::ResetByStr(const char* /*sNewJsonStr*/)
{
    throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name());
}

// 内存回收
bool CKCJsonPack::GC(void) const
{
    GCImpl();
    return true;
}
void CKCJsonPack::GC(const char* name) const
{
    string sName = CUtilFunc::PCharSafeToStr(name);
    auto it = m_mapSubItm_obj.find(sName);
    if (m_mapSubItm_obj.end() != it) m_mapSubItm_obj.erase(it);
}
void CKCJsonPack::GCImpl(void) const
{
    m_mapSubItm_obj.clear();
    m_mapSubItm_inArr.clear();
    m_stepSub.reset();
}

// 生成字符串
const char* CKCJsonPack::ToStr(int fmt) const
{
    if (IsValid())
        return Core().Json().ToStr(2 == fmt || (m_cb.GetCfgLogLevel() <= 1 && 1 != fmt));
    return "{}";
}

// 源最终的错误码和错误信息
int CKCJsonPack::GetErrCode(void) const
{
    throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name());
}
const char* CKCJsonPack::GetErrMsg(void) const
{
    throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name());
}
