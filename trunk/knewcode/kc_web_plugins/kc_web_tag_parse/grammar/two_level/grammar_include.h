#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"

namespace KC { namespace grammar { namespace two_level
{
    // 定义
    class CGrammarInclude : public TGrammarBase<void()>
    {
    public:
        // 构造函数
        CGrammarInclude(IWebPageData&, ICurrentParsePos&,
                        const one_level::CGrammarBase&, const one_level::CGrammarExpr&);

    private:
        // 页面数据
        IWebPageData& m_page;
        // 当前解析位置
        ICurrentParsePos& m_currPos;

        // 基本语法
        const one_level::CGrammarBase& m_base;
        // 表达式
        const one_level::CGrammarExpr& m_expr;

    private:
        /// 关键字位置的规则
        TGrammarRule<TSynPosAttr()> rKeywordInclude;      // 包含关键字

        /// 定义项规则
        TGrammarRule<void()> rSect;
        TGrammarRule<TKcIncludeCL()> rInclude;            // 页面引用

    private:
        // 设置确定关键字位置的规则
        void SetRulePos(void);
        // 设置规则
        void SetRule(void);
    };
}}}
