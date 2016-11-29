#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"
#include "grammar_define.h"

namespace KC { namespace grammar { namespace two_level
{
    // 定义
    class CGrammarFuncLib : public TGrammarBase<void()>
    {
    public:
        // 构造函数
        CGrammarFuncLib(IWebPageData&, ICurrentParsePos&,
                        const one_level::CGrammarBase&, const one_level::CGrammarExpr&, const CGrammarDefine&);

    private:
        // 页面数据
        IWebPageData& m_page;
        // 当前解析位置
        ICurrentParsePos& m_currPos;

        // 基本语法
        const one_level::CGrammarBase& m_base;
        // 表达式
        const one_level::CGrammarExpr& m_expr;
        // 定义
        const CGrammarDefine& m_def;

    private:
        /// 关键字位置的规则
        TGrammarRule<TSynPosAttr()> rKeywordInterface;      // 接口关键字
        TGrammarRule<TSynPosAttr()> rKeywordLoadMod;        // 加载关键字

        /// 定义项规则
        TGrammarRule<void()> rFullSect;
        TGrammarRule<TKcParmFL()> rParm;                    // 形参
        TGrammarRule<TKcParmsFL()> rParms;                  // 形参列表
        TGrammarRule<TKcFuncDefFL()> rFunction;             // 函数声明
        TGrammarRule<TKcFuncDefsFL()> rFunctions;           // 函数声明列表
        TGrammarRule<TKcInfFL()> rInterface;                // 接口定义
        TGrammarRule<TKcInfFullFL()> rInterfaceFull;        // 完整接口定义
        TGrammarRule<TKcLoadFL()> rLoadMod;                 // 模块加载
        TGrammarRule<TKcLoadFullFL()> rLoadModFull;         // 完整模块加载

    private:
        // 设置确定关键字位置的规则
        void SetRulePos(void);
        // 设置规则
        void SetRule(void);
    };
}}}
