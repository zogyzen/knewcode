#include "std.h"
#include "action_data.h"
#include "page_data.h"

////////////////////////////////////////////////////////////////////////////////
// CActionData类
KC::CActionData::CActionData(IKCRequestRespond& re, IWebPageData& pd)
        : m_now(this), m_re(re), m_pd(pd)
        , m_mtxTmpSimData(m_pd.GetFactory().getContext().LockWork())
        , m_mtxTmpInfData(m_pd.GetFactory().getContext().LockWork())
{
}

KC::CActionData::CActionData(IKCActionData& own, bool prv, IWebPageData& pd)
        : m_own(&own), m_now(dynamic_cast<CActionData&>(own.GetActRoot()).m_now)
        , m_private(prv), m_re(own.GetRequestRespond()), m_pd(pd)
        , m_mtxTmpSimData(m_pd.GetFactory().getContext().LockWork())
        , m_mtxTmpInfData(m_pd.GetFactory().getContext().LockWork())
{
    dynamic_cast<CActionData&>(own.GetActRoot()).m_now = this;
}

KC::CActionData::~CActionData()
{
    if (nullptr != m_own) dynamic_cast<CActionData&>(this->GetActRoot()).m_now = m_now;
    else m_now = nullptr;
}

// 得到页面请求和应答
IKCRequestRespond& KC::CActionData::GetRequestRespond(void)
{
    return m_re;
}

// 得到归属
IActionData* KC::CActionData::GetActOwn(void)
{
    return m_own;
}
IActionData& KC::CActionData::GetActRoot(void)
{
    IActionData* act = this;
    for (; nullptr != act->GetActOwn(); act = act->GetActOwn());
    return *act;
}
IActionData& KC::CActionData::GetActNow(void)
{
    if (nullptr == m_own && nullptr == m_now)
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, m_pd.GetFactory().getHint("Null_Point_Data_") + string("GetActNow"), m_pd.GetFactory());
    return nullptr == m_own ? *m_now : this->GetActRoot().GetActNow();
}

// 写日志
bool KC::CActionData::WriteLogTrace(const char* info, const char* place, const char* other) const
{
    string sInfo = "[Line: " + lexical_cast<string>(this->GetCurrLineID()) + "]" + info;
    return m_pd.GetFactory().WriteLogTrace(sInfo.c_str(), place, other);
}
bool KC::CActionData::WriteLogDebug(const char* info, const char* place, const char* other) const
{
    string sInfo = "[Line: " + lexical_cast<string>(this->GetCurrLineID()) + "]" + info;
    return m_pd.GetFactory().WriteLogDebug(sInfo.c_str(), place, other);
}
bool KC::CActionData::WriteLogInfo(const char* info, const char* place, const char* other) const
{
    string sInfo = "[Line: " + lexical_cast<string>(this->GetCurrLineID()) + "]" + info;
    return m_pd.GetFactory().WriteLogInfo(sInfo.c_str(), place, other);
}
bool KC::CActionData::WriteLogError(const char* info, const char* place, const char* other) const
{
    string sInfo = this->GetCurrPosInfo() + info;
    return m_pd.GetFactory().WriteLogError(sInfo.c_str(), place, other);
}

