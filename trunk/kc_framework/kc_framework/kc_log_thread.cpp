#include "std.h"
#include "kc_log_thread.h"
#include "kc_log.h"

//---------------------------------------------------------------------------
// CLogThread类
CLogThread::CLogThread(IBundleContext& bc) : m_context(bc), m_mtx(m_context.LockWork())
{
}
CLogThread::~CLogThread(void)
{
    this->Stop();
}

void CLogThread::WriteLog(TLogInfo& log)
{
    log::trivial::severity_level lv = log::trivial::info;
    switch (log.m_level)
    {
    case TLogInfo::lglvTrace:
        lv = log::trivial::trace;
        break;
    case TLogInfo::lglvDebug:
        lv = log::trivial::debug;
        break;
    case TLogInfo::lglvInfo:
        lv = log::trivial::info;
        break;
    case TLogInfo::lglvWarning:
        lv = log::trivial::warning;
        break;
    case TLogInfo::lglvError:
        lv = log::trivial::error;
        break;
    case TLogInfo::lglvFatal:
        lv = log::trivial::fatal;
        break;
    }
    KC_WRITE_LOG(lv)
                << " \t&\t " << log.m_info
                << " \t\t@ " << log.m_place
                << " \t&\t " << log.m_other
                << " \t&\t " << " [" << log.m_ProcessID << ":" << log.m_threadID << "] - [" << CProcMemInfo::Get() << "]"
				<< " \t&\t " << log.m_excpType
				<< " \t&\t " << log.m_excpID
				<< " \t&\t " << log.m_bundleName
				<< " \t&\t " << log.m_serviceName
				<< " \t&\t " << log.m_serviceGUID
				<< " \t&\t " << log.m_refID
				<< endl;
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
        for (TLogInfo& log: logList)
            this->WriteLog(log);
    return bResult;
}

void CLogThread::Start(void)
{
    m_thrd = new thread(&CLogThread::Run, this);
}

void CLogThread::Stop(void)
{
    if (nullptr != m_thrd && m_thrd->joinable())
    {
        m_end = true;
        pthread_cancel(m_thrd->native_handle());
        m_thrd->join();
    }
    delete m_thrd;
    m_thrd = nullptr;
    this->WriteLogList();
}

void CLogThread::AppendLog(TLogInfo& log)
{
    CKcLock lck(m_mtx);
    if (nullptr != m_thrd) m_logList.push_back(log);
    else this->WriteLog(log);
}

void CLogThread::Run(void)
{
    while (!m_end)
        if (this->WriteLogList())
            this_thread::sleep_for(chrono::milliseconds(2000));
}
