#include "../std.h"
#include "binary_divi.h"

////////////////////////////////////////////////////////////////////////////////
// TBinaryDivi类
KC::one_level::TBinaryDivi::TBinaryDivi(IExprTreeWork& wk, IWebPageData& pd, const TKcDivExpr& syn)
    : TBinaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TBinaryDivi::GetSymbol(void)
{
    return string() + Keychar::one_level::g_SymbolDiv;
}

// 计算
boost::any KC::one_level::TBinaryDivi::Calculate(boost::any& lv, boost::any& rv)
{
    // 除数不能为0
    if ((rv.type() == typeid(int) && boost::any_cast<int>(rv) == 0) || (rv.type() == typeid(double) && boost::any_cast<double>(rv) == 0))
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, m_work.getHint("The_divisor_can_t_be_0_") + lexical_cast<string>(this->GetLineID()), m_work);
    // 只能是整型或浮点型
    if ((lv.type() != typeid(int) && lv.type() != typeid(double)) || (rv.type() != typeid(int) && rv.type() != typeid(double)))
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("The_operand_can_only_be_specific_type")) % "integer and double" % lv.type().name() % rv.type().name() % this->GetLineID()).str(), m_work);
    // 都是整型
    if (lv.type() == typeid(int) && rv.type() == typeid(int))
        return boost::any_cast<int>(lv) / boost::any_cast<int>(rv);
    // 转换为浮点型
    double  dLv = (lv.type() == typeid(int)) ? boost::any_cast<int>(lv) : boost::any_cast<double>(lv),
            dRv = (rv.type() == typeid(int)) ? boost::any_cast<int>(rv) : boost::any_cast<double>(rv);
    return dLv / dRv;
}
