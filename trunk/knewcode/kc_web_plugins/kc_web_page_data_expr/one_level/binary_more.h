#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 大于比较运算
    class TBinaryMore : public TBinaryNode
    {
    public:
        // 构造
        TBinaryMore(IExprTreeWork&, IWebPageData&, const TKcMoreExpr&);
        // 获取运算符号
        virtual string GetSymbol(void);
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
