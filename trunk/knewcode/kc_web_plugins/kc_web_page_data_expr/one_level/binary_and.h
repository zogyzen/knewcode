#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 逻辑与运算
    class TBinaryAnd : public TBinaryNode
    {
    public:
        // 构造
        TBinaryAnd(IExprTreeWork&, IWebPageData&, const TKcAndExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
