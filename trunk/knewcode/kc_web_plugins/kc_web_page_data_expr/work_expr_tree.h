#pragma once

namespace KC
{
    // 表达式树管理接口
    class CExprTreeWork : public IExprTreeWork
    {
    public:
        CExprTreeWork(const IBundle& bundle);
        virtual CALL_TYPE ~CExprTreeWork(void) = default;

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 设置表达式项运算符
        virtual IOperandNode* SetExprOperator(IWebPageData&, TKcWebExpr&, const TKcSynBaseClass&);
        // 设置表达式项操作数
        virtual IOperandNode* SetExprOperand(IWebPageData&, TKcWebExpr&, const TKcWebExpr&);

    public:
        // 获取动态调用的应用
        IDynamicCall& getDynamicCallInf(void);
        // 获取内部变量的应用
        IInnerVar& getInnerVarInf(void);
        // 获取标识符的应用
        IIDNameItemWork& getIDNameInf(void);

    private:
        // 创建操作数节点
        template<typename OPERAND, typename SYN> OPERAND& CreateOperandNode(TKcWebExpr&, IWebPageData&, const SYN&);
        // 创建一元运算符节点
        template<typename OPERATOR, typename SYN> OPERATOR& CreateUnaryOperator(TKcWebExpr&, IWebPageData&, const SYN&);
        // 创建二元运算符节点
        template<typename OPERATOR, typename SYN> OPERATOR& CreateBinaryOperator(TKcWebExpr&, IWebPageData&, const SYN&);

    private:
        IBundleContext& m_context;
        const IBundle& m_bundle;
        // 动态调用的应用
        IServiceReference* m_DynamicCallRef = nullptr;
        // 内部变量的应用
        IServiceReference* m_InnerVarRef = nullptr;
        // 标识符的应用
        IServiceReference* m_IDNameRef = nullptr;
    };
}
