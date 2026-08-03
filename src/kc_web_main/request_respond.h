#pragma once

#include "std.h"

// 请求和应答
class CKCRequestRespond : public IKCRequestRespondData
{
private:
    // multipart/form-data
    struct TMultiFormData : public IMultiFormData
    {
        TMultiFormData(string, string);
        ~TMultiFormData() override;

        // 得到头
        const char* CALL_TYPE GetHeader(const char*) override;
        // 得到体
        const char* CALL_TYPE GetBody(unsigned&) override;
        // 是否文件
        bool CALL_TYPE isFile(void) override;

        // 数据
        bool m_isFile = false;
        string m_body;
        map<string, string> m_header;
    };

public:
    ISrcRequestRespond& getRe(void) const override;

    // 锁
    IKcLockWork& CALL_TYPE LockWork(void) override;
    // 超时时间
    int CALL_TYPE GetTimeOutSeconds(void) const override;
    // 执行中的页面数据接口
    IActionData* CALL_TYPE GetActionData(void) override;
    void CALL_TYPE SetActionData(IActionData* = nullptr) override;
    IActionData& CALL_TYPE ActionData(void) override;

    ////////////////////////////请求部分///////////////////////////
    // 得到服务器端全部信息
    const char* CALL_TYPE GetAllInfo(const char* = "\n") override;
    // 得到服务器端各单个请求信息
    const char* CALL_TYPE GetSingleInfo(const char* = "the_request", const char* = "") override;
    // 得到本地完整文件名
    const char* CALL_TYPE GetLocalFilename(void) override;
    const char* CALL_TYPE GetLocalFilename(const char*) override;
    const char* CALL_TYPE GetLocalPath(const char*) override;
    // 规范化url路径（去除“..”和“.”）
    const char* CALL_TYPE CanonicalUrl(const char*) override;
    // 得到网络文件名
    const char* CALL_TYPE GetUriFilename(void) override;
    // 得到网络路径
    const char* CALL_TYPE GetUrlPagePath(void) override;
    // 得到网络根路径
    const char* CALL_TYPE GetUrlPageRootPath(void) override;
    const char* CALL_TYPE GetUrlPageRootPath(const char*) override;
    // 协议
    const char* CALL_TYPE GetProtocol(void) override;
    // 端口
    int CALL_TYPE GetPort(void) override;
    // GET的参数
    const char* CALL_TYPE GetGetArgStr(void) override;
    const char* CALL_TYPE GetGetArg(const char*, const char* = "") override;
    // Post的参数
    const char* CALL_TYPE GetPostArgType(void) override;
    int CALL_TYPE GetPostArgLength(void) override;
    int CALL_TYPE GetPostArgBuffer(char*&, int) override;
    const char* CALL_TYPE GetPostArgStr(void) override;
    // multipart/form-data参数
    bool CALL_TYPE IsMultipartFormData(void) override;
    unsigned CALL_TYPE GetMultiFormDataCount(void) override;
    IMultiFormData& CALL_TYPE GetMultiFormData(void) override;
    void CALL_TYPE SetCurrentMultiFormData(unsigned) override;
    // 得到网站本地完整根目录
    const char* CALL_TYPE GetLocalRootPath(void) override;
    // 得到当前页本地完整根目录
    const char* CALL_TYPE GetLocalPagePath(void) override;
    // 按名称得到请求头
    const char* CALL_TYPE GetRequestHeader(const char*) override;
    // 得到Cookie
    const char* CALL_TYPE GetCookieVal(const char*) override;
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
    // 按名称添加响应头
    void CALL_TYPE AddResponseHeader(const char*, const char*) override;
    void CALL_TYPE DelResponseHeader(const char*) override;
    // 输出网页内容
    bool CALL_TYPE AddResponseBody(const char*, int = 0) override;
    // 提交响应（将这段时间，服务器端的输出，同时推给客户端）
    void CALL_TYPE CommitResponse(void) override;
    // 添加Cookie
    bool CALL_TYPE AddCookie(const char*, const char*, const char* = "", const char* = "/", const char* = "") override;

    ////////////////////////请求过程中的数据///////////////////////
    // 判断某名称数据是否存在
    bool CALL_TYPE ExistsReStepData(const char*) override;
    // 得到某名称数据
    IReStepData& CALL_TYPE GetReStepData(const char*) override;
    // 添加某名称数据
    bool CALL_TYPE AddReStepData(const char*, IReStepData*&) override;

public:
    CKCRequestRespond(ISrcRequestRespond&, IKCWebWork&);
    ~CKCRequestRespond() override;
    // 服务接口
    IKCWebWork& GetWork(void);
    // 释放
    void Release(void);

private:
    // 运行顺序号
    static inline unsigned long long m_runCount = 0;
    unsigned long long m_runIndex = 0;
    // 主工作类
    IKCWebWork& m_webWork;
    // 源request和respond
    ISrcRequestRespond& m_re;
    // 系统的活动数据
    IActionData* m_ActionData = nullptr;
    // 页面文件本地物理路径
    string m_LocalPagePath;
    // 协议
    string m_Protocol;
    // GET参数
    map<string, string> m_getParms;
    // Cookie
    map<string, string> m_cookies;
    // 请求过程中的数据
    typedef boost::shared_ptr<IReStepData> TReStepDataPtr;
    typedef map<string, TReStepDataPtr> TReStepDataPtrList;
    TReStepDataPtrList m_ReStepDataPtrList;
    // multipart/form-data
    bool m_IsMultipartFormData = false;
    unsigned m_posMultiFormData = 0;
    string m_multiFormDataBoundary;
    vector<boost::shared_ptr<TMultiFormData>> m_multiFormData;
};
