#pragma once

namespace KC
{
    class CParseWork : public IParseWork
    {
    public:
        CParseWork(const IBundle& bundle);
        virtual CALL_TYPE ~CParseWork(void);

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 解析：参数为页数据
        virtual bool Parse(IWebPageData& page);

    private:
        IBundleContext& m_context;
        const IBundle& m_bundle;
    };
}
