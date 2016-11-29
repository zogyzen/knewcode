#include "../std.h"
#include "exit_item.h"

////////////////////////////////////////////////////////////////////////////////
// CExitItem类
KC::one_level::CExitItem::CExitItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcExitSent& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CExitItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordExit;
}

// 语法类型转换
const TKcExitSent& KC::one_level::CExitItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcExitSent&>(syn);
}

// 相关语法项
int KC::one_level::CExitItem::GetNextIndex(IKCActionData& act)
{
    // 条件
    bool bCond = this->GetSynCondValue(act, m_syn);
    // 条件为真退出
    if (bCond)
    {
        // 输出
        const TKcWebExpr& exprInfo = m_syn.GetVal<1>();
        if (!exprInfo.IsNullptr())
        {
            boost::any valInfo = act.GetExprValue(exprInfo);
            string sPrint = "";
            if (valInfo.type() == typeid(int))
                sPrint = lexical_cast<string>(boost::any_cast<int>(valInfo));
            if (valInfo.type() == typeid(double))
                sPrint = lexical_cast<string>(boost::any_cast<double>(valInfo));
            if (valInfo.type() == typeid(string))
                sPrint = boost::any_cast<string>(valInfo);
            if (valInfo.type() == typeid(bool))
                sPrint = boost::any_cast<bool>(valInfo) ? "true" : "false";
            act.GetRequestRespond().OutputHTML(sPrint.c_str());
        }
        // 退出
        return -1;
    }
    else
        return CSyntaxItemImpl::GetNextIndex(act);
}
