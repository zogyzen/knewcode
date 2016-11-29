#include "std.h"
#include "work_util.h"
#include "action_algorithm.h"

////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	IKCUtilWork& CALL_TYPE GetWorkInf(int i)
	{
	    static CKCUtilWork s_Inf;
	    return s_Inf;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 公共功能页面数据类
class CKCUtilStepData : public IKCRequestRespond::IReStepData
{
public:
    CKCUtilStepData(IActionData& act, CKCUtilWork& wk) : m_algorithm(act)
    {
    }
    virtual CALL_TYPE ~CKCUtilStepData() = default;

    // 得到名称
    virtual const char* CALL_TYPE GetName(void)
    {
        return CKCUtilStepData::GetNameS();
    }
    static const char* GetNameS(void)
    {
        return "kc_util-algorithm@util|algorithm#1945/09/03";
    }

public:
    CKCAlgorithmAction m_algorithm;
};

////////////////////////////////////////////////////////////////////////////////
// CKCUtilWork类
CKCUtilWork::CKCUtilWork(void)
{
}

// 获取公共功能页面数据接口
IKCRequestRespond::IReStepData& CKCUtilWork::GetKCSessionWorkStepData(IActionData& act)
{
    if (!act.GetRequestRespond().ExistsReStepData(CKCUtilStepData::GetNameS()))
    {
        IKCRequestRespond::IReStepData* data = new CKCUtilStepData(act, *this);
        act.GetRequestRespond().AddReStepData(CKCUtilStepData::GetNameS(), data);
    }
    return act.GetRequestRespond().GetReStepData(CKCUtilStepData::GetNameS());
}


// 创建公共功能活动接口
IKCAlgorithmAction& CKCUtilWork::GetAlgorithmAction(IActionData& act)
{
    CKCUtilStepData& stepData = dynamic_cast<CKCUtilStepData&>(this->GetKCSessionWorkStepData(act));
    return stepData.m_algorithm;
}
