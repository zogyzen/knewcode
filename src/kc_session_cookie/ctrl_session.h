#pragma once

#include "std.h"

class ICtrlSession : public IKCController
{
public:
    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&) override
    {
        // 控制器参数
        const char* pAct = objCtrlD.ActName();
        const char* pMethod = objCtrlD.Method();
        // 异常信息
        auto fExceptInfo = [&](void)
        {
            string sAct(nullptr != pAct ? pAct : "");
            return "\n" + sAct + " - \t" + objCtrlD.GetPostStr();
        };
        try
        {
            auto fGetParmsJson = [&](void) -> const IKCJson&
            {
                const IKCJson& jr = objCtrlD.JsonRequest();
                const IKCJson& jp = jr.GetItem(c_RESTful_inParm);
                return jp.IsValid() ? jp : jr;
            };
            const IKCJson& jsonParms = fGetParmsJson();
            if (jsonParms.IsValid())
            {
                string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "set"));
                // client编号
                const char* pClnID = objCtrlD.ActionData().GetKCCLNID();
                if (nullptr == pClnID || strlen(pClnID) == 0)
                    throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 8, __CURR_CODE_PLACE_C__, string(m_own.getHint("The_field_value_is_NULL_")) + c_RESTful_KCClientID, m_own);
                // session编号
                const char* pSessID = objCtrlD.ActionData().GetKCSSID();
                if (nullptr == pSessID || strlen(pSessID) == 0)
                    throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 9, __CURR_CODE_PLACE_C__, string(m_own.getHint("The_field_value_is_NULL_")) + c_RESTful_KCSessoinID, m_own);
                // session前缀
                string sPrefixName = PrefixName(pSessID, pClnID);
                // 验证session信息
                // 循环每个参数
                for (int i = 0, c = jsonParms.GetItemCount(); i < c; ++i)
                {
                    const IKCJson& jsonParm = jsonParms.GetItem(i);
                    string sSimName = jsonParm.GetName();
                    string sName = sPrefixName + sSimName;
                    string sVal = jsonParm.GetStr();
                    // 删除（method="del"）
                    if ("del" == sMethod) DelSession(sName, &objCtrlD.ActionData());
                    // 获取（method="get"）
                    else if ("get" == sMethod)
                        objCtrlD.JsonRespond().SetStr(sSimName.c_str(), GetSession(sName, &objCtrlD.ActionData()).c_str());
                    // 设置（method="set"）
                    else SetSession(sName, sVal, &objCtrlD.ActionData());
                }
            }
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
            objCtrlD.SetJsonRespond(ecd_ErrCode_KCSessionCookie + 31, ex.what());
        }
        catch (...)
        {
            string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
            m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
            objCtrlD.SetJsonRespond(ecd_ErrCode_KCSessionCookie + 30, (string("Session ") + m_own.getHint("Unknown_exception")).c_str());
        }
    }

public:
    // 前缀名
    virtual string PrefixName(string sSessionID, string sClientID) const = 0;
    // 添加Session
    virtual void SetSession(string sName, string sVal, IActionData* = nullptr) = 0;
    // 得到Session
    virtual string GetSession(string sName, IActionData* = nullptr) = 0;
    // 得到下一个自增值
    virtual string GetSessionNextVal(string sName, string sValBegin, IActionData* = nullptr) = 0;
    // 删除Session
    virtual void DelSession(string sName, IActionData* = nullptr) = 0;
    // 设置期限（d：天。h：小时。s：秒。）
    virtual void SetExpire(string sName, string exp = "1d", IActionData* = nullptr) = 0;

protected:
    // 宿主
    IKCSessionCookie& m_own;
    // 运行状态
    bool m_running = true;

