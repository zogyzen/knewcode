#include "../std.h"
#include "exec_item.h"

////////////////////////////////////////////////////////////////////////////////
// CExecItem类
KC::one_level::CExecItem::CExecItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcExecSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CExecItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordExecute;
}

// 语法类型转换
const TKcExecSent& KC::one_level::CExecItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcExecSent&>(syn);
}

// 处理页数据
void KC::one_level::CExecItem::Action(IKCActionData& act)
{
    boost::any val;
    // 条件
    bool bCond = true;
    if (!m_syn.GetVal<1>().IsNullptr())
    {
        bCond = this->GetSynCondValue(act, m_syn);
        val = act.GetExprValue(m_syn.GetVal<1>());
    }
    else
        val = act.GetExprValue(m_syn.GetVal<0>());
    // 条件为真输出
    if (bCond)
    {
        // 值
        string sExec;
        if (val.type() == typeid(int)) sExec = lexical_cast<string>(boost::any_cast<int>(val));
        else if (val.type() == typeid(double)) sExec = lexical_cast<string>(boost::any_cast<double>(val));
        else if (val.type() == typeid(string)) sExec = boost::any_cast<string>(val);
        else if (val.type() == typeid(bool)) sExec = boost::any_cast<bool>(val) ? "true" : "false";
        else if (val.type() == typeid(TKcWebInfVal)) sExec = boost::any_cast<TKcWebInfVal>(val).str();
        act.ParseActionScript("kc_web_page_data_syntax##CExecItem::Action@@script", sExec.c_str());
    }
}