// 解析并执行字符串脚本
bool KC::CActionData::ParseActionScript(const char* name, const char* script)
{
    bool bResult = false;
    string sName = "[" + posix_time::to_iso_string(posix_time::microsec_clock::local_time()) + "R" + lexical_cast<string>(rand()) + "]_" + name;
    try
    {
        IWebPageData& pdSub = m_pd.GetFactory().WebPageData(*this, sName.c_str(), script);
        CAutoRelease _auto([&](){ m_pd.GetFactory().RemoveWebPageData(sName.c_str()); });
        bResult = pdSub.Action(*this, false);
    }
    catch(TException& ex)
    {
        ex.CurrPosInfo() = this->GetCurrPosInfo() + ex.CurrPosInfo();
        ex.ExceptType() = typeid(ex).name();
        ex.OtherInfo() = sName;
        TLogInfo log(ex, string("ParseActionScript: \r\n") + script + "\r\n");
        log.m_place = __FUNCTION__;
        m_pd.GetFactory().WriteLog(log);
        throw ex;
    }
    catch(std::exception& e)
    {
        TException ex(this->GetCurrLineID(), __FUNCTION__, e.what());
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = typeid(e).name();
        ex.OtherInfo() = sName;
        m_pd.GetFactory().WriteLog(TLogInfo(ex, string("ParseActionScript: \r\n") + script + "\r\n"));
        throw ex;
    }
	catch(const char* e)
	{
        TException ex(this->GetCurrLineID(), __FUNCTION__, e);
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = "const char*";
        ex.OtherInfo() = sName;
        m_pd.GetFactory().WriteLog(TLogInfo(ex, string("ParseActionScript: \r\n") + script + "\r\n"));
        throw ex;
	}
	catch(int e)
	{
        TException ex(this->GetCurrLineID(), __FUNCTION__, lexical_cast<string>(e));
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = "int";
        ex.OtherInfo() = sName;
        m_pd.GetFactory().WriteLog(TLogInfo(ex, string("ParseActionScript: \r\n") + script + "\r\n"));
        throw ex;
	}
    catch(...)
    {
        TException ex(this->GetCurrLineID(), __FUNCTION__, m_pd.GetFactory().getHint("Unknown_exception"));
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = "Unknown_exception";
        ex.OtherInfo() = sName;
        m_pd.GetFactory().WriteLog(TLogInfo(ex, string("ParseActionScript: \r\n") + script + "\r\n"));
        throw ex;
    }
    return bResult;
}

// 创建、释放简单数据或接口
IKcWebDataVal& KC::CActionData::CreateDataVal(void)
{
    TKcWebDataVal* pVal = new TKcWebDataVal();
    TKcWebDataValPtr ValPtr(pVal);
    CKcLock lck(m_mtxTmpSimData, "Create Data Value", __FUNCTION__);
    m_KcWebDataValList.insert(make_pair((void*)pVal, ValPtr));
    return *pVal;
}

bool KC::CActionData::ReleaseDataVal(IKcWebDataVal& val)
{
    CKcLock lck(m_mtxTmpSimData, "Release Data Value", __FUNCTION__);
    auto it = m_KcWebDataValList.find((void*)&val);
    if (m_KcWebDataValList.end() != it) m_KcWebDataValList.erase(it);
    return true;
}

IKcWebInfVal& KC::CActionData::CreateInfVal(void)
{
    TKcWebInfVal* pInf = new TKcWebInfVal();
    TKcWebInfValPtr InfPtr(pInf);
    CKcLock lck(m_mtxTmpInfData, "Create Interface", __FUNCTION__);
    m_KcWebInfValList.insert(make_pair((void*)pInf, InfPtr));
    return *pInf;
}

bool KC::CActionData::ReleaseInfVal(IKcWebInfVal& inf)
{
    CKcLock lck(m_mtxTmpInfData, "Release Interface", __FUNCTION__);
    auto it = m_KcWebInfValList.find((void*)&inf);
    if (m_KcWebInfValList.end() != it) m_KcWebInfValList.erase(it);
    return true;
}

// 标识符、内部变量是否存在
bool KC::CActionData::ExistsVar(const char* n) const
{
    IActionData* pOwn = const_cast<CActionData*>(this)->GetActOwn();
    auto it = m_IDNameItemList.find(n);
    return m_IDNameItemList.end() != it || (nullptr != pOwn && pOwn->ExistsVar(n));
}

bool KC::CActionData::ExistsInnerVar(const char* t, const char* a) const
{
    TAnyTypeValList arr;
    arr.push_back(string(a));
    return dynamic_cast<CWebPageData&>(m_pd).getInnerVarInf().Exists(const_cast<CActionData&>(*this), t, arr);
}

