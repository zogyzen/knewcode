#pragma once

namespace KC
{
    // �������ڵ�
    class TOperandNode : public IOperandNode
    {
    public:
        // ����
        TOperandNode(IExprTreeWork&, IWebPageData&, const TKcSynBaseClass&);
        // �õ��к�
        virtual int GetLineID(void) const;

    protected:
        // ���ʽ����ӿ�
        IExprTreeWork& m_work;
        // ҳ���ݽӿ�
        IWebPageData& m_pd;
        // �﷨��ʼλ��ָ��
        const char* m_pBeginPtr = nullptr;
        const char* m_pEndPtr = nullptr;
        // �к�
        int m_LineID = -1;
    };

    // һԪ������ڵ�
    class TUnaryNode : public TOperandNode
    {
    public:
        // ��ȡ�������
        virtual string GetSymbol(void) = 0;
        // ����
        virtual boost::any Calculate(boost::any&) = 0;

    public:
        // ����
        TUnaryNode(IExprTreeWork&, IWebPageData&, const TKcSynBaseClass&);
        // ��ȡֵ
        virtual boost::any GetValue(IKCActionData&);

    public:
        // ������
        TKcWebExpr::TExprTreeNodePtr OperandPtr;
    };

    // ��Ԫ������ڵ�
    class TBinaryNode : public TOperandNode
    {
    public:
        // ��ȡ�������
        virtual string GetSymbol(void) = 0;
        // ����
        virtual boost::any Calculate(boost::any&, boost::any&) = 0;

    public:
       // ����
        TBinaryNode(IExprTreeWork&, IWebPageData&, const TKcSynBaseClass&);
        // ��ȡֵ
        virtual boost::any GetValue(IKCActionData&);

    public:
        // ���Ҳ�����
        TKcWebExpr::TExprTreeNodePtr OperandPtrL;
        TKcWebExpr::TExprTreeNodePtr OperandPtrR;

    protected:
        // �õ�����Ӧ���͵���ֵ
        boost::any GetLValueNewType(boost::any&, boost::any&);
    };
}
