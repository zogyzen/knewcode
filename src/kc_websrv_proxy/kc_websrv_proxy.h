#pragma once

#include "std.h"

class KCWebSrvProxy : public IKCWebSrvProxy
{
public:
    KCWebSrvProxy(IWSProxyServerCB&);
    ~KCWebSrvProxy() override;

    // 初始化
    void CALL_TYPE Init(void) override;
    void CALL_TYPE Free(void) override;

    // 宿主名称
    const char* CALL_TYPE OwnName(void) const override
    {
        return m_srvCB.OwnName();
    }

    // 宿主版本
    const char* CALL_TYPE OwnVersion(void) const override
    {
        return m_srvCB.OwnVersion();
    }

    // 打包Web链接
    IKCWebLongConn& CALL_TYPE PackWebConn(IBaseRequestRespond&, EWebConnType) override;
    // 释放web连接
    void CALL_TYPE ReleaseWebConn(IKCWebLongConn&) override;

    // 虚拟目录
    unsigned CALL_TYPE VirtualPathCount(void) override;
    const char* CALL_TYPE GetVirtualPath(unsigned) override;
    const char* CALL_TYPE GetVirtualPathUri(unsigned) override;
    // 得到url对应的本地完整文件名
    const char* CALL_TYPE GetUrlLocalPath(const char*) override;
    // 得到网页根路径
    const char* CALL_TYPE GetUrlPageRootPath(const char*) override;

    // 得到网站或应用根路径
    const char* CALL_TYPE GetWebsiteRootPath(void) override;
    // 得到主平台根路径
    const char* CALL_TYPE GetPlatformRootPath(void) override;
    // 得到web服务程序或应用程序的根目录
    const char* CALL_TYPE GetApPath(void) override;

    // 处理请求
    void CALL_TYPE Work(IWSProxyRequestCB&) override;
    // 静态页面处理
    void CALL_TYPE StaticPage(IWSProxyRequestCB&) override;
    // 链接关闭前
    void CALL_TYPE PreClose(IWSProxyConnectCB&) override;
    // 得到模块上下文
    IBundleContext& GetContext(void) const override;

public:
    // 获取链接的弱指针
    boost::weak_ptr<IKCWebLongConn> GetConnWPtr(long);
    // web服务回调接口
    IWSProxyServerCB& SrvCB(void);

private:
    // 是否运行
    std::atomic_bool m_running;
    // web服务回调接口
    IWSProxyServerCB& m_srvCB;
    // 主框架
    BundleContextHelper m_contHelp;
    // 主插件上下文
    IBundleContext &m_BundleContextIF;
    // 主应用引用
    IServiceReferenceEx &m_WebMainRef;
    // WebApi后端引用
    IServiceReferenceEx &m_WebApiWrkRef;
    // 虚拟目录
    map<string, string> m_vPath;
    vector<string> m_vPathVct;
    // 需保存的持久链接
    boost::shared_mutex m_mtxWC;
    std::map<long, boost::shared_ptr<IKCWebLongConn>> m_wcs;

    friend class CWebRequestRespond;
};
