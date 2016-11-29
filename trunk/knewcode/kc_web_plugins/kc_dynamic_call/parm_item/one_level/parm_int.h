#pragma once

#include "../parm_item_if.h"

namespace KC { namespace parm_item { namespace one_level
{
    // ���Ͳ���
    class TParmInt : public IParmItem
    {
    public:
        // ����
        TParmInt(boost::any&, TKcParmFL&, IFuncCallDyn::TParmRefBackPtr);

        // ��Ӳ���
        virtual void AddArg(CKCFuncDyncall&);
        // ���ò����ص�
        virtual void ParmRefBack(void);

    private:
        IFuncCallDyn::TParmRefBackPtr m_refBack;
        boost::any m_val;
        TKcParmFL m_dt;
        int m_value = 0;
    };
}}}
