#include "../std.h"
#include "unary_sub.h"

////////////////////////////////////////////////////////////////////////////////
// TUnarySub类
KC::one_level::TUnarySub::TUnarySub(IExprTreeWork& wk, IWebPageData& pd, const TKcSubtExpr& syn)
    : TUnaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TUnarySub::GetSymbol(void)
{
    return string() + Keychar::one_level::g_SymbolSubt;
}

// 计算
boost::any KC::one_level::TUnarySub::Calculate(boost::any& val)
{
    // 只能是整型或浮点型
    if (val.type() != typeid(int) && val.type() != typeid(double))
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_operand_can_only_be_specific_type_unary")) % "integer and double" % val.type().name() % this->GetLineID()).str(), m_work);
    // 整型
    if (val.type() == typeid(int))
        return -boost::any_cast<int>(val);
    // 浮点型
    else
        return -boost::any_cast<double>(val);
}
