#pragma once

namespace KC
{
    class CCurrentParsePos : public grammar::ICurrentParsePos
    {
    public:
        CCurrentParsePos(IWebPageData&);
        virtual ~CCurrentParsePos(void);

        // 设置当前解析的位置
        virtual void Set(TSynSectInBuf&, const TSynPosAttr&);
        virtual void SetPos(const TSynPosAttr&, int);

        // 得到当前解析位置
        const TSynPosAttr& Get(void);
        int GetSynID(void);

    private:
        void SetPosBase(const TSynPosAttr&, int);

    private:
        // 页接口
        IWebPageData& m_page;
        // 当前位置、当前语法编号
        TSynPosAttr m_pos;
        int m_SynID = 0;
    };
}
