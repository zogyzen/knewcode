#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/page_data_i.h"

namespace KC
{
    // 操作数节点接口
    class IOperandNode : public TKcWebExpr::IExprTreeNodeBase
    {
    public:
        // 获取值
        virtual boost::any GetValue(IKCActionData&) = 0;
        // 得到行号
        virtual int GetLineID(void) const = 0;
    };

    // 表达式树管理接口
    class IExprTreeWork : public IServiceEx
    {
    public:
        // 设置表达式项运算符
        virtual IOperandNode* SetExprOperator(IWebPageData&, TKcWebExpr&, const TKcSynBaseClass&) = 0;
        // 设置表达式项操作数
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
