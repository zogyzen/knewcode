#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 加运算
    class TBinaryPlus : public TBinaryNode
    {
    public:
        // 构造
        TBinaryPlus(IExprTreeWork&, IWebPageData&, const TKcPlusExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
