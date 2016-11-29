#include "../../std.h"
#include "parm_double.h"

////////////////////////////////////////////////////////////////////////////////
// TParmDouble类
KC::parm_item::one_level::TParmDouble::TParmDouble(boost::any& val, TKcParmFL& dt, IFuncCallDyn::TParmRefBackPtr refBack)
    : m_refBack(refBack), m_val(val), m_dt(dt)
{
}

// 添加参数
void KC::parm_item::one_level::TParmDouble::AddArg(CKCFuncDyncall& vm)
{
    // 得到参数值
    if (m_val.empty())
        m_value = 0;
    else if (m_val.type() == typeid(bool))
        m_value = boost::any_cast<bool>(m_val) ? 1 : 0;
    else if (m_val.type() == typeid(int))
        m_value = boost::any_cast<int>(m_val);
    else if (m_val.type() == typeid(double))
        m_value = boost::any_cast<double>(m_val);
    else if (m_val.type() == typeid(string))
        m_value = lexical_cast<double>(boost::trim_copy(boost::any_cast<string>(m_val)));
    // 添加实参
    if (m_dt.GetVal<2>())
        vm.AddArg(CKCFuncDyncall::DtPointer, (void*)&m_value);
    else
        vm.AddArg(CKCFuncDyncall::DtDouble, m_value);
}

// 引用参数回调
void KC::parm_item::one_level::TParmDouble::ParmRefBack(void)
{
    if (m_dt.GetVal<2>() && nullptr != m_refBack.get())
    {
        boost::any val = m_value;
        m_refBack->SetValue(val);
    }
}
