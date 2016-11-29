#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // �����ڵ�
    class TVarNode : public TOperandNode
    {
    public:
        // ���죬����
        TVarNode(IExprTreeWork&, IWebPageData&, const TKcVarSyn&);

        // ��ȡ������ֵ
        virtual boost::any GetValue(IKCActionData&);
        // ���ñ�����ֵ
        void SetValue(IKCActionData&, boost::any);

    private:
        // ��ȡ�����ӿ�
        IVarItemBase& GetVarIf(IKCActionData& act);

    private:
        TKcVarSyn m_syn;
    };
}}
