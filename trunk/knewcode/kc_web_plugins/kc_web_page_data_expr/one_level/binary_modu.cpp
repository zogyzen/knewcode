#include "../std.h"
#include "binary_modu.h"

////////////////////////////////////////////////////////////////////////////////
// TBinaryModu类
KC::one_level::TBinaryModu::TBinaryModu(IExprTreeWork& wk, IWebPageData& pd, const TKcModExpr& syn)
    : TBinaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TBinaryModu::GetSymbol(void)
{
    return string() + Keychar::one_level::g_SymbolMod;
}

// 计算
boost::any KC::one_level::TBinaryModu::Calculate(boost::any& lv, boost::any& rv)
{
    if (lv.type() != typeid(int) || rv.type() != typeid(int))
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_operand_can_only_be_specific_type")) % "integer" % lv.type().name() % rv.type().name() % this->GetLineID()).str(), m_work);
    return boost::any_cast<int>(lv) % boost::any_cast<int>(rv);
}