// 接口变量
IKcWebInfVal& KC::CActionData::GetInterfaceVar(const char* n)
{
    // 得到接口变量标识符
    IVarItemBase* pVarIDName = dynamic_cast<IVarItemBase*>(&this->GetIDName(n));
    if (nullptr == pVarIDName)
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifiers_are_not_a_variable")) % n % this->GetCurrLineID()).str(), m_pd.GetFactory());
    boost::any val = pVarIDName->GetVarValue();
    if (val.empty() || val.type() != typeid(TKcWebInfVal))
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifiers_are_not_a_interface_variable")) % n % this->GetCurrLineID()).str(), m_pd.GetFactory());
    // 保存临时值
    TKcWebInfVal* pInf = new TKcWebInfVal(boost::any_cast<TKcWebInfVal>(val));
    TKcWebInfValPtr InfPtr(pInf);
    CKcLock lck(m_mtxTmpInfData, "Save Interface, at line - " + lexical_cast<string>(this->GetCurrLineID()) + ", by " + n, __FUNCTION__);
    m_KcWebInfValList.insert(make_pair((void*)pInf, InfPtr));
    return *pInf;
}

bool KC::CActionData::SetInterfaceVar(const char* n, IKcWebInfVal& inf)
{
    // 得到接口变量标识符
    IVarItemBase* pVarIDName = dynamic_cast<IVarItemBase*>(&this->GetIDName(n));
    if (nullptr == pVarIDName)
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifiers_are_not_a_variable")) % n % this->GetCurrLineID()).str(), m_pd.GetFactory());
    // 设置新接口
    TKcWebInfVal newInf(dynamic_cast<TKcWebInfVal&>(inf));
    pVarIDName->SetVarValue(newInf);
    return true;
}

// 简单类型变量值
IKcWebDataVal& KC::CActionData::GetVarValue(const char* n)
{
    TKcWebDataVal* pVal = new TKcWebDataVal();
    TKcWebDataValPtr ValPtr(pVal);
    // 得到变量标识符
    IVarItemBase* pVarIDName = dynamic_cast<IVarItemBase*>(&this->GetIDName(n));
    if (nullptr == pVarIDName)
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifiers_are_not_a_variable")) % n % this->GetCurrLineID()).str(), m_pd.GetFactory());
    boost::any val = pVarIDName->GetVarValue();
    if (val.empty())
        pVal->dataType = EKcDtVoid;
    else if (val.type() == typeid(bool))
        pVal->dataType = EKcDtBool, pVal->SetVal(boost::any_cast<bool>(val));
    else if (val.type() == typeid(int))
        pVal->dataType = EKcDtInt, pVal->SetVal(boost::any_cast<int>(val));
    else if (val.type() == typeid(double))
        pVal->dataType = EKcDtDouble, pVal->SetVal(boost::any_cast<double>(val));
    else if (val.type() == typeid(string))
        pVal->dataType = EKcDtStr, pVal->SetVal(boost::any_cast<string>(val).c_str());
    else
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifiers_are_not_a_base_type_variable")) % n % this->GetCurrLineID()).str(), m_pd.GetFactory());
    // 保存临时值
    CKcLock lck(m_mtxTmpSimData, "Save Data Value, at line - " + lexical_cast<string>(this->GetCurrLineID()) + ", by " + n, __FUNCTION__);
    m_KcWebDataValList.insert(make_pair((void*)pVal, ValPtr));
    return *pVal;
}

bool KC::CActionData::SetVarValue(const char* n, IKcWebDataVal& val)
{
    // 得到接口变量标识符
    IVarItemBase* pVarIDName = dynamic_cast<IVarItemBase*>(&this->GetIDName(n));
    if (nullptr == pVarIDName)
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifiers_are_not_a_variable")) % n % this->GetCurrLineID()).str(), m_pd.GetFactory());
    // 设置新接口
    TKcWebDataVal newVal(dynamic_cast<TKcWebDataVal&>(val));
    pVarIDName->SetVarValue(newVal);
    return true;
}

// 内部变量值
IKcWebInfVal& KC::CActionData::GetInnerVarInf(const char* tp, const char* nm)
{
    // 得到内部变量接口
    TAnyTypeValList arr;
    arr.push_back(string(nm));
    boost::any val = dynamic_cast<CWebPageData&>(m_pd).getInnerVarInf().GetValue(*this, tp, arr);
    if (val.empty() || val.type() != typeid(TKcWebInfVal))
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifiers_are_not_a_interface_variable")) % nm % this->GetCurrLineID()).str(), m_pd.GetFactory());
    // 保存临时值
    TKcWebInfVal* pInf = new TKcWebInfVal(boost::any_cast<TKcWebInfVal>(val));
    TKcWebInfValPtr InfPtr(pInf);
    CKcLock lck(m_mtxTmpInfData, "Save Inner Interface, at line - " + lexical_cast<string>(this->GetCurrLineID()) + ", by " + tp + " - " + nm, __FUNCTION__);
    m_KcWebInfValList.insert(make_pair((void*)pInf, InfPtr));
    return *pInf;
}

