#pragma once

namespace KC
{
    // �﷨��
    class CSyntaxItemImpl : public ISyntaxItem
    {
    public:
        CSyntaxItemImpl(ISyntaxItemWork&, IWebPageData&, int, const TKcSynBaseClass&);

        // �õ����
        virtual int GetIndex(void) const;
        // �õ��к�
        virtual int GetLineID(void) const;
        // ��ȡ�﷨��ʼλ��ָ��
        virtual const char* GetBeginPtr(void) const;
        virtual const char* GetEndPtr(void) const;
        // ��ȡ�﷨����
        virtual string GetSyntaxContent(void) const;
        // ��������
        virtual void ParseItem(void);
        // ��һ����﷨��
        virtual int GetNextIndex(IKCActionData&);
        // ����ҳ����
        virtual void ActionItem(IKCActionData&);

    protected:
        // ����ҳ����
        virtual void Action(IKCActionData&);
        // ��˳����������ջ����ѹջ
        void PushConditionStatck(TConditionStatck&, string, bool);
        void PushConditionStatck(TConditionStatck&, const TConditionItem&);
        // ��˳����������ջ������ջ
        bool PopConditionStatck(TConditionStatck&, TConditionItem&);
        // ��ȡ��ʶ����Ӧ��
        IIDNameItemWork& getIIDNameInf(void);
        // ��ȡҳ�湤����Ӧ��
        IWebPageDataFactory& getPageFactoryRef(void);

        // ��ȡ�﷨�������ʽ��ֵ
        template<typename TSYN>
        bool GetSynCondValue(IKCActionData& act, TSYN& syn, bool bDef = true)
        {
            bool bCond = bDef;
            const TKcWebExpr& exprCond = typename TSYN::template TSetGetVal<0>(syn).Get();
            if (!exprCond.IsNullptr())
            {
                boost::any valCond = act.GetExprValue(exprCond);
                if (valCond.type() != typeid(bool))
                    throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(this->m_work.getHint("The_condition_must_be_Boolean_type")) % this->GetKeychar() % this->GetLineID()).str(), this->m_work, this->GetBeginPtr(), this->GetEndPtr());
                bCond = boost::any_cast<bool>(valCond);
            }
            return bCond;
        }

    public:
        // ��������﷨��
        void SetNextRelated(CSyntaxItemImpl& syn);
        // �õ�����﷨��
        CSyntaxItemImpl* GetNextRelated(void);

    protected:
        // �﷨����ӿ�
        ISyntaxItemWork& m_work;
        // ҳ���ݽӿ�
        IWebPageData& m_pd;
        // ˳���
        int m_Index = 0;
        // �﷨��ʼλ��ָ��
        const char* m_pBeginPtr = nullptr;
        const char* m_pEndPtr = nullptr;
        // �к�
        int m_LineID = -1;
        // ����
        CSyntaxItemImpl *m_next = nullptr;
    };
}
