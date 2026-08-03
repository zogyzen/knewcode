#include "kc_log_thread.h"

//---------------------------------------------------------------------------
// CLogThread类
CLogThread::CLogThread(IBundleContext& bc) : m_context(bc), m_mtx(m_context.LockWork())
{
}
CLogThread::~CLogThread(void)
{
    BreakRun();
}

void CLogThread::WriteLog(TLogInfo& log)
{
    try
    {
        string sEmoji = "  \t", sBacktrace = "";
        if (strcmp(m_context.getSelLang(), "cn") == 0)
        {
            if (IFuncLog::lglvWarning == log.m_level) sEmoji = "⚠ \t";
            else if (IFuncLog::lglvError == log.m_level) sEmoji = "🚨 \t";
            else if (IFuncLog::lglvFatal == log.m_level)
            {
                sEmoji = "💥 \t";
                sBacktrace = CBacktraceSymbols::Get();
            }
        }
        string sMsg = (boost::format("%s %d:%d \t[%s | %s]\n\t\t =>>  \t%s - %s  \t<<= \n%s\r\n  \t\t.&\t %s \t&\t %d %s \t&\t %s \t&\t %s \t&\t %d  \r\n%s\n%s\n")
                       % sEmoji % log.m_ProcessID % log.m_threadID % CProcMemInfo::Get() % SystemRunStatus::GetSysRunInfo()
                       % log.m_place % log.m_lineCode % log.m_info % log.m_excpType % log.m_excpID
                       % log.m_bundleName % log.m_serviceName % log.m_serviceGUID % log.m_refID
                       % log.m_other % sBacktrace
                ).str();
        m_logLib.Inf().WriteLog(log.m_level, sMsg.c_str());
    }
    catch (...) {}
}

bool CLogThread::WriteLogList(void)
{
    vector<TLogInfo> logList;
    {
        CKcLock lck(m_mtx);
        if (!m_logList.empty()) m_logList.swap(logList);
    }
    bool bResult = logList.empty();
    if (!bResult)
        for (TLogInfo& lg: logList)
            this->WriteLog(lg);
    return bResult;
}

void CLogThread::RunThd(void)
{
    while (!m_end)
        if (this->WriteLogList())
            boost::this_thread::sleep_for(boost::chrono::milliseconds(2000));
}

void CLogThread::BreakRun(void)
{
    if (nullptr != m_thrd && m_thrd->joinable())
    {
        //pthread_cancel(m_thrd->native_handle());
        m_thrd->interrupt();
        //m_thrd->join();
        m_thrd->timed_join(1000);
    }
    if (nullptr != m_thrd)
    {
        delete m_thrd;
        m_thrd = nullptr;
        this->WriteLogList();
    }
}

void CLogThread::Start(string sOwnName)
{
    // 日志等级
    string sLv = m_context.GetCfgInfo("Config.Parameters.logging", "Level", "info");
    if ("trace" == sLv) m_logLevel = IFuncLog::lglvTrace;
    else if ("debug" == sLv) m_logLevel = IFuncLog::lglvDebug;
    else if ("info" == sLv) m_logLevel = IFuncLog::lglvInfo;
    else if ("warning" == sLv) m_logLevel = IFuncLog::lglvWarning;
    else if ("error" == sLv) m_logLevel = IFuncLog::lglvError;
    else if ("fatal" == sLv) m_logLevel = IFuncLog::lglvFatal;
    // 日志库
    string sLogLib = m_context.GetCfgInfo("Config.Parameters.logging", c_RESTful_so);
    if (sLogLib.empty()) sLogLib = m_context.GetCfgInfo("Config.Parameters.logging", "lib", ">so_log_boost");
    sLogLib = m_context.transCfgPathToFullPath(sLogLib.c_str());
    // 日志文件大小
    int logFileSize = atoi(m_context.GetCfgInfo("Config.Parameters.logging", "fileSize", "2"));
    if (logFileSize <= 0) logFileSize = 2;
    // 系统标识
    //boost::posix_time::time_duration dtNow = boost::posix_time::second_clock::local_time().time_of_day();
    //string sLogID = (format("%02d%02d%02d") % dtNow.hours() % dtNow.minutes() % dtNow.seconds()).str();
    string sSysFlag = m_context.GetCfgInfo("Config.Parameters.sys_flag", "customer", "my") + string("_") + sOwnName;
    // 日志目录
    string sLogPth = m_context.GetCfgInfo("Config.Parameters.logging", "dir", c_LogDirectoryName);
    sLogPth = m_context.transCfgPathToFullPath(sLogPth.c_str());
    boost::gregorian::date dtToday = boost::gregorian::day_clock::local_day();
    int iYear = dtToday.year();
    string sLogDir = (format("%s/%s/%d/%d%02d%02d") % sLogPth % CUtilFunc::KcVersionForPathName() % iYear % iYear % static_cast<int>(dtToday.month()) % dtToday.day()).str();
    cout << "*[knewcode] CLogThread::Start (" << sLv << "): " << sLogDir << endl;
    // 初始化日志
    try
    {
        // string sErr = m_logLib.Load(m_context, string() + m_context.getPath() + "/" + sLogLib);
        string sErr = m_logLib.Load(m_context, sLogLib);
        if (!sErr.empty()) throw std::runtime_error(sErr);
        sErr = m_logLib.Inf().Init(sLogDir.c_str(), sSysFlag.c_str(), m_logLevel, logFileSize, m_context.getHint("EachPrefix"));
        if (!sErr.empty()) throw std::runtime_error(sErr + " (" + __CURR_CODE_FLINE__ + ")");
    }
    catch (std::exception &ex)
    {
        string sExcept = (boost::format("*[knewcode] %s CLogThread::Start Failed \n<%s> %s") % sLogLib % typeid(ex).name() % ex.what()).str();
        cout << sExcept << endl;
        CTempLog::WriteInFile(sLogDir + "/error.log", sExcept, __CURR_CODE_PLACE_C__);
        throw;
    }
    catch (...)
    {
        string sExcept = "*[knewcode] " + sLogLib + " CLogThread::Start Failed.";
        cout << sExcept << endl;
        CTempLog::WriteInFile(sLogDir + "/error.log", sExcept, __CURR_CODE_PLACE_C__);
        throw;
    }
    // 启动线程
    //if (string(m_context.GetCfgInfo("Config.Parameters.log_use_thread", "value", "0")) == "1")
    //    m_thrd = new boost::thread(&CLogThread::RunThd, this);
}

void CLogThread::Stop(void)
{
    m_end = true;
    BreakRun();
    m_logLib.Inf().Free();
}

void CLogThread::AppendLog(TLogInfo& log)
{
    if (nullptr != m_thrd)
    {
        CKcLock lck(m_mtx);
        m_logList.push_back(log);
    }
    else this->WriteLog(log);
}

IFuncLog::TLogLevel CLogThread::GetCfgLogLevel(void) const
{
    return m_logLevel;
}
