#pragma once

#include "std.h"

// 日志功能库
class CMyLog : public IFuncLog
{
public:
    CMyLog(IBundleContext& own);
    ~CMyLog() override = default;

    // 初始化
    const char* Init(const char* dir, const char* id, TLogLevel lv = lglvInfo, int szFile = 2, const char* eachPrefix = "", bool inDay = false) override;
    // 释放
    void Free(void) override;

    // 写日志
    bool WriteLog(TLogLevel, const char*) const override;

    // 提交
    void Flush(void) const override;

protected:
    // 线程
    void RunThrd(void);
    // 将日志写入文件
    void WriteLogToFile(void) const;

private:
    IBundleContext& m_own;
    string m_modPath;
    // 开启运行
    static std::atomic_bool s_running;
    // 线程
    std::shared_ptr<boost::thread> m_runThrd;
    mutable boost::mutex m_mtxLog, m_mtxWrFile;
    // 日志的初始化参数
    string m_logPath, m_logID, m_logEachPrefix;
    TLogLevel m_logLV = lglvInfo;
    int m_logSizeFile = 2;
    bool m_logInDay = false;

private:
    // 日志数据
    struct TLogData
    {
        // 时间
        boost::posix_time::ptime m_dt = boost::posix_time::microsec_clock::local_time();
        // 线程编号
        int m_thrdID = CUtilFunc::CurrThreadID();
        // 日志等级
        TLogLevel m_lv = TLogLevel::lglvInfo;
        // 内容
        string m_msg;

        TLogData(TLogLevel lv, string msg) : m_lv(lv), m_msg(msg) {}

        // 日志类型转字符串
        string LogLvToStr(void)
        {
            switch (m_lv)
            {
            case TLogLevel::lglvTrace:      return "Trace";
            case TLogLevel::lglvDebug:      return "Debug";
            case TLogLevel::lglvInfo:       return "Info";
            case TLogLevel::lglvWarning:    return "Warning";
            case TLogLevel::lglvError:      return "Error";
            case TLogLevel::lglvFatal:      return "Fatal";
            default:                        return "Info";
            }
        }
    };
    // 日志列表
    typedef std::shared_ptr<TLogData> TLogDataPtr;
    typedef std::vector<TLogDataPtr> TLogDataList;
    mutable TLogDataList m_logList;
};
