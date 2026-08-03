#pragma once

#include "std.h"

// 日志功能库
class CBoostLog : public IFuncLog
{
public:
    CBoostLog(IBundleContext&);
    ~CBoostLog() override = default;

    // 初始化
    const char* Init(const char* dir, const char* id, TLogLevel lv = lglvInfo, int szFile = 2, const char* eachPrefix = "", bool inDay = false) override;
    // 释放
    void Free(void) override;

    // 写日志
    bool WriteLog(TLogLevel, const char*) const override;

    // 提交
    void Flush(void) const override;

private:
    IBundleContext& m_own;
    string m_path;
    // 日志
    MyLog m_log;
};
