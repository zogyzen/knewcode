#pragma once

#include "../expr_item_if.h"

namespace KC { namespace one_level
{
    // ���ڱȽ�����
    class TBinaryMore : public TBinaryNode
    {
    public:
        // ����
        TBinaryMore(IExprTreeWork&, IWebPageData&, const TKcMoreExpr&);
        // ��ȡ�������
        virtual string GetSymbol(void);
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&);
    };
}}
