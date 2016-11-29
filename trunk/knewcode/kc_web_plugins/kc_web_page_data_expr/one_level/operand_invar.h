#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // 内部变量节点
    class TInnerVarNode : public TOperandNode
    {
    public:
        // 构造，拷贝
        TInnerVarNode(IExprTreeWork&, IWebPageData&, const TKcInnerVarSyn&);

        // 获取变量的值
        virtual boost::any GetValue(IKCActionData&);
        // 设置变量的值
        void SetValue(IKCActionData&, boost::any);

    private:
        // 获取下标值
        void GetArray(std::vector<boost::any>&, IKCActionData&);

    private:
        TKcInnerVarSyn m_syn;
    };
}}
