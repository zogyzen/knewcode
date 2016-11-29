#pragma once

namespace KC
{
    class CLogThread
    {
    public:
        CLogThread(IBundleContext&);
        ~CLogThread(void);

        void Start(void);
        void Stop(void);

        void AppendLog(TLogInfo&);

    private:
        void WriteLog(TLogInfo&);
        bool WriteLogList(void);
        void Run(void);

    private:
        IBundleContext& m_context;
        vector<TLogInfo> m_logList;
        CKcMutex m_mtx;
        bool m_end = false;
        thread* m_thrd = nullptr;
    };
}