protected:
    // 构造函数
    ICtrlSession(IKCSessionCookie& own) : m_own(own), m_GuardThrd(&ICtrlSession::GuardThrd, this)
    {
    }
    ~ICtrlSession(void) override
    {
        try
        {
            m_running = false;
            // 终止守护线程
            if (m_GuardThrd.joinable()) m_GuardThrd.interrupt();
            if (m_GuardThrd.joinable()) m_GuardThrd.timed_join(boost::posix_time::milliseconds(666));
        }
        catch (...) {}
    }
    // 守护线程
    boost::thread m_GuardThrd;
    void GuardThrd(void)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1666));
        while (m_running)
        {
            boost::this_thread::interruption_point();
            boost::this_thread::sleep(boost::posix_time::seconds(666));
            //m_segment.atomic_func();
            try
            {
                GC();
            }
            catch (interprocess::interprocess_exception &ex)
            {
                m_own.WriteLogError(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch(TException& ex)
            {
                ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
                ex.LineCode() = __LINE__;
                m_own.WriteLog(ex);
            }
            catch (std::exception& ex)
            {
                m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch (...)
            {
                m_own.WriteLogFatal("Unknown_exception", __CURR_CODE_PLACE_C__);
            }
        }
    }
    // 垃圾回收
    virtual void GC(void) = 0;
};

// 按存储类型（共享内存、映射文件）
template<typename TSegment>
class CCtrlSession : public ICtrlSession
{
public:
    CCtrlSession(IKCSessionCookie& own, string sName, property_tree::ptree& pt, bool isShareMem, TSegment& msm);

public:
    // 前缀名
    string PrefixName(string sSessionID, string) const override
    {
        return c_RESTful_KCSession + m_name + "_" + (m_isGlobal ? c_RESTful_KCGlobalKCSSID : sSessionID) + "__";
    }
    // 添加Session
    void SetSession(string sName, string sVal, IActionData* = nullptr) override
    {
        char_string sessName(sName.c_str(), m_alloc_char);
        for (int i = 0; i < 2; ++i)
        {
            TGlobalData &gd = *m_segment.template find_or_construct<TGlobalData>(m_globaDataName.c_str())();
            posix_time::ptime absDT = posix_time::second_clock::local_time() + posix_time::seconds(6);
            if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_lock_upgradable(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 6, ("[Upgradable]" + sName).c_str()))
            {
                CAutoRelease _auto1([&](){ GetGlobalMtx(m_globaDataName).unlock_upgradable(); }, true);
                auto it = m_mapSession.find(sessName);
                if (m_mapSession.end() == it)
                {
                    absDT = posix_time::second_clock::local_time() + posix_time::seconds(16);
                    if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_unlock_upgradable_and_lock(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 16, ("[Unique]" + sName).c_str()))
                    {
                        TSessionData sessData(sName.c_str(), sVal.c_str(), m_alloc_char, MakePTime(m_expire));
                        {
                            CAutoRelease _auto2([&](){ GetGlobalMtx(m_globaDataName).unlock_and_lock_upgradable(); }, true);
                            m_mapSession.insert(map_value_type(sessName, sessData));
                        }
                        m_own.WriteLogDebug("Add Session", __CURR_CODE_PLACE_C__, (sName + "=" + sVal).c_str());
                        break;
                    }
                }
                else
                {
                    it->second.m_expire = MakePTime(m_expire);
                    if (sVal.c_str() != it->second.m_val)
                    {
                        it->second.m_val = sVal.c_str();
                        m_own.WriteLogDebug("Set Session", __CURR_CODE_PLACE_C__, (sName + "=" + sVal).c_str());
                    }
                    break;
                }
            }
        }
    }
    // 得到Session
    string GetSession(string sName, IActionData* = nullptr) override
    {
        char_string sessName(sName.c_str(), m_alloc_char);
        for (int i = 0; i < 2; ++i)
        {
            TGlobalData &gd = *m_segment.template find_or_construct<TGlobalData>(m_globaDataName.c_str())();
            posix_time::ptime absDT = posix_time::second_clock::local_time() + posix_time::seconds(6);
            if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_lock_sharable(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 6, ("[Sharable]" + sName).c_str()))
            {
                CAutoRelease _auto([&](){ GetGlobalMtx(m_globaDataName).unlock_sharable(); }, true);
                auto it = m_mapSession.find(sessName);
                if (m_mapSession.end() != it && it->second.m_expire >= posix_time::second_clock::local_time())
                {
                    TSessionData &sd = it->second;
                    sd.m_expire = MakePTime(m_expire);
                    return sd.m_val.c_str();
                }
                break;
            }
        }
        return "";
    }
    // 得到下一个自增值
    string GetSessionNextVal(string sName, string sValBegin, IActionData* = nullptr) override
    {
        string sResult;
        char_string sessName(sName.c_str(), m_alloc_char);
        for (int i = 0; i < 2; ++i)
        {
            TGlobalData &gd = *m_segment.template find_or_construct<TGlobalData>(m_globaDataName.c_str())();
            posix_time::ptime absDT = posix_time::second_clock::local_time() + posix_time::seconds(6);
            if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_lock_upgradable(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 6, ("[Upgradable]" + sName).c_str()))
            {
                CAutoRelease _auto1([&](){ GetGlobalMtx(m_globaDataName).unlock_upgradable(); }, true);
                auto it = m_mapSession.find(sessName);
                if (m_mapSession.end() == it)
                {
                    absDT = posix_time::second_clock::local_time() + posix_time::seconds(16);
                    if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_unlock_upgradable_and_lock(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 16, ("[Unique]" + sName).c_str()))
                    {
                        TSessionData sessData(sName.c_str(), sValBegin.c_str(), m_alloc_char, MakePTime(m_expire));
                        {
                            CAutoRelease _auto2([&](){ GetGlobalMtx(m_globaDataName).unlock_and_lock_upgradable(); }, true);
                            m_mapSession.insert(map_value_type(sessName, sessData));
                        }
                        sResult = sValBegin;
                        m_own.WriteLogDebug("Add Session", __CURR_CODE_PLACE_C__, (sName + "=" + sResult).c_str());
                        break;
                    }
                }
                else
                {
                    it->second.m_expire = MakePTime(m_expire);
                    // 自增1
                    string sInc = std::to_string(atoll(it->second.m_val.c_str()) + 1);
                    it->second.m_val = sInc.c_str();
                    sResult = sInc;
                    m_own.WriteLogDebug("Set Session", __CURR_CODE_PLACE_C__, (sName + "=" + sResult).c_str());
                    break;
                }
            }
        }
        return sResult;
    }
    // 删除Session
    void DelSession(string sName, IActionData* = nullptr) override
    {
        char_string sessName(sName.c_str(), m_alloc_char);
        for (int i = 0; i < 2; ++i)
        {
            TGlobalData &gd = *m_segment.template find_or_construct<TGlobalData>(m_globaDataName.c_str())();
            posix_time::ptime absDT = posix_time::second_clock::local_time() + posix_time::seconds(6);
            if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_lock_upgradable(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 6, ("[Upgradable]" + sName).c_str()))
            {
                CAutoRelease _auto1([&](){ GetGlobalMtx(m_globaDataName).unlock_upgradable(); }, true);
                auto it = m_mapSession.find(sessName);
                if (m_mapSession.end() != it)
                {
                    absDT = posix_time::second_clock::local_time() + posix_time::seconds(16);
                    if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_unlock_upgradable_and_lock(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 16, ("[Unique]" + sName).c_str()))
                    {
                        {
                            CAutoRelease _auto2([&](){ GetGlobalMtx(m_globaDataName).unlock_and_lock_upgradable(); }, true);
                            m_mapSession.erase(it);
                        }
                        m_own.WriteLogDebug("Del Session", __CURR_CODE_PLACE_C__, sName.c_str());
                        break;
                    }
                }

            }
        }
    }
    // 设置期限（d：天。h：小时。s：秒。）
    void SetExpire(string sName, string exp = "1d", IActionData* = nullptr) override
    {
        char_string sessName(sName.c_str(), m_alloc_char);
        for (int i = 0; i < 2; ++i)
        {
            TGlobalData &gd = *m_segment.template find_or_construct<TGlobalData>(m_globaDataName.c_str())();
            posix_time::ptime absDT = posix_time::second_clock::local_time() + posix_time::seconds(6);
            if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_lock_sharable(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 6, ("[Sharable]" + sName).c_str()))
            {
                CAutoRelease _auto([&](){ GetGlobalMtx(m_globaDataName).unlock_sharable(); }, true);
                auto it = m_mapSession.find(sessName);
                if (m_mapSession.end() != it)
                    it->second.m_expire = MakePTime(exp);
                break;
            }
        }
    }

