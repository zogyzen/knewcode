#include "std.h"
#include "work_db_i.h"
#include "work_db.h"
#include "type_db.h"
#include "value_db.h"
#include "parms_db.h"
#include "row_db.h"
#include "table_db.h"
#include "table_set.h"
#include "action_db.h"

////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	IKCDBWork& CALL_TYPE GetDBInf(const char* dir)
	{
	    static CKCDBWork s_DBInf(dir);
	    return s_DBInf;
	}
}

////////////////////////////////////////////////////////////////////////////////
// 数据库管理页面数据类
class CKCDBWorkStepData : public IKCRequestRespond::IReStepData
{
public:
    CKCDBWorkStepData(IActionData& act) {}
    virtual CALL_TYPE ~CKCDBWorkStepData() = default;

    // 得到名称
    virtual const char* CALL_TYPE GetName(void)
    {
        return CKCDBWorkStepData::GetNameS();
    }
    static const char* GetNameS(void)
    {
        return "kc_db-work@action|parm|table#1945/09/03";
    }

public:
    // 数据库活动列表
    typedef boost::shared_ptr<CDBAction> TDBActionPtr;
    typedef map<string, TDBActionPtr> TDBActionList;
    TDBActionList m_DBActionList;
    // 数据库参数列表
    typedef boost::shared_ptr<CDBParms> TDBParmPtr;
    typedef vector<TDBParmPtr> TDBParmList;
    TDBParmList m_DBParmList;
};

////////////////////////////////////////////////////////////////////////////////
// CKCDBAction类
CKCDBWork::CKCDBWork(string dir) : m_dir(dir)
{
    dynamic_backends::add_search_directory(dir);
}

// 获取数据库管理页面数据接口
IKCRequestRespond::IReStepData& CKCDBWork::GetKCDBWorkStepData(IActionData& act)
{
    if (!act.GetRequestRespond().ExistsReStepData(CKCDBWorkStepData::GetNameS()))
    {
        IKCRequestRespond::IReStepData* data = new CKCDBWorkStepData(act.GetActRoot());
        act.GetRequestRespond().AddReStepData(CKCDBWorkStepData::GetNameS(), data);
    }
    return act.GetRequestRespond().GetReStepData(CKCDBWorkStepData::GetNameS());
}

// 创建数据库活动（参数依次为链接名称、驱动名、连接串）
IDBAction& CKCDBWork::CreateDBAction(IActionData& act, const char* n, const char* d, const char* s)
{
    act.WriteLogDebug((string("CreateDBAction: ") + n + " [" + d + "][" + s + "]").c_str(), __FUNCTION__);
    // 创建数据库活动
    CKCDBWorkStepData& stepData = dynamic_cast<CKCDBWorkStepData&>(this->GetKCDBWorkStepData(act));
    if (stepData.m_DBActionList.end() != stepData.m_DBActionList.find(n))
        throw (string(act.getHint("Repeating_Create_DB_Action_")) + n).c_str();
    CDBAction *pDBAct = new CDBAction(act.GetActRoot(), n, d, s);
    stepData.m_DBActionList.insert(make_pair(string(n), CKCDBWorkStepData::TDBActionPtr(pDBAct)));
    return *pDBAct;
}

// 创建数据库参数
IDBParms& CKCDBWork::NewParms(IActionData& act)
{
    CKCDBWorkStepData& stepData = dynamic_cast<CKCDBWorkStepData&>(this->GetKCDBWorkStepData(act));
    CDBParms *pDBParm = new CDBParms(act.GetActRoot());
    stepData.m_DBParmList.push_back(CKCDBWorkStepData::TDBParmPtr(pDBParm));
    return *pDBParm;
}
