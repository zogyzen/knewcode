#include "../std.h"
#include "binary_and.h"

////////////////////////////////////////////////////////////////////////////////
// TBinaryAnd类
KC::one_level::TBinaryAnd::TBinaryAnd(IExprTreeWork& wk, IWebPageData& pd, const TKcAndExpr& syn)
    : TBinaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TBinaryAnd::GetSymbol(void)
{
    return Keychar::one_level::g_SymbolAnd;
}

// 计算
boost::any KC::one_level::TBinaryAnd::Calculate(boost::any& lv, boost::any& rv)
{
    if (lv.type() != typeid(bool) || rv.type() != typeid(bool))
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_operand_can_only_be_specific_type")) % "boolean" % lv.type().name() % rv.type().name() % this->GetLineID()).str(), m_work);
    return boost::any_cast<bool>(lv) && boost::any_cast<bool>(rv);
}
