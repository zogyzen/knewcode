#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 常量节点
    class TConstNode : public TOperandNode
    {
    public:
        // 构造
        TConstNode(IExprTreeWork&, IWebPageData&, const TKcConstSyn&);

        // 获取操作数的值
        virtual boost::any GetValue(IKCActionData&);

    private:
        TKcConstSyn m_syn;
    };
}}
