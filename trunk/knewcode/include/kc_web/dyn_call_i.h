#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/syntax_struct_two.h"

namespace KC
{
    // 函数调用接口
    class IFuncCallDyn
    {
    public:
        // 引用参数回调接口
        class IParmRefBack
        {
        public:
            // 设置引用的返回值
            virtual void SetValue(boost::any&) = 0;

        public:
            virtual ~IParmRefBack() = default;
        };
        typedef boost::shared_ptr<IParmRefBack> TParmRefBackPtr;

    public:
        // 加实参
        virtual void AddArg(boost::any&, TKcParmFL&, TParmRefBackPtr) = 0;
        // 设置返回值类型
        virtual void SetResult(TKcParmFL&) = 0;
        // 设置函数指针
        virtual void SetFunc(TFuncPointer) = 0;
        // 设置成员函数指针
        virtual void SetMemberFunc(TObjectPointer, TFuncPointer) = 0;
        // 设置虚函数指针
        virtual void SetVirtualFunc(TObjectPointer, int) = 0;
        // 调用函数
        virtual boost::any Call(string) = 0;

    protected:
        virtual ~IFuncCallDyn(void) = default;
    };

    class IKCActionData;
    // 动态调用接口
    class IDynamicCall : public IServiceEx
    {
    public:
        // 创建函数调用接口
        virtual IFuncCallDyn& NewFuncCall(IKCActionData&) = 0;
        // 释放函数调用接口
        virtual void FreeFuncCall(IFuncCallDyn&) = 0;

    protected:
        virtual CALL_TYPE ~IDynamicCall(void) = default;
    };

    constexpr const char c_DynamicCallSrvGUID[] = "IDynamicCall_3AE366D7-A8E1-C84D-B6A4-E7FC44C90FD8";

    class TDynamicCallException : public TFWSrvException
	{
	public:
		TDynamicCallException(int id, string place, string msg, string name)
				: TFWSrvException(id, place, msg, name, c_DynamicCallSrvGUID) {}
		TDynamicCallException(int id, string place, string msg, IDynamicCall& srv)
				: TFWSrvException(id, place, msg, srv) {}
	};
}

