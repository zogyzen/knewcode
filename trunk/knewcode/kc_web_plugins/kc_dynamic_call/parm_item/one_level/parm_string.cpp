#include "../../std.h"
#include "parm_string.h"

////////////////////////////////////////////////////////////////////////////////
// TParmString类
KC::parm_item::one_level::TParmString::TParmString(boost::any& val, TKcParmFL& dt, IFuncCallDyn::TParmRefBackPtr refBack)
    : m_refBack(refBack), m_val(val), m_dt(dt)
{
}

// 添加参数
void KC::parm_item::one_level::TParmString::AddArg(CKCFuncDyncall& vm)
{
    // 得到参数值
    string sParm = "";
    if (m_val.empty())
        sParm = "";
    else if (m_val.type() == typeid(bool))
        sParm = boost::any_cast<bool>(m_val) ? "true" : "false";
    else if (m_val.type() == typeid(int))
        sParm = lexical_cast<string>(boost::any_cast<int>(m_val));
    else if (m_val.type() == typeid(double))
        sParm = lexical_cast<string>(boost::any_cast<double>(m_val));
    else if (m_val.type() == typeid(string))
        sParm = boost::any_cast<string>(m_val);
    // 分配字符串缓冲区
    int len = sParm.size() * 2 + 1;
    m_pBuf = new char[len]{0};
    m_bufPtr = boost::shared_ptr<char>(m_pBuf);
    memcpy(m_pBuf, sParm.c_str(), len);
    // 添加参数
    void *pPtr = m_dt.GetVal<2>() ? (void*)&m_pBuf : (void*)m_pBuf;
    vm.AddArg(CKCFuncDyncall::DtPointer, pPtr);
}

// 引用参数回调
void KC::parm_item::one_level::TParmString::ParmRefBack(void)
{
    if (m_dt.GetVal<2>() && nullptr != m_refBack.get())
    {
        boost::any val = string(m_pBuf);
        m_refBack->SetValue(val);
    }
}

// 转换返回值
string KC::parm_item::one_level::TParmString::TranResult(void* val)
{
    return string((char*)val);
}
