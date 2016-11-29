#pragma once

#include "../one_level/grammar_base.h"
#include "../one_level/grammar_expr.h"
#include "grammar_define.h"

namespace KC { namespace grammar { namespace two_level
{
    // ����
    class CGrammarFuncLib : public TGrammarBase<void()>
    {
    public:
        // ���캯��
        CGrammarFuncLib(IWebPageData&, ICurrentParsePos&,
                        const one_level::CGrammarBase&, const one_level::CGrammarExpr&, const CGrammarDefine&);

    private:
        // ҳ������
        IWebPageData& m_page;
        // ��ǰ����λ��
        ICurrentParsePos& m_currPos;

        // �����﷨
        const one_level::CGrammarBase& m_base;
        // ���ʽ
        const one_level::CGrammarExpr& m_expr;
        // ����
        const CGrammarDefine& m_def;

    private:
        /// �ؼ���λ�õĹ���
        TGrammarRule<TSynPosAttr()> rKeywordInterface;      // �ӿڹؼ���
        TGrammarRule<TSynPosAttr()> rKeywordLoadMod;        // ���عؼ���

        /// ���������
        TGrammarRule<void()> rFullSect;
        TGrammarRule<TKcParmFL()> rParm;                    // �β�
        TGrammarRule<TKcParmsFL()> rParms;                  // �β��б�
        TGrammarRule<TKcFuncDefFL()> rFunction;             // ��������
        TGrammarRule<TKcFuncDefsFL()> rFunctions;           // ���������б�
        TGrammarRule<TKcInfFL()> rInterface;                // �ӿڶ���
        TGrammarRule<TKcInfFullFL()> rInterfaceFull;        // �����ӿڶ���
        TGrammarRule<TKcLoadFL()> rLoadMod;                 // ģ�����
        TGrammarRule<TKcLoadFullFL()> rLoadModFull;         // ����ģ�����

    private:
        // ����ȷ���ؼ���λ�õĹ���
        void SetRulePos(void);
        // ���ù���
        void SetRule(void);
    };
}}}
