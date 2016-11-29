#pragma once

#include "../idname_item_if.h"

namespace KC { namespace one_level
{
    // ��̬���еĺ���
    class TFuncLib : public TFuncLibItemBase
    {
    public:
        // ���죬����
        TFuncLib(IIDNameItemWork&, IKCActionData&, string, const TKcLoadFullFL&);

        // ��ȡ��������
        virtual TKcFuncDefFL& GetFuncDef(string, IFuncCallDyn&);

    private:
        // ���ؿ�����
        TKcLoadFullFL m_loadSyn;
    };
}}
