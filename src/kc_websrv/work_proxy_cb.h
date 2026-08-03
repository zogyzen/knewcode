#pragma once

#include "std.h"

class CWebSrvWork;
// 请求和应答回调类
class CWSProxyRequestCB : public IWSProxyRequestCB
{
public:
    CWSProxyRequestCB(CWebSrvWork&, IKCWebSrvProxy&, KCSrv::KcSrvRespondPtr);
    ~CWSProxyRequestCB(void) override;

    // 设置请求应答接口
    void CALL_TYPE SetRe(ISrcRequestRespond&) override;
    // 得到本地完整文件名
    const char* CALL_TYPE GetLocalFilename(void) const override;
    const char* CALL_TYPE GetLocalFilename(const char*) const override;
    // 得到网络根路径
    const char* CALL_TYPE GetUrlPageRootPath(const char*) const override;
    // 虚拟目录
    unsigned CALL_TYPE VirtualPathCount(void) const override;
    const char* CALL_TYPE GetVirtualPath(unsigned) const override;
    const char* CALL_TYPE GetVirtualPathUri(unsigned) const override;
    // 判断是否ssl
    bool CALL_TYPE IsSSL(void) const override;
    // 得到服务器端各单个请求信息
    const char* CALL_TYPE GetSingleInfo(const char* pName, const char* pDef = "") const override;
    // 是否post请求
    bool CALL_TYPE IsPost(void) const override;
    // 按名称得到请求头
    const char* CALL_TYPE GetRequestHeader(const char* pName) const override;
    // 获取客户端post输入
    unsigned CALL_TYPE GetClientBlock(char*, unsigned) const override;
    // 设置应答类型
    void CALL_TYPE SetContentType(const char*) override;
    // 设置响应状态
    void CALL_TYPE SetResponseStatus(int iStt) override;
    // 按名称添加、删除响应头
    void CALL_TYPE AddResponseHeader(const char* name, const char* val) override;
    void CALL_TYPE DelResponseHeader(const char* name) override;
    // 输出网页内容
    bool CALL_TYPE AddResponseBody(const char* buf, int nbyte = 0) override;
    // 提交响应（将这段时间，服务器端的输出，同时推给客户端）
    void CALL_TYPE CommitResponse(void) override;

protected:
    CWebSrvWork& m_own;
    IKCWebSrvProxy& m_proxy;
    ISrcRequestRespond *m_srcRR = nullptr;
    KCSrv::KcSrvRespondPtr m_res;

    friend class CWSProxyServerCB;
};
