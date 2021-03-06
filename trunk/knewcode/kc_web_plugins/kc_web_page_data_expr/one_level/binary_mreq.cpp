#include "../std.h"
#include "binary_mreq.h"

////////////////////////////////////////////////////////////////////////////////
// TBinaryMrEq类
KC::one_level::TBinaryMrEq::TBinaryMrEq(IExprTreeWork& wk, IWebPageData& pd, const TKcEqualMoreExpr& syn)
    : TBinaryNode(wk, pd, syn)
{
}

// 获取运算符号
string KC::one_level::TBinaryMrEq::GetSymbol(void)
{
    return Keychar::one_level::g_SymbolEqualMore;
}

// 计算
boost::any KC::one_level::TBinaryMrEq::Calculate(boost::any& lv, boost::any& rv)
{
    // 统一左右值的类型
    boost::any nLv = this->GetLValueNewType(lv, rv);
    boost::any nRv = this->GetLValueNewType(rv, lv);
    // 比较
    if (nLv.type() == typeid(int))
        return boost::any_cast<int>(nLv) >= boost::any_cast<int>(nRv);
    else if (nLv.type() == typeid(double))
        return boost::any_cast<double>(nLv) >= boost::any_cast<double>(nRv);
    else if (nLv.type() == typeid(bool))
        return boost::any_cast<bool>(nLv) >= boost::any_cast<bool>(nRv);
    else
        return boost::any_cast<string>(nLv) >= boost::any_cast<string>(nRv);
}
