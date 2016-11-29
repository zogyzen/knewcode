#pragma once

namespace KC { namespace grammar { namespace one_level
{
    // 基础语法
    class CGrammarBase : public TGrammarBase<void()>
    {
    public:
        /// 符号位置的规则
        TGrammarRule<TSynPosAttr()> rSymbHead, rSymbTail;                 // 标记头、尾
        TGrammarRule<TSynPosAttr()> rKeyBegin;                            // 关键字开始
        TGrammarRule<TSynPosAttr()> rIDNameBegin;                         // 标识符开始
        TGrammarRule<TSynPosAttr()> rIDNameInnerBegin;                    // 内部标识符开始
        TGrammarRule<TSynPosAttr()> rMemberBegin;                         // 成员引用开始
        TGrammarRule<TSynPosAttr()> rSentEnd;                             // 语句结束
        TGrammarRule<TSynPosAttr()> rCommaSpilit;                         // 逗号分割
        TGrammarRule<TSynPosAttr()> rCommaColon;                          // 冒号分割
        TGrammarRule<TSynPosAttr()> rDbQuotesRange;                       // 双引号范围
        TGrammarRule<TSynPosAttr()> rSymbolYKHLeft, rSymbolYKHRight;      // 圆括号
        TGrammarRule<TSynPosAttr()> rSymbolArrayLeft, rSymbolArrayRight;  // 方括号
        TGrammarRule<TSynPosAttr()> rEvaluateSpilit;                      // 赋值符号分割
        TGrammarRule<TSynPosAttr()> rSymbolSubt;                          // 减号
        TGrammarRule<TSynPosAttr()> rSymbolMore, rSymbolLess;             // 尖括号
        TGrammarRule<TSynPosAttr()> rAppendSpilit;                        // 添加符号分割
        TGrammarRule<TSynPosAttr()> rSymbolRef;                           // 引用符号
        TGrammarRule<TSynPosAttr()> rOptionPrivate;                       // 私有定义选项
        TGrammarRule<TSynPosAttr()> rOptionReference;                     // 引用定义选项
        TGrammarRule<TSynPosAttr()> rOptionStatic;                        // 静态定义选项

        /// 关键字位置的规则
        TGrammarRule<TSynPosAttr()> rKeywordVoid;
        TGrammarRule<TSynPosAttr()> rKeywordInterface;
        TGrammarRule<TSynPosAttr()> rKeywordBool;
        TGrammarRule<TSynPosAttr()> rKeywordInt;
        TGrammarRule<TSynPosAttr()> rKeywordDouble;
        TGrammarRule<TSynPosAttr()> rKeywordStr;
        TGrammarRule<TSynPosAttr()> rKeywordTrue;
        TGrammarRule<TSynPosAttr()> rKeywordFalse;

        /// 基本规则
        TGrammarRule<string()> rCommName;                                 // 名称
        TGrammarRule<string()> rCommStrText;                              // 字符串值
        TGrammarRule<TKcStringSyn()> rCommStr;                            // 字符串
        TGrammarRule<TKcIDNameSyn()> rCommIDName;                         // 标识符
        TGrammarRule<TKcMemberSyn()> rCommMember;                         // 成员引用
        TGrammarRule<TKcDataTypeSyn()> rCommDataType;                     // 数据类型
        TGrammarRule<TKcConstBoolSyn()> rCommConstBool;                   // 布尔型常量
        TGrammarRule<TKcConstSyn()> rCommConst;                           // 常量

    public:
        // 构造函数
        CGrammarBase(IWebPageData&, ICurrentParsePos&);

    private:
        // 页面数据
        IWebPageData& m_page;
        // 当前解析位置
        ICurrentParsePos& m_currPos;

        // 未使用规则
        TGrammarRule<void()> rUnuse;

    private:
        // 设置确定语法位置的规则
        void SetRulePos(void);
        // 设置基本规则
        void SetRuleCommon(void);
    };
}}}
