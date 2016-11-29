#pragma once

#include "kc_func_dyncall.h"

namespace KC
{
    // 汇编语言代码的类
    class KC_Asm : public CKCFuncDyncall::IActionAsm
    {
    public:
        KC_Asm(void);
        virtual ~KC_Asm(void);

        // 添加实参
        void AddArg(CKCFuncDyncall::EDataType, boost::any);
        // 设置返回值类型
        void SetResult(CKCFuncDyncall::EDataType);
        void SetResult(void);
        // 设置函数指针
        void SetFunc(CKCFuncDyncall::TFuncPointer);
        // 设置成员函数指针
        void SetMemberFunc(CKCFuncDyncall::TObjectPointer, CKCFuncDyncall::TFuncPointer);
        // 设置虚函数指针
        void SetVirtualFunc(CKCFuncDyncall::TObjectPointer, int);
        // 调用函数
        void CallFunc(void*&, double&);
        // 清除设置
        void Clear(void);

    public:
        // 实参列表
        struct TArgValue
        {
            CKCFuncDyncall::EDataType m_Type = CKCFuncDyncall::DtBool;
            union
            {
                bool m_bVal;
                int m_iVal;
                double m_dVal;
                void* m_pVal;
            };
        };
        typedef vector<TArgValue> TArgValueList;

    public:
        // 实参缓冲区（用于汇编）
        struct IArgAsmWorkBuf
        {
            virtual ~IArgAsmWorkBuf(void) = default;
        };

    private:
        // 函数指针
        CKCFuncDyncall::TFuncPointer m_FuncPointer = nullptr;
        // 类实例指针
        CKCFuncDyncall::TObjectPointer m_ThisPointer = nullptr;
        // 返回值类型
        bool m_haveResult = false;
        CKCFuncDyncall::EDataType m_ResultType = CKCFuncDyncall::DtBool;
        // 实参列表
        TArgValueList m_ArgList;
        IArgAsmWorkBuf* m_ArgAsmWorkBuf = nullptr;
    };
}
