#pragma once

#include "std.h"

namespace KC
{
    class CLogThread
    {
    public:
        CLogThread(IBundleContext&);
        ~CLogThread(void);

        void Start(string);
        void Stop(void);

        void AppendLog(TLogInfo&);
        IFuncLog::TLogLevel GetCfgLogLevel(void) const;

    private:
        void WriteLog(TLogInfo&);
        bool WriteLogList(void);
        void RunThd(void);
        void BreakRun(void);

    private:
        IBundleContext& m_context;
        vector<TLogInfo> m_logList;
        CKcMutex m_mtx;
        bool m_end = false;
        boost::thread* m_thrd = nullptr;
        // 日志等级
        IFuncLog::TLogLevel m_logLevel = IFuncLog::TLogLevel::lglvInfo;
        // 日志库
        TSubModule<IFuncLog> m_logLib;
    };
}
