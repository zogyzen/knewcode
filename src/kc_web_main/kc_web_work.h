#pragma once

#include "std.h"

class CKCWebWork : public IKCWebWork
{
public:
    CKCWebWork(const IBundle& bundle);
    ~CKCWebWork() override;

    // 得到服务特征码
    const char* CALL_TYPE getGUID(void) const override;
    // 对应的模块
    const IBundle& CALL_TYPE getBundle(void) const override;

    // 展现一个网页：参数是请求和应答接口
    int CALL_TYPE RenderKC(ISrcRequestRespond& re) override;
    // 静态页面处理
    void CALL_TYPE StaticPage(ISrcRequestRespond&) override;

public:
    // 初始化/释放
    bool init(void);
    bool free(void);

    // 开始/停止
    bool start(void)
    {
        return true;
    }
    bool stop(void)
    {
        return true;
    }

    // 固定响应类型字符串
    const char* FixContentTypeString(const char*);

private:
    // 插件及上下文
    IBundleContext& m_context;
    const IBundle& m_bundle;
    // webapi引用
    IServiceReference &m_WebApiRef;
    // 响应的类型
    std::list<string> m_ResponseContentTypes;
    // 配置中的响应头信息
    std::map<string, string> m_CfgHeader;
};
