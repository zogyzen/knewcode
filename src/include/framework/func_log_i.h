#pragma once

#include "for_user/kc_object_i.h"

namespace KC
{
    // 日志功能库接口
    class IFuncLog : public IKCObject
    {
    public:
        virtual ~IFuncLog() = default;

        // 日志等级
        enum TLogLevel
        {
            lglvTrace,          // 调试
            lglvDebug,          // 调试
            lglvInfo,           // 信息
            lglvWarning,		// 警告
            lglvError,			// 错误
            lglvFatal           // 致命
        };

    public:
        // 初始化
        virtual const char* Init(const char* dir, const char* id, TLogLevel lv = lglvInfo, int szFile = 2, const char* eachPrefix = "", bool inDay = false) = 0;
        // 释放
        virtual void Free(void) = 0;

        // 写日志
        virtual bool WriteLog(TLogLevel, const char*) const = 0;

        // 提交
        virtual void Flush(void) const = 0;
    };
}
