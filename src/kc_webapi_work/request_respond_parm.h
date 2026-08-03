#pragma once

#include "std.h"

// 控制器的输入和输出参数
class CKCCtrlParmInOut : public IKCCtrlParmInOut
{
public:
    // 参数类型是否json
    void CALL_TYPE SetIsJson(void) override;
    bool CALL_TYPE IsJson(void) const override;
    // 正文类型
    void CALL_TYPE SetContentType(const char* = c_DefaultResponseContentType) override;
    const char* CALL_TYPE GetContentType(void) const override;
    // 头部数据（针对web请求的应答）
    void CALL_TYPE SetHeader(const char*, const char*) override;
    const char* CALL_TYPE GetHeader(const char*, const char*) const override;
    // 内容数据
    void CALL_TYPE SetContent(const char*, const unsigned) override;
    const char* CALL_TYPE GetContent(void) const override;
    unsigned CALL_TYPE GetContentLength(void) const override;

    // 是否区分大小写
    bool CALL_TYPE JsonCaseSensitive(void) const override;
    // 字符集
    const char* CALL_TYPE GetCharset(void) const override;

    // 源最终的错误码和错误信息
    int CALL_TYPE GetErrCode(void) const override;
    const char* CALL_TYPE GetErrMsg(void) const override;

public:
    CKCCtrlParmInOut(ICoreParmJson&, string nameErrCode = c_RESTful_errCode, string nameErrMsg = c_RESTful_errMsg, string charset = c_RESTful_UTF8, bool jsonCaseSensitive = false);

    // 应答头
    typedef std::map<string, string> THeader;
    THeader& Header(void) { return m_header; }

    // json格式的正文类型
    string JsonContentType(void) { return m_jsonContentType; }

    // 固定的参数字段名称：错误码，错误信息，输入参数
    const string m_nameErrCode = c_RESTful_errCode;
    const string m_nameErrMsg = c_RESTful_errMsg;

    // 字符集（源）
    const string m_charset = c_RESTful_UTF8;
    // 设置json是否区分大小写（源）
    const bool m_jsonCaseSensitive = true;

private:
    ICoreParmJson& m_own;
    bool m_isJson = true;
    const string m_jsonContentType;
    string m_contentType = c_DefaultResponseContentType;
    string m_content;
    THeader m_header;
};
typedef std::shared_ptr<CKCCtrlParmInOut> CKCParmRespondPtr;
typedef std::tuple<IKCJsonCore::IJsonCorePtr, CKCParmRespondPtr> TCtrlCallResult;
