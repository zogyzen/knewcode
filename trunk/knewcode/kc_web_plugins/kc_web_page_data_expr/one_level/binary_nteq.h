#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // �����ڱȽ�����
    class TBinaryNtEq : public TBinaryNode
    {
    public:
        // ����
        TBinaryNtEq(IExprTreeWork&, IWebPageData&, const TKcNotEqualExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
