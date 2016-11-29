#pragma once

namespace KC
{
    // 操作数节点
    class TOperandNode : public IOperandNode
    {
    public:
        // 构造
        TOperandNode(IExprTreeWork&, IWebPageData&, const TKcSynBaseClass&);
        // 得到行号
        virtual int GetLineID(void) const;

    protected:
        // 表达式运算接口
        IExprTreeWork& m_work;
        // 页数据接口
        IWebPageData& m_pd;
        // 语法起始位置指针
        const char* m_pBeginPtr = nullptr;
        const char* m_pEndPtr = nullptr;
        // 行号
        int m_LineID = -1;
    };

    // 一元运算符节点
    class TUnaryNode : public TOperandNode
    {
    public:
        // 获取运算符号
        virtual string GetSymbol(void) = 0;
        // 计算
        virtual boost::any Calculate(boost::any&) = 0;

    public:
        // 构造
        TUnaryNode(IExprTreeWork&, IWebPageData&, const TKcSynBaseClass&);
        // 获取值
        virtual boost::any GetValue(IKCActionData&);

    public:
        // 操作数
        TKcWebExpr::TExprTreeNodePtr OperandPtr;
    };

    // 二元运算符节点
    class TBinaryNode : public TOperandNode
    {
    public:
        // 获取运算符号
        virtual string GetSymbol(void) = 0;
        // 计算
        virtual boost::any Calculate(boost::any&, boost::any&) = 0;

    public:
       // 构造
        TBinaryNode(IExprTreeWork&, IWebPageData&, const TKcSynBaseClass&);
        // 获取值
        virtual boost::any GetValue(IKCActionData&);

    public:
        // 左右操作数
        TKcWebExpr::TExprTreeNodePtr OperandPtrL;
        TKcWebExpr::TExprTreeNodePtr OperandPtrR;

    protected:
        // 得到自适应类型的左值
        boost::any GetLValueNewType(boost::any&, boost::any&);
    };
}
