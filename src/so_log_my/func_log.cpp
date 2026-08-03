#include "func_log.h"

CMyLog *g_ctrl = nullptr;
extern "C"
{
    IFuncLog& CALL_TYPE create(IBundleContext& own)
    {
        if (nullptr == g_ctrl)
        {
            // ExceptBacktrace::SetExceptFilter(own.getPath());
            g_ctrl = new CMyLog(own);
        }
        return *g_ctrl;
    }
    void CALL_TYPE destroy(IFuncLog&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// CMyLog 类
CMyLog::CMyLog(IBundleContext& own) : m_own(own), m_modPath(own.getPath())
{
}
std::atomic_bool CMyLog::s_running(false);

// 初始化
const char* CMyLog::Init(const char* dir, const char* id, TLogLevel lv, int szFile, const char* eachPrefix, bool inDay)
{
    static thread_local string sResult;
    sResult.clear();
    try
    {
        std::cout << "*[knewcode] CMyLog::Init Begin (" << lv << "): " << dir << std::endl;
        // 参数
        m_logPath = CUtilFunc::PCharSafeToStr(dir);
        if (m_logPath.empty()) m_logPath = m_modPath + "/logs";
        m_logID = CUtilFunc::PCharSafeToStr(id);
        m_logEachPrefix = CUtilFunc::PCharSafeToStr(eachPrefix);
        m_logLV = lv;
        m_logSizeFile = szFile;
        m_logInDay = inDay;
        // 日志目录
        if (!boost::filesystem::exists(m_logPath)) boost::filesystem::create_directories(m_logPath);
        // 开始线程
        s_running = true;
        m_runThrd.reset(new boost::thread(&CMyLog::RunThrd, this));
    }
    catch (std::exception &ex)
    {
        sResult = (boost::format("*[knewcode] CFuncLog::Init Failed <%s> %s") % typeid(ex).name() % ex.what()).str();
    }
    catch (...)
    {
        sResult = "*[knewcode] CFuncLog::Init Failed.";
    }
    return sResult.c_str();
}

// 释放
void CMyLog::Free(void)
{
    try
    {
        // 终止线程
        s_running = false;
        if (m_runThrd.get() != nullptr && m_runThrd->joinable()) m_runThrd->interrupt();
        m_runThrd.reset();
    }
    catch (...) {}
}

// 写日志
bool CMyLog::WriteLog(TLogLevel lv, const char* msg) const
{
    bool bResult = false;
    try
    {
        TLogDataPtr logd(new TLogData(lv, msg));
        boost::unique_lock<boost::mutex> lck(m_mtxLog);
        m_logList.push_back(logd);
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
void CMyLog::Flush(void) const
{
    WriteLogToFile();
}

// 线程
void CMyLog::RunThrd(void)
{
    // 刷新间隔
    string s = m_own.GetCfgInfo("Config.Parameters.logging", "flushMS", "3666");
    const int flushMS = std::max(atoi(m_own.GetCfgInfo("Config.Parameters.logging", "flushMS", "3666")), 666);
    boost::this_thread::sleep(boost::posix_time::milliseconds(999));
    cout << "\t" << typeid(*this).name() << ": Start Write Log Thread." << endl;
    // 循环
    while (s_running)
    try
    {
        boost::this_thread::interruption_point();
        WriteLogToFile();
        boost::this_thread::sleep(boost::posix_time::milliseconds(flushMS));
    }
    catch (...) {}
}
// 将日志写入文件
void CMyLog::WriteLogToFile(void) const
{
    // 日志列表
    TLogDataList logList;
    {
        boost::unique_lock<boost::mutex> lck(m_mtxLog);
        logList.swap(m_logList);
    }
    // 写日志
    if (!logList.empty())
    {
        boost::unique_lock<boost::mutex> lck(m_mtxWrFile);
        // 整合日志内容
        string sContent;
        for (TLogDataPtr logPtr : logList)
            sContent += (boost::format("%s[%s @%d:%d *%s] %s\n")
                         % m_logEachPrefix % CUtilFunc::TimeToStr(logPtr->m_dt)
                         % getpid() % logPtr->m_thrdID % logPtr->LogLvToStr()
                         % logPtr->m_msg
            ).str();
        // 日志文件名
        string sLogFile = (boost::format("%s/%s_%s_%d.log") % m_logPath % m_logID % CUtilFunc::TimeToStr(boost::posix_time::microsec_clock::local_time(), "%Y-%m-%d-%H") % getpid()).str();
        // 写入文件
        std::ofstream flog(sLogFile, std::ios::out | std::ios::app);
        CAutoRelease _auto(boost::bind(&std::ofstream::close, &flog));
        flog << sContent << std::endl;
        cout << "\t" << typeid(*this).name() << ": Write Log Count - " << logList.size() << endl;
    }
}
