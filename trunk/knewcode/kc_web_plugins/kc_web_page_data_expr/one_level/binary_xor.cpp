#include "../std.h"
#include "binary_xor.h"

////////////////////////////////////////////////////////////////////////////////
// TBinaryXor类
KC::one_level::TBinaryXor::TBinaryXor(IExprTreeWork& wk, IWebPageData& pd, const TKcXorExpr& syn)
    : TBinaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TBinaryXor::GetSymbol(void)
{
    return string() + Keychar::one_level::g_SymbolXor;
}

// 计算
boost::any KC::one_level::TBinaryXor::Calculate(boost::any& lv, boost::any& rv)
{
    if (lv.type() != typeid(bool) || rv.type() != typeid(bool))
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_operand_can_only_be_specific_type")) % "boolean" % lv.type().name() % rv.type().name() % this->GetLineID()).str(), m_work);
    return boost::any_cast<bool>(lv) ^ boost::any_cast<bool>(rv);
}