protected:
    // 转换时间
    posix_time::ptime MakePTime(string sExpire = "1d")
    {
        char unt = sExpire.back();
        int iVal = lexical_cast<int>(sExpire.substr(0, sExpire.size() - 1));
        if ('d' == unt)
            return posix_time::second_clock::local_time() + gregorian::days(iVal);
        else if ('h' == unt)
            return posix_time::second_clock::local_time() + posix_time::hours(iVal);
        else if ('s' == unt)
            return posix_time::second_clock::local_time() + posix_time::seconds(iVal);
        else
            throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 10, __CURR_CODE_PLACE_C__, string(m_own.getHint("Parameter_type_unknown_")) + sExpire, m_own);
    }

private:
    // 控制器名称
    string m_name;
    // 是否全局（全部变量，可以被所有的请求访问。非全局变量，只能被对应 session id 的请求访问）
    bool m_isGlobal = false;
    // 过期设置（d-天。h-小时。s-秒）
    string m_expire = "1d";
    // 是否区分大小写
    bool m_nameCaseSensitive = false;
    // true是共享内存，false是映射文件
    bool m_isShareMem = true;

protected:
    typedef interprocess::interprocess_upgradable_mutex TMtx;
    // 全局Session共享内存锁
    struct TGlobalData
    {
        TMtx m_mtx;
    };
    // Session共享内存
    typedef typename TSegment::segment_manager                                          segment_manager_t;
    typedef interprocess::allocator<char, segment_manager_t>                            char_allocator;
    typedef interprocess::basic_string<char, std::char_traits<char>, char_allocator>    char_string;
    struct TSessionData
    {
        char_string       m_name;
        int               m_type    = 0;   // 0普通数据；>0指向其他共享内存（名称放在m_val里）
        char_string       m_val;
        posix_time::ptime m_expire  = posix_time::second_clock::local_time() + gregorian::days(1);
        //TMtx m_mtx;

        TSessionData(const char *name, const char *val, const char_allocator &alloc, posix_time::ptime exp = posix_time::second_clock::local_time() + gregorian::days(1))
            : m_name(name, alloc), m_val(val, alloc), m_expire(exp) {}
    };
    typedef std::pair<const char_string, TSessionData>                                                                                  map_value_type;
    typedef interprocess::allocator<map_value_type, segment_manager_t>                                                                  map_value_type_allocator;
    typedef interprocess::map<char_string, TSessionData, /*std::less<char_string>*/TKcNameLess<char_string>, map_value_type_allocator>  session_map_type;
    TSegment& m_segment;
    map_value_type_allocator m_alloc_inst_session;
    char_allocator m_alloc_char;
    string m_globaDataName = c_RESTful_KCSession + string("xxx_Global_");
    session_map_type &m_mapSession;

