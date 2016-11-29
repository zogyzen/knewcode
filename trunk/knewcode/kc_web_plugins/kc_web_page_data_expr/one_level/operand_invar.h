#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // �ڲ������ڵ�
    class TInnerVarNode : public TOperandNode
    {
    public:
        // ���죬����
        TInnerVarNode(IExprTreeWork&, IWebPageData&, const TKcInnerVarSyn&);

        // ��ȡ������ֵ
        virtual boost::any GetValue(IKCActionData&);
        // ���ñ�����ֵ
        void SetValue(IKCActionData&, boost::any);

    private:
        // ��ȡ�±�ֵ
        void GetArray(std::vector<boost::any>&, IKCActionData&);

    private:
        TKcInnerVarSyn m_syn;
    };
}}
