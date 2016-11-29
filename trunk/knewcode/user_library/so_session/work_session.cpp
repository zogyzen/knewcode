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
// Sessionҳ��������
class CKCSessionWorkStepData : public IKCRequestRespond::IReStepData
{
public:
    CKCSessionWorkStepData(IActionData& act, IKCSessionWork& wk) : m_session(act, wk)
    {
    }
    virtual CALL_TYPE ~CKCSessionWorkStepData() = default;

    // �õ�����
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

// ���̼乲���ڴ����
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
// CKCSessionWork��
CKCSessionWork::CKCSessionWork(void) : m_procMemList(new CProcNameListWork)
{
}

CKCSessionWork::~CKCSessionWork(void)
{
    delete m_procMemList;
}

// ��ȡSessionҳ�����ݽӿ�
IKCRequestRespond::IReStepData& CKCSessionWork::GetKCSessionWorkStepData(IActionData& act)
{
    if (!act.GetRequestRespond().ExistsReStepData(CKCSessionWorkStepData::GetNameS()))
    {
        IKCRequestRespond::IReStepData* data = new CKCSessionWorkStepData(act, *this);
        act.GetRequestRespond().AddReStepData(CKCSessionWorkStepData::GetNameS(), data);
    }
    return act.GetRequestRespond().GetReStepData(CKCSessionWorkStepData::GetNameS());
}

// ��ȡSession�ӿ�
IKCSession& CKCSessionWork::GetSession(IActionData& act)
{
    CKCSessionWorkStepData& stepData = dynamic_cast<CKCSessionWorkStepData&>(this->GetKCSessionWorkStepData(act.GetActRoot()));
    stepData.m_session.ClearUp();
    return stepData.m_session;
}

//���̼乲���ڴ�
IProcNameListWork& CKCSessionWork::GetProcMemListWork(void)
{
    return *m_procMemList;
}
