#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 负数运算
    class TUnarySub : public TUnaryNode
    {
    public:
        // 构造
        TUnarySub(IExprTreeWork&, IWebPageData&, const TKcSubtExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&);
    };
}}
