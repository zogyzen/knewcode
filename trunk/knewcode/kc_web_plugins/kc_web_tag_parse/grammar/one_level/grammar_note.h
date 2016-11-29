#pragma once

#include "grammar_base.h"

namespace KC { namespace grammar { namespace one_level
{
    // ע�⣺���ʱ�ɼ�������ʱ���ɼ�
    class CGrammarNote : public TGrammarBase<void()>
    {
    public:
        CGrammarNote(IWebPageData&, ICurrentParsePos&, const CGrammarBase&);

    private:
        // ҳ������
        IWebPageData& m_page;
        // ��ǰ����λ��
        ICurrentParsePos& m_currPos;

        // �����﷨
        const CGrammarBase& m_base;

    private:
        /// �ؼ���λ�õĹ���
        TGrammarRule<TSynPosAttr()> rKeywordNoteLeft;
        TGrammarRule<TSynPosAttr()> rKeywordNoteRight;

        /// ע�������
        TGrammarRule<void()> rNote;
        TGrammarRule<void()> rLeft;       // ע��ͷ
        TGrammarRule<void()> rRight;      // ע��β
        TGrammarRule<string()> rBody;     // ע������

    private:
        void SetRule(void);
    };
}}}
