#include "std.h"
#include "work_session.h"
#include "session_action.h"

////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	IKCSessionWork& CALL_TYPE GetWorkInf(void)
	{
	    static CKCSessionWork s_Inf;
	    return s_Inf;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Session页面数据类
class CKCSessionWorkStepData : public IKCRequestRespond::IReStepData
{
public:
    CKCSessionWorkStepData(IActionData& act, IKCSessionWork& wk) : m_session(act, wk)
    {
    }
    virtual CALL_TYPE ~CKCSessionWorkStepData() = default;

    // 得到名称
    virtual const char* CALL_TYPE GetName(void)
    {
        return CKCSessionWorkStepData::GetNameS();
    }
    static const char* GetNameS(void)
    {
        return "kc_session-work@session|other#1945/09/03";
    }

public:
    CKCSession m_session;
};

// 进程间共享内存管理
class CProcNameListWork : public IProcNameListWork
{
public:
    CProcNameListWork(void)
    {
        //this->Start();
    }
    ~CProcNameListWork(void)
    {
        //this->Stop();
    }

    TProcNameList& GetProcMemList(void) override
    {
        return m_procNameList;
    }

    void Run(void)
    {
        while (!m_end)
        {
            this_thread::sleep_for(chrono::milliseconds(m_procNameList.GetTimeOut()));
            m_procNameList.GC();
        }
    }

    void Start(void)
    {
        m_thrd = new thread(&CProcNameListWork::Run, this);
    }

    void Stop(void)
    {
        try
        {
            if (nullptr != m_thrd && m_thrd->joinable())
            {
                m_end = true;
                //pthread_cancel(m_thrd->native_handle());
                this_thread::sleep_for(chrono::milliseconds(1500));
                if (m_thrd->joinable()) m_thrd->join();
            }
            delete m_thrd;
            m_thrd = nullptr;
        }
        catch (...) {}
    }

private:
    TProcNameList m_procNameList;
    thread* m_thrd = nullptr;
    bool m_end = false;
};

////////////////////////////////////////////////////////////////////////////////
// CKCSessionWork类
CKCSessionWork::CKCSessionWork(void) : m_procMemList(new CProcNameListWork)
{
}

CKCSessionWork::~CKCSessionWork(void)
{
    delete m_procMemList;
}

// 获取Session页面数据接口
IKCRequestRespond::IReStepData& CKCSessionWork::GetKCSessionWorkStepData(IActionData& act)
{
    if (!act.GetRequestRespond().ExistsReStepData(CKCSessionWorkStepData::GetNameS()))
    {
        IKCRequestRespond::IReStepData* data = new CKCSessionWorkStepData(act, *this);
        act.GetRequestRespond().AddReStepData(CKCSessionWorkStepData::GetNameS(), data);
    }
    return act.GetRequestRespond().GetReStepData(CKCSessionWorkStepData::GetNameS());
}

// 获取Session接口
IKCSession& CKCSessionWork::GetSession(IActionData& act)
{
    CKCSessionWorkStepData& stepData = dynamic_cast<CKCSessionWorkStepData&>(this->GetKCSessionWorkStepData(act.GetActRoot()));
    stepData.m_session.ClearUp();
    return stepData.m_session;
}

//进程间共享内存
IProcNameListWork& CKCSessionWork::GetProcMemListWork(void)
{
    return *m_procMemList;
}
