#include "func_log.h"

CBoostLog *g_ctrl = nullptr;
extern "C"
{
    IFuncLog& CALL_TYPE create(IBundleContext& own)
    {
        if (nullptr == g_ctrl) g_ctrl = new CBoostLog(own);
        return *g_ctrl;
    }
    void CALL_TYPE destroy(IFuncLog&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// CBoostLog 类
CBoostLog::CBoostLog(IBundleContext& own) : m_own(own), m_path(own.getPath())
{
}

// 初始化
const char* CBoostLog::Init(const char* dir, const char* id, TLogLevel lv, int szFile, const char* eachPrefix, bool inDay)
{
    static thread_local string sResult;
    sResult.clear();
    try
    {
        m_log.Init(dir, id, static_cast<boost::log::trivial::severity_level>(lv), szFile, eachPrefix, inDay);
    }
    catch (std::exception &ex)
    {
        sResult = (boost::format("*[knewcode] CFuncLog::Init Failed <%s> %s (%s)") % typeid(ex).name() % ex.what() % __CURR_CODE_PLACE__).str();
    }
    catch (...)
    {
        sResult = "*[knewcode] CFuncLog::Init Failed. (" + __CURR_CODE_PLACE__ + ")";
    }
    return sResult.c_str();
}

// 释放
void CBoostLog::Free(void)
{
    m_log.Free();
}

// 写日志
bool CBoostLog::WriteLog(TLogLevel lv, const char* msg) const
{
    bool bResult = false;
    try
    {
        switch (lv)
        {
        case lglvTrace:
            BOOST_LOG_TRIVIAL(trace) << msg;
            break;
        case lglvDebug:
            BOOST_LOG_TRIVIAL(debug) << msg;
            break;
        case lglvInfo:
            BOOST_LOG_TRIVIAL(info) << msg;
            break;
        case lglvWarning:
            BOOST_LOG_TRIVIAL(warning) << msg;
            break;
        case lglvError:
            BOOST_LOG_TRIVIAL(error) << msg;
            break;
        case lglvFatal:
            BOOST_LOG_TRIVIAL(fatal) << msg;
            break;
        default:
            throw std::runtime_error("Log Level Error - " + std::to_string(lv));
        }
        bResult = true;
    }
    catch (std::exception &ex)
    {
        cout << "*[knewcode] CFuncLog::WriteLogTrace Failed <" << typeid(ex).name() << ">: " << ex.what() << "\n\t" <<  msg << endl;
    }
    catch (...)
    {
        cout << "*[knewcode] CFuncLog::WriteLogTrace Failed \n\t" << msg << endl;
    }
    return bResult;
}

// 提交
void CBoostLog::Flush(void) const
{
    m_log.Flush();
}
