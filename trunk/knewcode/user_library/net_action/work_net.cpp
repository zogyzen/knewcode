#include "std.h"
#include "work_net.h"
#include "net_action.h"

////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	IKCNetWork& CALL_TYPE GetWorkInf(int i)
	{
	    static CKCNetWork s_Inf;
	    return s_Inf;
	}
}

////////////////////////////////////////////////////////////////////////////////
// ���繦��ҳ��������
class CKCNetStepData : public IKCRequestRespond::IReStepData
{
public:
    CKCNetStepData(IActionData& act, CKCNetWork& wk, asio::io_service& ns) : m_net(act, ns)
    {
    }
    virtual CALL_TYPE ~CKCNetStepData() = default;

    // �õ�����
    virtual const char* CALL_TYPE GetName(void)
    {
        return CKCNetStepData::GetNameS();
    }
    static const char* GetNameS(void)
    {
        return "kc_net-work@net|action#1945/09/03";
    }

public:
    CKCNetAction m_net;
};

////////////////////////////////////////////////////////////////////////////////
// CKCNetWork��
CKCNetWork::CKCNetWork(void)
{
}

// ��ȡ��������ҳ�����ݽӿ�
IKCRequestRespond::IReStepData& CKCNetWork::GetKCSessionWorkStepData(IActionData& act)
{
    if (!act.GetRequestRespond().ExistsReStepData(CKCNetStepData::GetNameS()))
    {
        IKCRequestRespond::IReStepData* data = new CKCNetStepData(act, *this, m_io_serv);
        act.GetRequestRespond().AddReStepData(CKCNetStepData::GetNameS(), data);
    }
    return act.GetRequestRespond().GetReStepData(CKCNetStepData::GetNameS());
}


// �����������ܻ�ӿ�
IKCNetAction& CKCNetWork::GetNetAction(IActionData& act)
{
    CKCNetStepData& stepData = dynamic_cast<CKCNetStepData&>(this->GetKCSessionWorkStepData(act));
    return stepData.m_net;
}