IKcWebDataVal& KC::CActionData::GetInnerVarVal(const char* tp, const char* nm)
{
    // 得到内部变量值
    TKcWebDataVal* pVal = new TKcWebDataVal();
    TKcWebDataValPtr ValPtr(pVal);
    TAnyTypeValList arr;
    arr.push_back(string(nm));
    boost::any val = dynamic_cast<CWebPageData&>(m_pd).getInnerVarInf().GetValue(*this, tp, arr);
    if (val.empty())
        pVal->dataType = EKcDtVoid;
    else if (val.type() == typeid(bool))
        pVal->dataType = EKcDtBool, pVal->SetVal(boost::any_cast<bool>(val));
    else if (val.type() == typeid(int))
        pVal->dataType = EKcDtInt, pVal->SetVal(boost::any_cast<int>(val));
    else if (val.type() == typeid(double))
        pVal->dataType = EKcDtDouble, pVal->SetVal(boost::any_cast<double>(val));
    else if (val.type() == typeid(string))
        pVal->dataType = EKcDtStr, pVal->SetVal(boost::any_cast<string>(val).c_str());
    // 保存临时值
    CKcLock lck(m_mtxTmpSimData, "Save Inner Data Value, at line - " + lexical_cast<string>(this->GetCurrLineID()) + ", by " + tp + " - " + nm, __FUNCTION__);
    m_KcWebDataValList.insert(make_pair((void*)pVal, ValPtr));
    return *pVal;
}

bool KC::CActionData::SetInnerVar(const char* tp, const char* nm, IKcWebInfVal& inf)
{
    // 设置新接口
    TAnyTypeValList arr;
    arr.push_back(string(nm));
    TKcWebInfVal newInf(dynamic_cast<TKcWebInfVal&>(inf));
    dynamic_cast<CWebPageData&>(m_pd).getInnerVarInf().SetValue(*this, tp, arr, newInf);
    return true;
}

bool KC::CActionData::SetInnerVar(const char* tp, const char* nm, IKcWebDataVal& val)
{
    // 设置新值
    TAnyTypeValList arr;
    arr.push_back(string(nm));
    TKcWebDataVal newVal(dynamic_cast<TKcWebDataVal&>(val));
    dynamic_cast<CWebPageData&>(m_pd).getInnerVarInf().SetValue(*this, tp, arr, newVal);
    return true;
}

// 获取表达式的值
boost::any KC::CActionData::GetExprValue(const TKcWebExpr& expr)
{
    // 操作数
    IOperandNode *pOperand = dynamic_cast<IOperandNode*>(expr.exprTreeNodePtr.get());
    if (nullptr == pOperand)
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, m_pd.GetFactory().getHint("Not_Set_operand_") + lexical_cast<string>(this->GetCurrLineID()), m_pd.GetFactory());
    // 返回操作数的值
    return pOperand->GetValue(*this);
}

// 设置内部变量的值（参数依次为：变量名、下标、值）
void KC::CActionData::SetInnerVarValue(string sName, TAnyTypeValList& arr, boost::any val)
{
    dynamic_cast<CWebPageData&>(m_pd).getInnerVarInf().SetValue(*this, sName, arr, val);
}

// 垃圾回收
void KC::CActionData::GC(void)
{
    m_pd.GetFactory().GC();
}

// 获取本地化提示信息
const char* KC::CActionData::getHint(const char* key) const
{
    return m_pd.GetFactory().getHint(key);
}

// 抛一个字符串异常
void KC::CActionData::ThrowString(const char* ex) const
{
    throw ex;
}

