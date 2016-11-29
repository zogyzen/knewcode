#pragma once

#include "syntax_item/syntax_item_if.h"

namespace KC
{
    // 语法项管理接口
    class CSyntaxItemWork : public ISyntaxItemWork
    {
    public:
        CSyntaxItemWork(IWebPageData&);
        virtual ~CSyntaxItemWork() = default;

        /// 添加语法项
        // 添加HTML
        virtual void AddHtmlTextSyntax(string);
        // 添加include
        virtual void AddIncludeSyntax(const TKcWebExpr&);
        // 添加变量定义
        virtual void AddVarDefSyntax(const TKcWebVarDef&);
        // 添加变量赋值
        virtual void AddVarAssignSyntax(const TKcWebVarAss&);
        // 添加内部变量赋值
        virtual void AddInnerVarAssSyntax(const TKcWebInnerVarAss&);
        // 添加表达式运算
        virtual void AddExprSyntax(const TKcWebExpr&);
        // 添加事件
        virtual void AddEventDefSyntax(const TKcEventDef&);

        /// 添加函数组定义
        // 添加函数库（load、interface等）
        virtual void AddLoadSyntax(const TKcWebLoadSect&);

        /// 添加结构语句
        // 添加输出语句
        virtual void AddPrintSection(const TKcWebExpr&);
        // 添加条件语句
        virtual void AddIfSection(const TKcWebExpr&);
        virtual void AddElseIfSection(const TKcWebExpr&);
        virtual void AddElseSection(void);
        virtual void AddEndIfSection(void);
        // 添加循环语句
        virtual void AddWhileSection(const TKcWebExpr&);
        virtual void AddBreakSection(const TKcWebExpr&);
        virtual void AddContinueSection(const TKcWebExpr&);
        virtual void AddEndWhileSection(void);
        // 退出语句
        virtual void AddExitSection(const TKcWebArgms&);

    public:
        // 获取语法项
        ISyntaxItem* GetSyntaxItem(int);

    private:
        // 添加语法条款
        void AddSyntaxItem(ISyntaxItem*);
        template<typename SYNTAX> SYNTAX* AddSyntaxItem(void);
        template<typename SYNTAX, typename DATA> SYNTAX* AddSyntaxItem(const DATA&);

    private:
        IWebPageData& m_WebPageData;
        // 语法项队列
        typedef boost::shared_ptr<ISyntaxItem> TSyntaxItemPtr;
        typedef vector<TSyntaxItemPtr> TSyntaxItemList;
        TSyntaxItemList m_SyntaxItemList;
        // 条件和循环语句栈（用于配对）
        typedef vector<ISyntaxItem*> TSyntaxItemStack;
        TSyntaxItemStack m_SyntaxItemStack;
    };
}