protected:
    // 守护线程
    boost::thread m_GuardThrd;
    void GuardThrd(void)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1666));
        while (m_running)
        {
            boost::this_thread::interruption_point();
            boost::this_thread::sleep(boost::posix_time::seconds(666));
            //m_segment.atomic_func();
            try
            {
                GC();
            }
            catch (interprocess::interprocess_exception &ex)
            {
                m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch(TException& ex)
            {
                ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
                ex.LineCode() = __LINE__;
                m_own.WriteLog(ex);
            }
            catch (std::exception& ex)
            {
                m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch (...)
            {
                m_own.WriteLogFatal("Unknown_exception", __CURR_CODE_PLACE_C__);
            }
        }
    }
    void GC(void) override
    {
        if (m_isShareMem)
        {
            string sGdGC = c_RESTful_KCSession + m_name + "_Global_GC_";
            TGlobalData &gdGC = *m_segment.template find_or_construct<TGlobalData>(sGdGC.c_str())();
            posix_time::ptime absDTGC = posix_time::second_clock::local_time() + posix_time::seconds(666);
            if (ProcLockByTimeLimit([&](){return gdGC.m_mtx.timed_lock(absDTGC);}, __CURR_CODE_PLACE_C__, sGdGC, 666, "[Unique]"))
            {
                CAutoRelease _auto([&](){ GetGlobalMtx(sGdGC).unlock(); }, true);
                TGlobalData &gd = *m_segment.template find_or_construct<TGlobalData>(m_globaDataName.c_str())();
                posix_time::ptime absDT = posix_time::second_clock::local_time() + posix_time::seconds(6);
                if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_lock_upgradable(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 6, "[Upgradable]"))
                {
                    CAutoRelease _auto1([&](){ GetGlobalMtx(m_globaDataName).unlock_upgradable(); }, true);
                    for (auto it = m_mapSession.begin(); m_mapSession.end() != it;)
                    {
                        if (it->second.m_expire < posix_time::second_clock::local_time())
                        {
                            m_own.WriteLogDebug("Del Timeout Session", __CURR_CODE_PLACE_C__, (it->second.m_name + "=" + it->second.m_val).c_str());
                            absDT = posix_time::second_clock::local_time() + posix_time::seconds(16);
                            if (ProcLockByTimeLimit([&](){return gd.m_mtx.timed_unlock_upgradable_and_lock(absDT);}, __CURR_CODE_PLACE_C__, m_globaDataName, 16, ("[Unique]" + it->second.m_name).c_str()))
                            {
                                CAutoRelease _auto2([&](){ GetGlobalMtx(m_globaDataName).unlock_and_lock_upgradable(); }, true);
                                it = m_mapSession.erase(it);
                            }
                        }
                        else ++it;
                    }
                }
            }
        }
    }
    // 获取全局锁
    TMtx& GetGlobalMtx(string sName)
    {
        std::pair<TGlobalData*, unsigned> gd = m_segment.template find<TGlobalData>(sName.c_str());
        if (nullptr == gd.first)
        {
            m_own.WriteLogWarning((m_own.getHint("Don_t_exists_data_") + sName).c_str(), __CURR_CODE_PLACE_C__);
            throw 0;
        }
        return gd.first->m_mtx;
    }
    // 限时进程锁
    bool ProcLockByTimeLimit(std::function<bool(void)> fn, string pos, string sGloName, unsigned secs, string sMsg)
    {
        bool bResult = true;
        // 启动线程
        boost::thread::attributes attrs;
        attrs.set_stack_size(1024 * 1024);
        boost::thread thrd(attrs, [&](){ bResult = fn(); });
        // 如果死锁，则删除锁
        if (thrd.joinable() && !thrd.timed_join(boost::posix_time::seconds(secs + 6)))
        {
            if (thrd.joinable()) thrd.interrupt();
            m_segment.template destroy<TGlobalData>(sGloName.c_str());
            m_own.WriteLogWarning("Destroy Deadlock", pos.c_str(), ("[" + sGloName + "]" + sMsg).c_str());
            if (thrd.joinable()) thrd.timed_join(boost::posix_time::milliseconds(6666));
            bResult = false;
        }
        return bResult;
    }
};

