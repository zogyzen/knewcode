#pragma once

#include "../idname_item_if.h"

namespace KC { namespace two_level
{
    // �ӿ��ͱ���
    class TVarInterface : public TVarItemBase, public TFuncLibItemBase
    {
    public:
        // ���죬����
        TVarInterface(IIDNameItemWork&, IKCActionData&, string);

        // ������ֵ
        virtual void SetVarValue(boost::any);
        // ��ȡ������ֵ
        virtual boost::any GetVarValue(void);

        // ��ȡ��������
        virtual TKcFuncDefFL& GetFuncDef(string, IFuncCallDyn&);

    private:
        // ��ȡ�̳е�ϵ�нӿ�
        void GetBaseToTheInf(vector<TKcInfFullFL*>&, string);

    private:
        // ����ֵ
        TKcWebInfVal m_value;
    };
}}
