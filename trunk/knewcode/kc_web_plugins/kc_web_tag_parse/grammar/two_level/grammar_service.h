#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"
#include "grammar_define.h"

namespace KC { namespace grammar { namespace two_level
{
    // 定义
    class CGrammarService : public TGrammarBase<void()>
    {
    public:
        // 构造函数
        CGrammarService(IWebPageData&, ICurrentParsePos&,
                        const one_level::CGrammarBase&, const one_level::CGrammarExpr&, const CGrammarDefine&);

    private:
        // 页面数据
        IWebPageData& m_page;

        // 基本语法
        const one_level::CGrammarBase& m_base;
        // 表达式
        const one_level::CGrammarExpr& m_expr;
        // 定义
        const CGrammarDefine& m_def;

    private:
        /// 定义项规则
        TGrammarRule<void()> rFullSect;
        TGrammarRule<string()> rService;                  // 服务
        TGrammarRule<void()> rFuncDef;                    // 函数定义
        TGrammarRule<void()> rSrvSect;

    private:
        // 设置规则
        void SetRule(void);
    };
}}}
