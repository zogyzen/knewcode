#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 小于等于比较运算
    class TBinaryLsEq : public TBinaryNode
    {
    public:
        // 构造
        TBinaryLsEq(IExprTreeWork&, IWebPageData&, const TKcEqualLessExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
