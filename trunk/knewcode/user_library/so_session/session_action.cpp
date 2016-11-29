#include "std.h"
#include "session_action.h"
#include "work_session.h"

////////////////////////////////////////////////////////////////////////////////
// 日志
class CKCLog
{
public:
    CKCLog(IActionData& act, string info, string& endInfo, string pos, string name)
        : m_act(act), m_info(info), m_endInfo(endInfo), m_pos(pos), m_name(name)
    {
        m_act.WriteLogDebug(("Lock - Session: " + m_name).c_str(), m_pos.c_str(), m_info.c_str());
    }
    ~CKCLog()
    {
        m_act.WriteLogDebug(("UnLock - Session: " + m_name).c_str(), m_pos.c_str(), (m_info + m_endInfo).c_str());
    }

private:
    IActionData& m_act;
    string m_info, &m_endInfo, m_pos, m_name;
};

static TProcNameList& GetProcMemList(IKCSessionWork& wk)
{
    return dynamic_cast<CKCSessionWork&>(wk).GetProcMemListWork().GetProcMemList();
}

////////////////////////////////////////////////////////////////////////////////
// CKCSession类
CKCSession::CKCSession(IActionData& act, IKCSessionWork& wk)
    : m_act(act.GetActRoot()), m_work(wk)
    , m_invar(dynamic_cast<IServiceReferenceEx&>(*dynamic_cast<IKCActionData&>(act).GetWebPageData().GetFactory().getBundle().getContext().takeServiceReference(c_InnerVarSrvGUID)))
{
}

// 得到session编号
string CKCSession::GetSessionID(void)
{
    IInnerVar& invar = m_invar.getServiceSafe<IInnerVar>();
    // 下标值
    std::vector<boost::any> arrList;
    arrList.push_back(string("__KCPage_Session_ID_1945_0903___"));
    // 获取Session编号
    string SessionID = boost::any_cast<string>(invar.GetValue(m_act, Keychar::two_level::g_InnerCookie, arrList));
    if (SessionID.empty() || SessionID.size() == 0)
    {
        SessionID = "SID" + posix_time::to_iso_string(posix_time::second_clock::local_time()) + "R" + lexical_cast<string>(rand());
        invar.SetValue(m_act, Keychar::two_level::g_InnerCookie, arrList, SessionID);
    }
    return SessionID;
}

// 清除过期Session
void CKCSession::ClearUp(void)
{
}

// 设置Session值
bool CKCSession::Set(const char* n, const char* v)
{
    string sClientIP = m_act.GetRequestRespond().GetClientIP(),
           sSessionID = this->GetSessionID(),
           sPnmName = sClientIP + ">>" + sSessionID + ">>" + n,
           sEndInfo;
    CKCLog lg(m_act, "[" + sSessionID + "@" + sClientIP + "], set[" + n + "]=[" + v + "]", sEndInfo, __FUNCTION__, n);
    GetProcMemList(m_work).SetTimeOut(m_act.GetRequestRespond().GetTimeOutSeconds());
    GetProcMemList(m_work).Set(n, v);
    return GetProcMemList(m_work).Exists(n);
}

// 获取Session值
const char* CKCSession::Get(const char* n)
{
    string sClientIP = m_act.GetRequestRespond().GetClientIP(),
           sSessionID = this->GetSessionID(),
           sPnmName = sClientIP + ">>" + sSessionID + ">>" + n,
           sEndInfo;
    CKCLog lg(m_act, "[" + sSessionID + "@" + sClientIP + "], get[" + n + "]", sEndInfo, __FUNCTION__, n);
    m_tmpStr = GetProcMemList(m_work).Get(n);
    sEndInfo = "=[" + m_tmpStr + "]";
    return m_tmpStr.c_str();
}

// 移除Session
bool CKCSession::Remove(const char* n)
{
    string sClientIP = m_act.GetRequestRespond().GetClientIP(),
           sSessionID = this->GetSessionID(),
           sPnmName = sClientIP + ">>" + sSessionID + ">>" + n,
           sEndInfo;
    CKCLog lg(m_act, "[" + sSessionID + "@" + sClientIP + "], remove[" + n + "]", sEndInfo, __FUNCTION__, n);
    GetProcMemList(m_work).Remove(n);
    return !GetProcMemList(m_work).Exists(n);
}

// 是否存在Session
bool CKCSession::Exists(const char* n)
{
    string sClientIP = m_act.GetRequestRespond().GetClientIP(),
           sSessionID = this->GetSessionID(),
           sPnmName = sClientIP + ">>" + sSessionID + ">>" + n,
           sEndInfo;
    CKCLog lg(m_act, "[" + sSessionID + "@" + sClientIP + "], exists[" + n + "]", sEndInfo, __FUNCTION__, n);
    return GetProcMemList(m_work).Exists(n);
}
