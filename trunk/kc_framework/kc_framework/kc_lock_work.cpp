#include "std.h"
#include "kc_lock_work.h"
#include "bundle_context.h"

////////////////////////////////////////////////////////////////////////////////
// CSignoCatch类
CKcLockWork::CKcLockWork(IBundleContextEx& cn) : m_context(cn)
{
}

// 互斥量
IKcLockWork::IMutex* CKcLockWork::NewMutex(void)
{
    m_mtx.lock();
    CAutoRelease _autoB(boost::bind(&mutex::unlock, &this->m_mtx));
    CMutex *pMtx = new CMutex(*this, dynamic_cast<CBundleContext&>(m_context).GetTimeOutSeconds());
    std::shared_ptr<CMutex> MtxPtr(pMtx);
    m_MutexList.insert(make_pair(pMtx->MutexID(), MtxPtr));
    return pMtx;
}

void CKcLockWork::DelMutex(IMutex*& pMtx)
{
    if (nullptr != pMtx)
    {
        m_mtx.lock();
        CAutoRelease _autoB(boost::bind(&mutex::unlock, &this->m_mtx));
        auto iter = m_MutexList.find(reinterpret_cast<int>(pMtx));
        if (m_MutexList.end() != iter) m_MutexList.erase(iter);
        pMtx = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// 线程锁列表
extern CBundleContext *g_context;
struct CThrdMtxLst
{
    typedef multimap<int, IKcLockWork::IMutex*> TMtxLst;

    ~CThrdMtxLst(void)
    {
        if (nullptr != m_MtxLst && !m_MtxLst->empty())
        {
            string sMtxLst = "/";
            TMtxLst mtxLs;
            m_MtxLst->swap(mtxLs);
            for (auto v: mtxLs)
                try
                {
                    sMtxLst += lexical_cast<string>(v.first) + "/";
                    v.second->Unlock(nullptr, nullptr);
                }
                catch (...) {}
            string sInfo = "Deadlock - " + sMtxLst;
            if (nullptr != g_context)
                g_context->WriteLogFatal(sInfo.c_str(), __FUNCTION__);
            else
                CTempLog::Write(sInfo, __FUNCTION__);
            delete m_MtxLst;
            m_MtxLst = nullptr;
            exit(pthread_self());
        }
    };

    TMtxLst& MtxLst(void)
    {
        return *m_MtxLst;
    }

private:
    TMtxLst *m_MtxLst = new TMtxLst;
};
thread_local static CThrdMtxLst g_ThrdMtxLst;

////////////////////////////////////////////////////////////////////////////////
// CSignoCatch::CMutex类
CKcLockWork::CMutex::CMutex(CKcLockWork& lw, int to) : m_LockWork(lw), m_timeOut(to)
{
}

unsigned int CKcLockWork::CMutex::MutexID(void) const
{
    return reinterpret_cast<unsigned int>(this);
}


bool CKcLockWork::CMutex::Lock(const char* info, const char* pos)
{
    bool bResult = false;
    if (nullptr != this)
    {
        if (nullptr != info && m_LockWork.m_context.GetCfgLogLevel() <= (int)log::trivial::debug)
            m_LockWork.m_context.WriteLogDebug(("Lock[" + lexical_cast<string>(this->MutexID()) + "] - " + info).c_str(), pos,
                                lexical_cast<string>(m_timeOut).c_str());
        bResult = m_mtx.try_lock_for(chrono::milliseconds(m_timeOut));
        if (!bResult)
        {
            if (nullptr != info)
                m_LockWork.m_context.WriteLogFatal(("Lock[" + lexical_cast<string>(this->MutexID()) + "] Failure, Process Exit - " + info).c_str(), pos,
                                ("timeout: " + lexical_cast<string>(m_timeOut)).c_str());
            else
                CTempLog::Write("Lock[" + lexical_cast<string>(this->MutexID()) + "] Failure, Process Exit - " + lexical_cast<string>(m_timeOut), __FUNCTION__);
            exit(pthread_self());
        }
        g_ThrdMtxLst.MtxLst().insert(make_pair(this->MutexID(), this));
    }
    return bResult;
}

void CKcLockWork::CMutex::Unlock(const char* info, const char* pos)
{
    if (nullptr != this)
    {
        auto iter = g_ThrdMtxLst.MtxLst().find(this->MutexID());
        if (g_ThrdMtxLst.MtxLst().end() != iter) g_ThrdMtxLst.MtxLst().erase(iter);
        m_mtx.unlock();
        if (nullptr != info && m_LockWork.m_context.GetCfgLogLevel() <= (int)log::trivial::debug)
            m_LockWork.m_context.WriteLogDebug(("UnLock[" + lexical_cast<string>(this->MutexID()) + "] - " + info).c_str(), pos,
                                lexical_cast<string>(m_timeOut).c_str());
    }
}
