#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/syntax_struct_two.h"

namespace KC
{
    // �������ýӿ�
    class IFuncCallDyn
    {
    public:
        // ���ò����ص��ӿ�
        class IParmRefBack
        {
        public:
            // �������õķ���ֵ
            virtual void SetValue(boost::any&) = 0;

        public:
            virtual ~IParmRefBack() = default;
        };
        typedef boost::shared_ptr<IParmRefBack> TParmRefBackPtr;

    public:
        // ��ʵ��
        virtual void AddArg(boost::any&, TKcParmFL&, TParmRefBackPtr) = 0;
        // ���÷���ֵ����
        virtual void SetResult(TKcParmFL&) = 0;
        // ���ú���ָ��
        virtual void SetFunc(TFuncPointer) = 0;
        // ���ó�Ա����ָ��
        virtual void SetMemberFunc(TObjectPointer, TFuncPointer) = 0;
        // �����麯��ָ��
        virtual void SetVirtualFunc(TObjectPointer, int) = 0;
        // ���ú���
        virtual boost::any Call(string) = 0;

    protected:
        virtual ~IFuncCallDyn(void) = default;
    };

    class IKCActionData;
    // ��̬���ýӿ�
    class IDynamicCall : public IServiceEx
    {
    public:
        // �����������ýӿ�
        virtual IFuncCallDyn& NewFuncCall(IKCActionData&) = 0;
        // �ͷź������ýӿ�
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

