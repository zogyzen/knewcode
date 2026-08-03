#pragma once

#include <string>
#include <atomic>

#include <boost/format.hpp>
#include <boost/date_time.hpp>

#ifdef WIN32    // Windows环境
    #include <windows.h>
#else           // linux环境
#endif

namespace KC
{
    class SystemRunStatus
    {
    public:
        // 系统运行信息
        static std::string GetSysRunInfo(void)
        {
            return (boost::format("CPU:%.0f%%  Memory:%d%%") % GetTotalCpuUsage() % GetUsageMemory()).str();
        }

        // 计算CPU占有率
        static double GetTotalCpuUsage()
        {
        #ifdef WIN32    // Windows环境
            static std::atomic_ullong ptLast = 0;
            // 上次cpu时间
            static std::atomic_ullong lastTotal_ = 0, lastIdle_ = 0, lastTotalPre_ = 0, lastIdlePre_ = 0;
            // 本次cpu时间
            FILETIME idleTime, kernelTime, userTime;
            if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
            {
                // Convert FILETIME to ULARGE_INTEGER
                ULARGE_INTEGER idle, kernel, user;
                idle.LowPart = idleTime.dwLowDateTime;
                idle.HighPart = idleTime.dwHighDateTime;
                kernel.LowPart = kernelTime.dwLowDateTime;
                kernel.HighPart = kernelTime.dwHighDateTime;
                user.LowPart = userTime.dwLowDateTime;
                user.HighPart = userTime.dwHighDateTime;

                // Calculate total CPU time by subtracting idle time from total time
                const ULONGLONG totalCpu = (kernel.QuadPart + user.QuadPart);
                const ULONGLONG totalIdle = idle.QuadPart;

                const ULONGLONG totalSinceLastCall = totalCpu - lastTotal_;
                const ULONGLONG idleSinceLastCall = totalIdle - lastIdle_;

                unsigned long long ptNow = GetCurrentStampMS();
                if (ptNow - ptLast > 999)
                {
                    ptLast = ptNow;
                    lastTotal_ = lastTotalPre_.load();
                    lastIdle_ = lastIdlePre_.load();
                    lastTotalPre_ = totalCpu;
                    lastIdlePre_ = totalIdle;
                }

                // Calculate CPU usage since last call
                const double cpuUsage = totalSinceLastCall > 0 ? (1.0 - (double)idleSinceLastCall / (double)totalSinceLastCall) * 100 : 0.0;
                return cpuUsage;
            }
        #else           // linux环境
        #endif
            return 0;
        }

        // 获取内存使用率
        static unsigned GetUsageMemory(void)
        {
        #ifdef WIN32    // Windows环境
            MEMORYSTATUS ms;
            ::GlobalMemoryStatus(&ms);
            return ms.dwMemoryLoad;
        #else           // linux环境
            return 0;
        #endif
        }

        // 磁盘占有率
        static double GetDriveUsage(const char* drive = "C:\\")
        {
        #ifdef WIN32    // Windows环境
            ULARGE_INTEGER FreeBytesAvailable, TotalNumberOfBytes, TotalNumberOfFreeBytes;

            if (GetDiskFreeSpaceExA(drive, &FreeBytesAvailable, &TotalNumberOfBytes, &TotalNumberOfFreeBytes)) {
                ULARGE_INTEGER usedBytes;
                usedBytes.QuadPart = TotalNumberOfBytes.QuadPart - TotalNumberOfFreeBytes.QuadPart;
                double usage = (double)usedBytes.QuadPart / TotalNumberOfBytes.QuadPart * 100;
                return usage;
            }
        #else           // linux环境
        #endif
            return -1; // 错误处理
        }

    private:
        // 时间戳（毫秒）
        static unsigned long long GetCurrentStampMS(boost::posix_time::ptime pt = boost::posix_time::microsec_clock::local_time())
        {
            boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
            boost::posix_time::time_duration time_from_epoch = pt - epoch;
            return time_from_epoch.total_milliseconds();
        }

    };
}
