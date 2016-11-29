#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 变量节点
    class TVarNode : public TOperandNode
    {
    public:
        // 构造，拷贝
        TVarNode(IExprTreeWork&, IWebPageData&, const TKcVarSyn&);

        // 获取变量的值
        virtual boost::any GetValue(IKCActionData&);
        // 设置变量的值
        void SetValue(IKCActionData&, boost::any);

    private:
        // 获取变量接口
        IVarItemBase& GetVarIf(IKCActionData& act);

    private:
        TKcVarSyn m_syn;
    };
}}
