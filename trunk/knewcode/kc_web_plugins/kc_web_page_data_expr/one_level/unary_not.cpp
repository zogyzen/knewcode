#include "../std.h"
#include "unary_not.h"

////////////////////////////////////////////////////////////////////////////////
// TUnaryNot类
KC::one_level::TUnaryNot::TUnaryNot(IExprTreeWork& wk, IWebPageData& pd, const TKcNotExpr& syn)
    : TUnaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TUnaryNot::GetSymbol(void)
{
    return string() + Keychar::one_level::g_SymbolNot;
}

// 计算
boost::any KC::one_level::TUnaryNot::Calculate(boost::any& val)
{
    if (val.type() != typeid(bool))
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_operand_can_only_be_specific_type_unary")) % "boolean" % val.type().name() % this->GetLineID()).str(), m_work);
    return !boost::any_cast<bool>(val);
}
