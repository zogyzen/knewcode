#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 不等于比较运算
    class TBinaryNtEq : public TBinaryNode
    {
    public:
        // 构造
        TBinaryNtEq(IExprTreeWork&, IWebPageData&, const TKcNotEqualExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
