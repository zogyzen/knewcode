#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // �����ڵ�
    class TConstNode : public TOperandNode
    {
    public:
        // ����
        TConstNode(IExprTreeWork&, IWebPageData&, const TKcConstSyn&);

        // ��ȡ��������ֵ
        virtual boost::any GetValue(IKCActionData&);

    private:
        TKcConstSyn m_syn;
    };
}}
