#pragma once

namespace KC
{
    class CKCWebWork : public IKCWebWork
    {
    public:
        CKCWebWork(const IBundle& bundle);
        virtual CALL_TYPE ~CKCWebWork();

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 注册异常捕捉信号
        virtual void Signal(void);
        // 展现一个网页：参数是请求和应答接口
        virtual void RenderPage(IWebRequestRespond& re);

    public:
        // 固定响应类型字符串
        const char* FixContentTypeString(const char*);
        // 获取页面工厂的应用
        IServiceReferenceEx& getPageFactoryRef(void);

    private:
        // 展现一个网页（信号捕捉）
        void RenderPageTry(IKCRequestRespondData&);

    private:
        // 插件及上下文
        IBundleContext& m_context;
        const IBundle& m_bundle;
        // 页面工厂的应用
        IServiceReference* m_PageFactoryRef = nullptr;
        // 响应的类型
        std::list<string> m_ResponseContentTypes;
    };
}
