#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 乘运算
    class TBinaryMult : public TBinaryNode
    {
    public:
        // 构造
        TBinaryMult(IExprTreeWork&, IWebPageData&, const TKcStarExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
