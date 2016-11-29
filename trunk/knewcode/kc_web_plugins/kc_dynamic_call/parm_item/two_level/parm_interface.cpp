#include "../../std.h"
#include "parm_interface.h"

////////////////////////////////////////////////////////////////////////////////
// TParmInterfacer类
KC::parm_item::two_level::TParmInterfacer::TParmInterfacer(boost::any& val, TKcParmFL& dt, IFuncCallDyn::TParmRefBackPtr refBack)
    : m_refBack(refBack), m_val(val), m_dt(dt)
{
}

// 添加参数
void KC::parm_item::two_level::TParmInterfacer::AddArg(CKCFuncDyncall& vm)
{
    // 得到参数值
    if (!m_val.empty() && m_val.type() == typeid(TKcWebInfVal))
        m_value = boost::any_cast<TKcWebInfVal>(m_val);
    else
        throw "Is_not_an_interface_type";
    if (m_dt.GetVal<2>())
        vm.AddArg(CKCFuncDyncall::DtPointer, (void*)&m_value.pointer);
    else
        vm.AddArg(CKCFuncDyncall::DtInterface, (void*)(CKCFuncDyncall::TObjectPointer)m_value.pointer);
}

// 引用参数回调
void KC::parm_item::two_level::TParmInterfacer::ParmRefBack(void)
{
    if (m_dt.GetVal<2>() && nullptr != m_refBack.get())
    {
        m_value.name = m_dt.GetVal<1>().GetVal<0>();
        boost::any val = m_value;
        m_refBack->SetValue(val);
    }
}

// 转换返回值
boost::any KC::parm_item::two_level::TParmInterfacer::TranResult(void* val, string name)
{
    return TKcWebInfVal(name, (TObjectPointer)val);
}
