#pragma once

namespace KC
{
    // 语法项
    class CSyntaxItemImpl : public ISyntaxItem
    {
    public:
        CSyntaxItemImpl(ISyntaxItemWork&, IWebPageData&, int, const TKcSynBaseClass&);

        // 得到序号
        virtual int GetIndex(void) const;
        // 得到行号
        virtual int GetLineID(void) const;
        // 获取语法起始位置指针
        virtual const char* GetBeginPtr(void) const;
        virtual const char* GetEndPtr(void) const;
        // 获取语法内容
        virtual string GetSyntaxContent(void) const;
        // 解析数据
        virtual void ParseItem(void);
        // 下一相关语法项
        virtual int GetNextIndex(IKCActionData&);
        // 处理页数据
        virtual void ActionItem(IKCActionData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);
        // 非顺序语句的条件栈——压栈
        void PushConditionStatck(TConditionStatck&, string, bool);
        void PushConditionStatck(TConditionStatck&, const TConditionItem&);
        // 非顺序语句的条件栈——弹栈
        bool PopConditionStatck(TConditionStatck&, TConditionItem&);
        // 获取标识符的应用
        IIDNameItemWork& getIIDNameInf(void);
        // 获取页面工厂的应用
        IWebPageDataFactory& getPageFactoryRef(void);

        // 获取语法条件表达式的值
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
        // 设置相关语法项
        void SetNextRelated(CSyntaxItemImpl& syn);
        // 得到相关语法项
        CSyntaxItemImpl* GetNextRelated(void);

    protected:
        // 语法运算接口
        ISyntaxItemWork& m_work;
        // 页数据接口
        IWebPageData& m_pd;
        // 顺序号
        int m_Index = 0;
        // 语法起始位置指针
        const char* m_pBeginPtr = nullptr;
        const char* m_pEndPtr = nullptr;
        // 行号
        int m_LineID = -1;
        // 关联
        CSyntaxItemImpl *m_next = nullptr;
    };
}
