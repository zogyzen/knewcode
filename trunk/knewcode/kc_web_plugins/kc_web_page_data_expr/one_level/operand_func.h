#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 函数调用节点
    class TFuncallNode : public TOperandNode
    {
    public:
        // 构造，拷贝
        TFuncallNode(IExprTreeWork&, IWebPageData&, const TKcFuncCallSyn&);

        // 获取操作数的值
        virtual boost::any GetValue(IKCActionData&);

    private:
        // 执行函数
        boost::any RunFunc(IKCActionData&, IFuncLibItemBase*, string, TKcFuncCallBodySyn&);

    private:
        TKcFuncCallSyn m_syn;
    };
}}
