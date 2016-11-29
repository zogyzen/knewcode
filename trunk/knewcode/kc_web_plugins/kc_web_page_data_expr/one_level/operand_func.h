#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // �������ýڵ�
    class TFuncallNode : public TOperandNode
    {
    public:
        // ���죬����
        TFuncallNode(IExprTreeWork&, IWebPageData&, const TKcFuncCallSyn&);

        // ��ȡ��������ֵ
        virtual boost::any GetValue(IKCActionData&);

    private:
        // ִ�к���
        boost::any RunFunc(IKCActionData&, IFuncLibItemBase*, string, TKcFuncCallBodySyn&);

    private:
        TKcFuncCallSyn m_syn;
    };
}}
