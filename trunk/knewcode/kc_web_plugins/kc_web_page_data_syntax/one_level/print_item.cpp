#include "../std.h"
#include "print_item.h"

////////////////////////////////////////////////////////////////////////////////
// CPrintItem类
KC::one_level::CPrintItem::CPrintItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcPrintSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CPrintItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordPrint;
}

// 语法类型转换
const TKcPrintSent& KC::one_level::CPrintItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcPrintSent&>(syn);
}

// 处理页数据
void KC::one_level::CPrintItem::Action(IKCActionData& act)
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
        string sPrint;
        if (val.type() == typeid(int)) sPrint = lexical_cast<string>(boost::any_cast<int>(val));
        else if (val.type() == typeid(double)) sPrint = lexical_cast<string>(boost::any_cast<double>(val));
        else if (val.type() == typeid(string)) sPrint = boost::any_cast<string>(val);
        else if (val.type() == typeid(bool)) sPrint = boost::any_cast<bool>(val) ? "true" : "false";
        else if (val.type() == typeid(TKcWebInfVal)) sPrint = boost::any_cast<TKcWebInfVal>(val).str();
        act.GetRequestRespond().OutputHTML(sPrint.c_str());
    }
}
