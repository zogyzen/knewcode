#pragma once

#include "../parm_item_if.h"

namespace KC { namespace parm_item { namespace one_level
{
    // 浮点型参数
    class TParmString : public IParmItem
    {
    public:
        // 构造
        TParmString(boost::any&, TKcParmFL&, IFuncCallDyn::TParmRefBackPtr);

        // 添加参数
        virtual void AddArg(CKCFuncDyncall&);
        // 引用参数回调
        virtual void ParmRefBack(void);

        // 转换返回值
        static string TranResult(void*);

    private:
        IFuncCallDyn::TParmRefBackPtr m_refBack;
        boost::any m_val;
        TKcParmFL m_dt;
        // 缓冲区
        char* m_pBuf = nullptr;
        boost::shared_ptr<char> m_bufPtr;
    };
}}}
