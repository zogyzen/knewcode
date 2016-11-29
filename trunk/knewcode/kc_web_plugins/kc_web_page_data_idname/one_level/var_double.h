#pragma once

#include "../idname_item_if.h"

namespace KC { namespace one_level
{
    // �����ͱ���
    class TVarDouble : public TVarItemBase
    {
    public:
        // ���죬����
        TVarDouble(IIDNameItemWork&, IKCActionData&, string);

        // ������ֵ
        virtual void SetVarValue(boost::any);
        // ��ȡ������ֵ
        virtual boost::any GetVarValue(void);

    private:
        // ����ֵ
        double m_value = 0;
    };
}}
