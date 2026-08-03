#pragma once

#include "std.h"

// “转发”控制器
class CCtrlRelay : public IKCController
{
public:
    CCtrlRelay(IKCRelayREST& own, std::string sName, boost::property_tree::ptree&);
    ~CCtrlRelay() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

public:
    typedef httplib::Headers KcHeaders;
    typedef httplib::DataSink DataSink;

    // 回调函数
    const char *status_message(int status);
    bool WriteLogError(const char* info, const char* place = "", const char* other = "") const;
    bool WriteLogDebug(const char* info, const char* place = "", const char* other = "") const;
    bool WriteLogTrace(const char* info, const char* place = "", const char* other = "") const;
    void RespondHeader(string sSrv, const KcHeaders&);

public:
    // 解析cookie
    static void ParseCookie(string sSrv, const httplib::Headers&, CUtilHttp::TKCCookieManager&);
    // 生成cookie
    static void MakeCookie(string sSrv, string sPth, string &sClnCookies, httplib::Headers&, CUtilHttp::TKCCookieManager&);

protected:
    // 请求
    string PostGet(ICtrlApiData&, map<string, string>&, bool isPost = false);
    // 生成请求头
    void MakeRequestHeaders(KcHeaders&, ICtrlApiData&, map<string, string>&, string, string);

private:
    // 宿主
    IKCRelayREST& m_own;
    string m_name;
    string m_server;
    // cookie
    CUtilHttp::TKCCookieManager m_cookieMan;
};

// 请求
template<typename TCallBack>
string _PostGet(TCallBack& cb, typename TCallBack::KcHeaders &headers, string sSrv, string sPth, string sPrm, string sBody, bool isPost, unsigned timeout = 30)
{
    // 分解
    string sHost = "127.0.0.1", sProto = "http";
    int iPort = 0;
    bool isSSL = CUtilHttp::splitUrl(sSrv, sProto, sHost, iPort);
    // 请求
    return isSSL ? CUtilHttp::httplibPostGet<httplib::SSLClient>(cb, headers, sBody, sSrv, sHost, iPort, sPth, sPrm, isPost, isSSL, timeout)
                 : CUtilHttp::httplibPostGet<httplib::Client>(cb, headers, sBody, sSrv, sHost, iPort, sPth, sPrm, isPost, isSSL, timeout);
}
