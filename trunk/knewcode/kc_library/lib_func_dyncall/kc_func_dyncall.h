#pragma once

#include <boost/any.hpp>

namespace KC
{
    // 动态调用函数的类
    class CKCFuncDyncall
    {
    private:
        class ObjectPointer{ virtual ~ObjectPointer(void) = default; };

    public:
        // 数据类型
        enum EDataType
        {
            DtPointer   = 0,    // 指针类型
            DtBool      = 1,    // 布尔型
            DtInt       = 2,    // 整型
            DtDouble    = 3,    // 浮点型
            DtInterface = 4     // 接口型
        };
        // 函数指针
        typedef void (*TFuncPointer)(void);
        // 对象指针
        typedef ObjectPointer *TObjectPointer;

    public:
        CKCFuncDyncall(void);
        ~CKCFuncDyncall(void);

        // 添加实参
        void AddArg(EDataType, boost::any);
        // 设置返回值类型
        void SetResult(EDataType);
        void SetResult(void);
        // 设置函数指针
        void SetFunc(TFuncPointer);
        // 设置成员函数指针
        void SetMemberFunc(TObjectPointer, TFuncPointer);
        // 设置虚函数指针
        void SetVirtualFunc(TObjectPointer, int);
        // 调用函数
        void CallFunc(void*&, double&);
        // 清除设置
        void Clear(void);

    public:
        // 实际处理接口
        class IActionAsm
        {
        public:
            virtual ~IActionAsm(void) = default;
        };

    private:
        // 实际处理接口
        IActionAsm* m_ActionAsm = nullptr;
    };
}
