#pragma once

namespace KC
{
    // 互斥锁
    class CKcLockWork : public IKcLockWork
    {
    public:
        CKcLockWork(IBundleContextEx&);

        // 互斥量
        IMutex* CALL_TYPE NewMutex(void) override;
        void CALL_TYPE DelMutex(IMutex*&) override;

    private:
        // 互斥量
        class CMutex : public IMutex
        {
        public:
            CMutex(CKcLockWork&, int);
            unsigned long long CALL_TYPE MutexID(void) const override;
            bool CALL_TYPE Lock(const char*, const char*) override;
            void CALL_TYPE Unlock(const char*, const char*) override;

        private:
            CKcLockWork& m_LockWork;
            int m_timeOut = 7200;
            boost::recursive_timed_mutex m_mtx;
        };
        friend class CMutex;
        map<unsigned long long, std::shared_ptr<CMutex>> m_MutexList;
        boost::mutex m_mtx;

    private:
        IBundleContextEx& m_context;
    };
}
