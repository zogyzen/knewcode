#pragma once

#include "for_user/kc_object_i.h"
#include "for_user/common_define.h"

namespace KC
{
    class IKCStartWork;

    // 框架上下文操作
    class IKCContext : public IKCObject
    {
    public:
        // 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogDebug(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogInfo(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogWarning(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogFatal(const char*, const char*, const char* = "") const = 0;
        // 得到主启动接口
        virtual IKCStartWork& GetMain(void) = 0;
        // 得到框架模块目录
        virtual const char* CALL_TYPE getPath(void) const = 0;
        // 得到网站或应用根路径
        virtual const char* CALL_TYPE getWebsiteRootPath(void) const = 0;
        // 配置路径转换到本地完整路径
        virtual const char* CALL_TYPE transCfgPathToFullPath(const char*) const = 0;
        // 得到配置的目录
        virtual const char* CALL_TYPE getPathCfg(const char*) const = 0;
        // 获取本地化语言选项
        virtual const char* CALL_TYPE getSelLang(void) const = 0;
        // 获取本地化提示信息
        virtual const char* CALL_TYPE getHint(const char*, const char* = nullptr) const = 0;
        // 得到配置的日志等级
        virtual int CALL_TYPE GetCfgLogLevel(void) const = 0;
        // 得到服务器编号
        virtual unsigned CALL_TYPE GetSrvID(const char* = "id") const = 0;
        // 得到系统标志信息
        virtual const char* CALL_TYPE GetSysFlag(const char* = "customer", const char* = nullptr) const = 0;
        // 得到配置信息
        virtual const char* CALL_TYPE GetCfgInfo(const char*, const char* = nullptr, const char* = nullptr) const = 0;
        // 子配置项
        virtual int CALL_TYPE GetCfgSubCount(const char*) const = 0;
        virtual bool CALL_TYPE IsCfgSubValid(const char*, int) const = 0;
        virtual const char* CALL_TYPE GetCfgSubName(const char*, int) const = 0;
        virtual const char* CALL_TYPE GetCfgSubInfo(const char*, int, const char* = nullptr, const char* = nullptr) const = 0;
        // 得到配置文件
        virtual const char* CALL_TYPE GetCfgFile(void) const = 0;
        // 通过别名得到服务标识
        virtual const char* CALL_TYPE GetSrvGUIDByAlias(const char*) const = 0;
        // 锁
        virtual IKcLockWork& CALL_TYPE LockWork(void) = 0;
        // 超时时间
        virtual int CALL_TYPE GetTimeOutSeconds(void) const = 0;
        // 版本信息
        virtual const char* CALL_TYPE VersionInfo(void) const = 0;
        virtual const char* CALL_TYPE BuildDatetime(void) const = 0;
        // 加解密
        virtual const char* CALL_TYPE Encrypted(const char*, unsigned long, const char*) const = 0;
        virtual const char* CALL_TYPE Decrypted(const char*, unsigned long&, const char*) const = 0;

    protected:
        virtual ~IKCContext(void) = default;
    };
}
