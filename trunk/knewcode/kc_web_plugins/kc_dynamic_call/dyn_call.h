#pragma once

#include "parm_item/parm_item_if.h"

namespace KC
{
    class CDynamicCall : public IDynamicCall
    {
    public:
        CDynamicCall(const IBundle& bundle);
        virtual CALL_TYPE ~CDynamicCall(void);

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 创建函数调用接口
        virtual IFuncCallDyn& NewFuncCall(IKCActionData&);
        // 释放函数调用接口
        virtual void FreeFuncCall(IFuncCallDyn&);

    private:
        class CFuncCall : public IFuncCallDyn
        {
        public:
            CFuncCall(CDynamicCall&, IKCActionData&);
            virtual ~CFuncCall(void);
            // 加实参
            virtual void AddArg(boost::any&, TKcParmFL&, TParmRefBackPtr);
            // 设置返回值类型
            virtual void SetResult(TKcParmFL&);
            // 设置函数指针
            virtual void SetFunc(TFuncPointer);
            // 设置成员函数指针
            virtual void SetMemberFunc(TObjectPointer, TFuncPointer);
            // 设置虚函数指针
            virtual void SetVirtualFunc(TObjectPointer, int);
            // 调用函数
            virtual boost::any Call(string);

        private:
            CDynamicCall& m_fct;
            IKCActionData& m_act;
            // 函数动态调用实例
            CKCFuncDyncall m_KCDyncall;
            // 参数列表
            TParmItemPtrList m_ParmItemPtrList;
            // 返回值
            TKcParmFL m_dtRes;
        };

    private:
        IBundleContext& m_context;
        const IBundle& m_bundle;
    };
}
