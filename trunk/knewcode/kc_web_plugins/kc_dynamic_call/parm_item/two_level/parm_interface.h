#pragma once

#include "../parm_item_if.h"

namespace KC { namespace parm_item { namespace two_level
{
    // �����Ͳ���
    class TParmInterfacer : public IParmItem
    {
    public:
        // ����
        TParmInterfacer(boost::any&, TKcParmFL&, IFuncCallDyn::TParmRefBackPtr);

        // ��Ӳ���
        virtual void AddArg(CKCFuncDyncall&);
        // ���ò����ص�
        virtual void ParmRefBack(void);

        // ת������ֵ
        static boost::any TranResult(void*, string);

    private:
        IFuncCallDyn::TParmRefBackPtr m_refBack;
        boost::any m_val;
        TKcParmFL m_dt;
        TKcWebInfVal m_value;
    };
}}}
