#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 圆括号运算
    class TUnaryYKH : public TUnaryNode
    {
    public:
        // 构造
        TUnaryYKH(IExprTreeWork&, IWebPageData&, const TKcYKHLeftExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&);
    };
}}
