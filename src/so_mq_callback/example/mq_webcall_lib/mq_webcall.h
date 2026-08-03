#pragma once

#include "std.h"

// 日志功能库
class CWebCallMQ : public IWebCallMQ
{
public:
    CWebCallMQ(ICallback& cb, std::string path);

    // 初始化/释放
    const char* init(const char* sMQName, const unsigned MQSize = 256, const unsigned MsgSize = 2048) override;
    void free(void) override;

    // 错误日志
    void WriteErr(const char*, const char*, const char* = "");

protected:
    ICallback& m_cb;
    MQCallbackHelper<CWebCallMQ> m_mqHelper;
};
