#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/page_data_i.h"

namespace KC
{
    // �������ڵ�ӿ�
    class IOperandNode : public TKcWebExpr::IExprTreeNodeBase
    {
    public:
        // ��ȡֵ
        virtual boost::any GetValue(IKCActionData&) = 0;
        // �õ��к�
        virtual int GetLineID(void) const = 0;
    };

    // ���ʽ������ӿ�
    class IExprTreeWork : public IServiceEx
    {
    public:
        // ���ñ��ʽ�������
        virtual IOperandNode* SetExprOperator(IWebPageData&, TKcWebExpr&, const TKcSynBaseClass&) = 0;
        // ���ñ��ʽ�������
        virtual IOperandNode* SetExprOperand(IWebPageData&, TKcWebExpr&, const TKcWebExpr&) = 0;

    protected:
        virtual CALL_TYPE ~IExprTreeWork(void) = default;
    };

    constexpr const char c_ExprTreeWorkSrvGUID[] = "IExprTreeWork_08001475-3590-4144-A5B3-8004131807CA";

    class TExprTreeWorkSrvException : public TFWSrvException
	{
	public:
		TExprTreeWorkSrvException(int id, string place, string msg, string name)
				: TFWSrvException(id, place, msg, name, c_ExprTreeWorkSrvGUID) {}
		TExprTreeWorkSrvException(int id, string place, string msg, IExprTreeWork& srv)
				: TFWSrvException(id, place, msg, srv) {}
	};
}
