#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 逻辑或运算
    class TBinaryOr : public TBinaryNode
    {
    public:
        // 构造
        TBinaryOr(IExprTreeWork&, IWebPageData&, const TKcOrExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
