#pragma once

namespace KC
{
    // 语法项管理接口
    class CSyntaxItemWork : public ISyntaxItemWork
    {
    public:
        CSyntaxItemWork(const IBundle& bundle);
        virtual CALL_TYPE ~CSyntaxItemWork() = default;

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 创建语法项
        virtual TSyntaxItemPtr NewSyntaxItem(const TKcSynBaseClass&, IWebPageData&, int, TSyntaxItemStack&);

    public:
        // 获取标识符的应用
        IIDNameItemWork& getIIDNameInf(void);
        // 获取页面工厂的应用
        IWebPageDataFactory& getPageFactoryRef(void);
        // 创建语法项（非结构语句）
        TSyntaxItemPtr NewSyntaxItem(const TKcSynBaseClass&, IWebPageData&, int);

    private:
        // 添加语法条款
        template<typename SYNTAX, typename DATA> SYNTAX* AddSyntaxItem(IWebPageData&, int, const DATA&);

        /// 创建结构语句
        // 条件语句
        ISyntaxItem* NewIfSection(IWebPageData&, int, const TKcIfSent&, TSyntaxItemStack&);
        ISyntaxItem* NewElseIfSection(IWebPageData&, int, const TKcElseIfSent&, TSyntaxItemStack&);
        ISyntaxItem* NewElseSection(IWebPageData&, int, const TKcElseSent&, TSyntaxItemStack&);
        ISyntaxItem* NewEndIfSection(IWebPageData&, int, const TKcEndIfSent&, TSyntaxItemStack&);
        // 循环语句
        ISyntaxItem* NewWhileSection(IWebPageData&, int, const TKcWhileSent&, TSyntaxItemStack&);
        ISyntaxItem* NewBreakSection(IWebPageData&, int, const TKcBreakSent&, TSyntaxItemStack&);
        ISyntaxItem* NewContinueSection(IWebPageData&, int, const TKcContinueSent&, TSyntaxItemStack&);
        ISyntaxItem* NewEndWhileSection(IWebPageData&, int, const TKcEndWhileSent&, TSyntaxItemStack&);

    private:
        IBundleContext& m_context;
        const IBundle& m_bundle;
        // 标识符的应用
        IServiceReference* m_IDNameRef = nullptr;
        // 页面工厂的应用
        IServiceReference* m_PageFactoryRef = nullptr;
    };
}
