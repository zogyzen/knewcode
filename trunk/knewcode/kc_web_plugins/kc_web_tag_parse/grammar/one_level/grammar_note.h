#pragma once

#include "grammar_base.h"

namespace KC { namespace grammar { namespace one_level
{
    // 注解：设计时可见，运行时不可见
    class CGrammarNote : public TGrammarBase<void()>
    {
    public:
        CGrammarNote(IWebPageData&, ICurrentParsePos&, const CGrammarBase&);

    private:
        // 页面数据
        IWebPageData& m_page;
        // 当前解析位置
        ICurrentParsePos& m_currPos;

        // 基础语法
        const CGrammarBase& m_base;

    private:
        /// 关键字位置的规则
        TGrammarRule<TSynPosAttr()> rKeywordNoteLeft;
        TGrammarRule<TSynPosAttr()> rKeywordNoteRight;

        /// 注解项规则
        TGrammarRule<void()> rNote;
        TGrammarRule<void()> rLeft;       // 注解头
        TGrammarRule<void()> rRight;      // 注解尾
        TGrammarRule<string()> rBody;     // 注解内容

    private:
        void SetRule(void);
    };
}}}