template<typename TSegment>
CCtrlSession<TSegment>::CCtrlSession(IKCSessionCookie& own, string sName, property_tree::ptree& pt, bool isShareMem, TSegment& msm)
    : ICtrlSession(own), m_name(sName), m_isShareMem(isShareMem)
    , m_segment(msm)
    , m_alloc_inst_session(m_segment.get_segment_manager())
    , m_alloc_char(m_segment.get_segment_manager())
    , m_globaDataName(c_RESTful_KCSession + sName + "_Global_")
    , m_mapSession(
          [&]() -> session_map_type&
          {
              string sMpName = c_RESTful_KCSession + sName + "_";
              try
              {
                return *(m_segment.template find_or_construct<session_map_type>(sMpName.c_str())(/*std::less<char_string>()*/TKcNameLess<char_string>(m_nameCaseSensitive), m_alloc_inst_session));
              }
              catch (...)
              {
                own.WriteLogWarning("find_or_construct session_map_type Fail.", __CURR_CODE_PLACE_C__, sMpName.c_str());
                try
                {
                    m_segment.template destroy<session_map_type>(sMpName.c_str());
                }
                catch (...) {}
                session_map_type *pResult = nullptr;
                for (int i = 0; i < 3; ++i)
                    try
                    {
                        sMpName = c_RESTful_KCSession + sName + "_" + std::to_string(i) + "_";
                        pResult = m_segment.template construct<session_map_type>(sMpName.c_str())(/*std::less<char_string>()*/TKcNameLess<char_string>(m_nameCaseSensitive), m_alloc_inst_session);
                    }
                    catch (...) {}
                if (nullptr == pResult)
                    throw TKCSessionCookieException(ecd_ErrCode_KCSessionCookie + 11, __CURR_CODE_PLACE_C__, string(m_own.getHint("Create_Fail_")) + sMpName, own);;
                return *pResult;
              }
          }()
    )
{
    // 是否全局值
    if (pt.get_child_optional("<xmlattr>.isGlobal"))
        m_isGlobal = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.isGlobal")) == "true";
    // 超时期限
    if (pt.get_child_optional("<xmlattr>.Expire"))
    try
    {
        string sExpire = pt.get<string>("<xmlattr>.Expire");
        char unt = sExpire.back();
        if ('d' != unt && 'h' != unt && 's' != unt) throw 0;
        lexical_cast<int>(sExpire.substr(0, sExpire.size() - 1));
        m_expire = sExpire;
    }
    catch (...) {}
    // 是否区分大小写
    if (pt.get_child_optional("<xmlattr>.nameCaseSensitive"))
        m_nameCaseSensitive = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.nameCaseSensitive")) == "true";
}

