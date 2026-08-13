#pragma once

#include "std.h"

class CWSProxyServerCB;
// 内置web服务器 工作类
class CWebSrvWork : public std::enable_shared_from_this<CWebSrvWork>
{
public:
    CWebSrvWork(string, string);
    ~CWebSrvWork(void) = default;

    // 初始化
    void Init(void);
    // 释放
    void Free(void);

    // 处理请求
    void Work(KCSrv::KcSrvRespondPtr res);

private:
    // 主页配置
    struct TMainPageCfg
    {
        string m_sysName, m_rootPath, m_indexPage;
        map<int, string> m_errPage;
        map<string, string> m_vPath;
        vector<string> m_vPathVct;

        // 获取本地路径
        string GetLocalPath(string sUri) const
        {
            string sPageFile;
            // 主页
            if ("/" == sUri || sUri.empty())
                sPageFile = m_rootPath + "/" + m_indexPage;
            // 其他文件
            else
            {
                // 先匹配虚拟路径
                sPageFile = CUtilFunc::TransVPathToLocal(m_vPath, sUri);
                // 网站根路径
                if (sPageFile.empty()) sPageFile = m_rootPath + sUri;
                boost::algorithm::trim_right_if(sPageFile, boost::is_any_of("/\\"));
                // 子目录
                if (sUri.back() == '/' && (boost::filesystem::exists(sPageFile) && boost::filesystem::is_directory(sPageFile)))
                    sPageFile += "/" + m_indexPage;
                // 去掉路径中的冗余（..和.）
                if (boost::filesystem::exists(sPageFile))
                    sPageFile = boost::filesystem::canonical(sPageFile).string();
            }
            return sPageFile;
        }
    };
    // 虚拟主机
    struct TVirtualHost
    {
        string m_name, m_domainName;
        TMainPageCfg m_pageCfg;
    };
    TVirtualHost m_mainHost;
    map<string, TVirtualHost> m_virtualHost;

private:
    // 可执行文件和参数
    const string m_exePath, m_exeParm;
    // 内置web服务器软件名称和版本、可执行文件根目录、KC框架路径、配置文件、网站或应用根路径、平台根路径、主页目录、虚拟目录
    const string m_ownName, m_ownVersion, m_MainExecPath, m_FxPath, m_CfgFile, m_WebsitePath, m_PlatformPath, m_PgPath, m_virtualPath;
    // 禁止访问文件的扩展名
    std::set<string> m_stDeniedUrlExtName;

    // 内置web服务器
    KCSrv::KcSrvMainExecPtr<CWebSrvWork> m_kcSrv;

    // 友元
    friend class CWSProxyServerCB;
    friend class CWSProxyConnectCB;
    friend class CWSProxyRequestCB;
    friend void WriteLog(string sFlag, string sErr, string sPos);

    // kc_websrv服务类
    CWSProxyServerCB &m_srvCB;
    // 代理接口
    LoadWebSrvProxy m_load;
    IKCWebSrvProxy& m_proxy;
};
extern std::shared_ptr<CWebSrvWork> g_work;     // 主框架
extern void WriteLog(string sFlag, string sErr, string sPos);   // 写日志函数
