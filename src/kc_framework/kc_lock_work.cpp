#include "std.h"
#include "kc_lock_work.h"
#include "bundle_context.h"

////////////////////////////////////////////////////////////////////////////////
// CKcLockWork类
CKcLockWork::CKcLockWork(IBundleContextEx& cn) : m_context(cn)
{
}

// 互斥量
IKcLockWork::IMutex* CKcLockWork::NewMutex(void)
{
    m_mtx.lock();
    CAutoRelease _autoB(boost::bind(&boost::mutex::unlock, &this->m_mtx));
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
        CAutoRelease _autoB(boost::bind(&boost::mutex::unlock, &this->m_mtx));
        auto iter = m_MutexList.find(reinterpret_cast<unsigned long long>(pMtx));
        if (m_MutexList.end() != iter) m_MutexList.erase(iter);
        pMtx = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// 线程锁列表
extern CBundleContext *g_context;
typedef multimap<unsigned long long, IKcLockWork::IMutex*> TMtxLst;
thread_local static TMtxLst *g_MtxLst = new TMtxLst;
struct CThrdMtxLst
{
    typedef multimap<unsigned long long, IKcLockWork::IMutex*> TMtxLst;

    ~CThrdMtxLst(void)
    {
        if (nullptr != g_MtxLst && !g_MtxLst->empty())
        {
            string sMtxLst = "/";
            TMtxLst mtxLs;
            g_MtxLst->swap(mtxLs);
            for (auto v : mtxLs)
            {
                try
                {
                    sMtxLst += std::to_string(v.first) + "/";
                    v.second->Unlock(nullptr, nullptr);
                }
                catch (...) {}
            }
            string sInfo = "Deadlock - " + sMtxLst;
            if (nullptr != g_context)
                g_context->WriteLogFatal(sInfo.c_str(), __CURR_CODE_PLACE_C__);
            else
                CTempLog::Write(sInfo, __CURR_CODE_PLACE_C__);
            exit(CUtilFunc::CurrThreadID());
        }
        else if (nullptr != g_MtxLst)
        {
            delete g_MtxLst;
            g_MtxLst = nullptr;
        }
    };

    TMtxLst& MtxLst(void)
    {
        return *g_MtxLst;
    }
};
thread_local static CThrdMtxLst g_ThrdMtxLst;

////////////////////////////////////////////////////////////////////////////////
// CKcLockWork::CMutex类
CKcLockWork::CMutex::CMutex(CKcLockWork& lw, int to) : m_LockWork(lw), m_timeOut(to)
{
}

unsigned long long CKcLockWork::CMutex::MutexID(void) const
{
    return reinterpret_cast<unsigned long long>(this);
}


bool CKcLockWork::CMutex::Lock(const char* info, const char* pos)
{
    bool bResult = false;
    if (nullptr != this)
    {
        if (nullptr != info && m_LockWork.m_context.GetCfgLogLevel() <= (int)IFuncLog::lglvDebug)
            m_LockWork.m_context.WriteLogDebug(("Lock[" + std::to_string(this->MutexID()) + "] - " + info).c_str(), pos,
                                std::to_string(m_timeOut).c_str());
        bResult = m_mtx.try_lock_for(boost::chrono::milliseconds(m_timeOut));
        if (!bResult)
        {
            if (nullptr != info)
                m_LockWork.m_context.WriteLogFatal(("Lock[" + std::to_string(this->MutexID()) + "] Failure, Process Exit - " + info).c_str(), pos,
                                ("timeout: " + std::to_string(m_timeOut)).c_str());
            else
                CTempLog::Write("Lock[" + std::to_string(this->MutexID()) + "] Failure, Process Exit - " + std::to_string(m_timeOut), __CURR_CODE_PLACE_C__);
            exit(CUtilFunc::CurrThreadID());
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
        if (nullptr != info && m_LockWork.m_context.GetCfgLogLevel() <= (int)IFuncLog::lglvDebug)
            m_LockWork.m_context.WriteLogDebug(("UnLock[" + std::to_string(this->MutexID()) + "] - " + info).c_str(), pos,
                                std::to_string(m_timeOut).c_str());
    }
}
