#pragma once

#include "../idname_item_if.h"

namespace KC { namespace one_level
{
    // �����ͱ���
    class TVarBool : public TVarItemBase
    {
    public:
        // ���죬����
        TVarBool(IIDNameItemWork&, IKCActionData&, string);

        // ������ֵ
        virtual void SetVarValue(boost::any);
        // ��ȡ������ֵ
        virtual boost::any GetVarValue(void);

    private:
        // ����ֵ
        bool m_value = false;
    };
}}
