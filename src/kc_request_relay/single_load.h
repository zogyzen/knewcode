#pragma once

#include "std.h"

class CSLWebRequest : public ISLWebRequest
{
public:
    CSLWebRequest(IKCSingleLoadSoCB& cb);
    ~CSLWebRequest(void) override;

    // POST请求
    unsigned POST(const char*& respond, const char* srv, const char* pth, const char* prm, const char* body, int len = 0, const char* cookie = nullptr) override;
    // GET请求
    unsigned GET(const char*& respond, const char* srv, const char* pth, const char* prm, const char* cookie = nullptr) override;
    // 得到cookie
    const char* Cookie(const char* srv, const char* cookie) override;
    const char* Cookie(const char* srv) override;
    // 得到响应头
    const char* RespondHeader(const char* key, const unsigned pos = 0) override;

public:
    typedef httplib::Headers KcHeaders;
    typedef httplib::DataSink DataSink;

    // 转换状态信息
    const char *status_message(int status);
    // 响应头
    void RespondHeader(string sSrv, const KcHeaders&);
    // 日志
    bool WriteLogError(const char* info, const char* place = "", const char* other = "") const;
    bool WriteLogDebug(const char* info, const char* place = "", const char* other = "") const;
    bool WriteLogTrace(const char* info, const char* place = "", const char* other = "") const;

private:
    // 回调接口
    IKCSingleLoadSoCB& m_cb;
    // cookie
    CUtilHttp::TKCCookieManager m_cookieMan;
    // 应答头
    KcHeaders m_respondHeader;
    boost::shared_mutex m_mtxresh;
};
