#pragma once

#include "std.h"

class KCWebSrvProxy;

// 请求和应答
class CWebRequestRespond : public ISrcRequestRespond
{
public:
    CWebRequestRespond(KCWebSrvProxy&, IWSProxyRequestCB&, IWSProxyServerCB&);
    ~CWebRequestRespond() override;

    // 是否子调用
    bool IsSubCall(void) override { return false; }

    // 得到服务器端全部信息
    const char* CALL_TYPE GetAllInfo(const char* = "\n") override;
    // 得到服务器端各单个请求信息
    const char* CALL_TYPE GetSingleInfo(const char* = "the_request", const char* = "") override;

    ////////////////////////////请求部分///////////////////////////
    // 得到本地完整文件名
    const char* CALL_TYPE GetLocalFilename(void) override;
    const char* CALL_TYPE GetLocalFilename(const char*) override;
    // 得到网络文件名
    const char* CALL_TYPE GetUriFilename(void) override;
    // 得到网络路径
    const char* CALL_TYPE GetUrlPagePath(void) override;
    // 得到网络根路径
    const char* CALL_TYPE GetUrlPageRootPath(void) override;
    const char* CALL_TYPE GetUrlPageRootPath(const char*) override;
    // 端口
    int CALL_TYPE GetPort(void) override;
    // GET的参数
    const char* CALL_TYPE GetGetArgStr(void) override;
    // Post的参数
    const char* CALL_TYPE GetPostArgType(void) override;
    int CALL_TYPE GetPostArgLength(void) override;
    int CALL_TYPE GetPostArgBuffer(char*&, int) override;
    const char* CALL_TYPE GetPostArgStr(void) override;
    // 得到网站本地完整根目录
    const char* CALL_TYPE GetLocalRootPath(void) override;
    // 按名称得到请求头
    const char* CALL_TYPE GetRequestHeader(const char*) override;
    // 虚拟目录
    unsigned CALL_TYPE VirtualPathCount(void) override;
    const char* CALL_TYPE GetVirtualPath(unsigned) override;
    const char* CALL_TYPE GetVirtualPathUri(unsigned) override;

    ////////////////////////////应答部分///////////////////////////
    // 响应文本类型
    bool CALL_TYPE SetResponseContentType(const char*) override;
    const char* CALL_TYPE GetResponseContentType(void) override;
    // 响应状态
    bool CALL_TYPE SetResponseStatus(int = 200) override;
    // 按名称添加、删除响应头
    void CALL_TYPE AddResponseHeader(const char*, const char*) override;
    void CALL_TYPE DelResponseHeader(const char*) override;
    // 输出网页内容
    bool CALL_TYPE AddResponseBody(const char*, int = 0) override;
    // 提交响应（将这段时间，服务器端的输出，同时推给客户端）
    void CALL_TYPE CommitResponse(void) override;

public:
    // 判断是否ssl
    bool IsSSL(void);

protected:
    KCWebSrvProxy& m_own;
    IWSProxyRequestCB& m_recb;
    IWSProxyServerCB& m_srv;
    string m_respond_type;  // 返回类型
    std::shared_ptr<char> m_postBuf;       // post缓冲区

    friend class KCWebSrvProxy;
    friend class TKCWebConn;
};