// 得到完整路径
string KC::CActionData::GetFullPath(string sPath)
{
    sPath = boost::trim_copy(sPath);
    string sResult = sPath;
    // 加配置目录
    if (sPath[0] == '<' && sPath.find('>') != string::npos)
    {
        int iEndTag = sPath.find('>') + 1;
        string sCfg = sPath.substr(0, iEndTag);
        sPath.erase(0, iEndTag);
        sCfg.erase(sCfg.size() - 1);
        sCfg.erase(0, 1);
        try
        {
            sResult = m_pd.GetFactory().getContext().getPathCfg(sCfg.c_str()) + sPath;
        }
        catch(TFWException& ex)
        {
            throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, ex.error_info() + ", at line - " + lexical_cast<string>(this->GetCurrLineID()), m_pd.GetFactory());
        }
    }
    // 加网站根目录
    else if (sPath[0] == '/' || sPath[0] == '\\')
        sResult = m_re.GetLocalRootPath() + sPath;
    // 加页目录
    else
        sResult = m_re.GetLocalPagePath() + ("/" + sPath);
    return sResult;
}

// 获取标识符
IIDNameItem& KC::CActionData::GetIDName(string sName)
{
    // 在本层内查找
    if (!m_IDNameItemList.empty())
    {
        auto iter = m_IDNameItemList.find(sName);
        if (m_IDNameItemList.end() != iter)
            return *(iter->second.get());
    }
    // 本层没找到，继续向上层查找
    if (nullptr == this->GetActOwn())
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifier_definition_does_not_exist")) % sName % this->GetCurrLineID()).str(), m_pd.GetFactory());
    return dynamic_cast<IKCActionData&>(*this->GetActOwn()).GetIDName(sName);
}

// 添加标识符
bool KC::CActionData::AddIDName(string sName, TIDNameItermPtr IDPtr, bool prv, bool rf, bool stt)
{
    // 判断是否引用已有的变量
    if (rf && this->ExistsVar(sName.c_str())) return true;
    // 在私有页或根页中定义（私有标识符，在本层定义）
    if (nullptr == this->GetActOwn() || m_private || prv)
    {
        // 判断是否重定义静态变量
        if (m_IDNameStaticList.end() != m_IDNameStaticList.find(sName)) return true;
        // 删除重复定义
        auto iter = m_IDNameItemList.find(sName);
        if (m_IDNameItemList.end() != iter) m_IDNameItemList.erase(iter);
        // 添加新加载库标识符定义
        m_IDNameItemList.insert(make_pair(sName, IDPtr));
        if (stt) m_IDNameStaticList.insert(sName);
        return true;
    }
    // 非私有，向上层定义
    else return dynamic_cast<IKCActionData&>(*this->GetActOwn()).AddIDName(sName, IDPtr, prv, false, stt);
}

// 获取接口定义
TKcInfFullFL& KC::CActionData::GetKcInfFullFL(string sName)
{
    // 在本层内查找
    if (!m_InfFullFLList.empty())
    {
        auto iter = m_InfFullFLList.find(sName);
        if (m_InfFullFLList.end() != iter)
            return iter->second;
    }
    // 本层没找到，继续向上层查找
    if (nullptr == this->GetActOwn())
        throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_pd.GetFactory().getHint("Identifier_definition_does_not_exist")) % sName % this->GetCurrLineID()).str(), m_pd.GetFactory());
    return dynamic_cast<IKCActionData&>(*this->GetActOwn()).GetKcInfFullFL(sName);
}

// 添加接口定义
bool KC::CActionData::AddKcInfFullFL(const TKcInfFullFL& inf)
{
    // 在私有页或根页中定义（私有接口，在本层定义）
    if (nullptr == this->GetActOwn() || m_private || inf.GetVal<0>().GetVal<2>())
    {
        string sName = inf.GetVal<0>().GetVal<0>().GetVal<0>();
        // 删除重复定义
        auto iter = m_InfFullFLList.find(sName);
        if (m_InfFullFLList.end() != iter) m_InfFullFLList.erase(iter);
        // 添加新接口定义
        m_InfFullFLList.insert(make_pair(sName, inf));
        return true;
    }
    // 非私有，向上层定义
    else
        return dynamic_cast<IKCActionData&>(*this->GetActOwn()).AddKcInfFullFL(inf);
}

// 得到当前行
int KC::CActionData::GetCurrLineID(void) const
{
    return nullptr == m_CurrSynItm ? -1 : m_CurrSynItm->GetLineID();
}

// 得到详细当前位置
string KC::CActionData::GetCurrPosInfo(string cl) const
{
    string sResult = "";
    for (IKCActionData* act = const_cast<CActionData*>(this); nullptr != act; act = dynamic_cast<IKCActionData*>(act->GetActOwn()))
        sResult = act->GetWebPageData().GetTextBuffer().fileName + ": line " + lexical_cast<string>(act->GetCurrLineID()) + ". " + cl + sResult;
    return cl + sResult + cl;
}

// 得到当前语法字符串
string KC::CActionData::GetCurrSynStr(void)
{
    string sResult = "";
    if (nullptr != m_CurrSynItm)
        sResult = m_pd.GetCurrSynStr(m_CurrSynItm->GetBeginPtr(), m_CurrSynItm->GetEndPtr());
    return sResult;
}

// 非顺序语句的条件栈
TConditionStatck& KC::CActionData::ConditionStatck(void)
{
    return m_ConditionStatck;
}

// 得到页面接口
IWebPageData& KC::CActionData::GetWebPageData(void)
{
    return m_pd;
}

// 延迟语法
int KC::CActionData::DelaySynNewIndex(void) const
{
    return (int)m_SyntaxItemListForDelay.size();
}
void KC::CActionData::AddDelaySyn(TSyntaxItemPtr syn)
{
    m_SyntaxItemListForDelay.push_back(syn);
}

// 处理延迟语法列表
void KC::CActionData::ActionForDelay(void)
{
    try
    {
        if (!m_SyntaxItemListForDelay.empty()) this->ActionForDelayAct();
    }
    catch(TException& ex)
    {
        ex.CurrPosInfo() = this->GetCurrPosInfo() + ex.CurrPosInfo();
        ex.ExceptType() = typeid(ex).name();
        TLogInfo log(ex, " [ActionForDelay] ");
        log.m_place = __FUNCTION__;
        m_pd.GetFactory().WriteLog(log);
    }
    catch(std::exception& e)
    {
        TException ex(this->GetCurrLineID(), __FUNCTION__, e.what());
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = typeid(e).name();
        m_pd.GetFactory().WriteLog(TLogInfo(ex, " [ActionForDelay] "));
    }
	catch(const char* e)
	{
        TException ex(this->GetCurrLineID(), __FUNCTION__, e);
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = "const char*";
        m_pd.GetFactory().WriteLog(TLogInfo(ex, " [ActionForDelay] "));
	}
	catch(int e)
	{
        TException ex(this->GetCurrLineID(), __FUNCTION__, lexical_cast<string>(e));
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = "int";
        m_pd.GetFactory().WriteLog(TLogInfo(ex, " [ActionForDelay] "));
	}
    catch(...)
    {
        TException ex(this->GetCurrLineID(), __FUNCTION__, m_pd.GetFactory().getHint("Unknown_exception"));
        ex.CurrPosInfo() = this->GetCurrPosInfo();
        ex.ExceptType() = "Unknown_exception";
        m_pd.GetFactory().WriteLog(TLogInfo(ex, " [ActionForDelay] "));
    }
}
void KC::CActionData::ActionForDelayAct(void)
{
    // 处理页面
    const char* pos = __FUNCTION__;
    auto TryFunc = [&]()
    {
        m_pd.GetFactory().WriteLogDebug((m_pd.GetFactory().getHint("Action_Delay_Syntax_") + m_pd.GetTextBuffer().fileName).c_str(),
                                         pos, lexical_cast<string>(reinterpret_cast<unsigned int>(this)).c_str());
        // 按顺序循环处理每个语法项
        for (TSyntaxItemPtr syn: m_SyntaxItemListForDelay)
        {
            boost::format fmtDebugInfo = boost::format(m_pd.GetFactory().getHint("Action_syntax_item")) % syn->GetKeychar() % syn->GetLineID() % syn->GetIndex();
            m_pd.GetFactory().WriteLogTrace(fmtDebugInfo.str().c_str(), pos, syn->GetSyntaxContent().c_str());
            syn->ActionItem(*this);
        }
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_re, ("ActionForDelay(" + m_pd.GetTextBuffer().fileName + ")").c_str());
    this->GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
}

// 设置当前语法项
void KC::CActionData::SetCurrSynItm(ISyntaxItem* CurrSynItm)
{
    m_CurrSynItm = CurrSynItm;
}
