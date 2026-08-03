#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/web_struct.h"

namespace KC
{
    // KC系统接口
    class IKCWebApiWork : public IServiceEx
    {
    public:
        // 初始化（卸载）系统
        virtual void CALL_TYPE InitSys(void) = 0;
        virtual void CALL_TYPE FreeSys(void) = 0;

        // 获取uri的本地地址
        virtual const char* CALL_TYPE GetApiUrlLocalDir(const char*) = 0;
        // 得到本地完整文件名
        virtual const char* CALL_TYPE GetUrlLocalPath(const char*) = 0;
        // 得到网络根路径
        virtual const char* CALL_TYPE GetUrlPageRootPath(const char*) = 0;

        // 展现一个网页：参数是请求和应答接口
        virtual int CALL_TYPE RenderREST(IKCRequestRespondData&) = 0;

    protected:
        ~IKCWebApiWork() override = default;
    };

    constexpr const char c_KCWebApiWorkSrvGUID[] = "IKCWebApiWork_B5E289FF-ADB8-92F8-CF31-BD411D2F52BF";

    class TKCWebApiWorkException : public TFWSrvException
    {
    public:
            TKCWebApiWorkException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                            : TFWSrvException(id, place, msg, name, c_KCWebApiWorkSrvGUID, oth) {}
            TKCWebApiWorkException(int id, std::string place, std::string msg, IKCWebApiWork& srv, std::string oth = "")
                            : TFWSrvException(id, place, msg, srv, oth) {}
    };
    class TActionException : public TKCWebApiWorkException
    {
    public:
            TActionException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                            : TKCWebApiWorkException(id, place, msg, name, oth) {}
    };
    class TCtrlException : public TActionException
    {
    public:
        TCtrlException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
            : TActionException(id, place, msg, name, oth) {}
    };
    class TCtrlJsonException : public TCtrlException
    {
    public:
        TCtrlJsonException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
            : TCtrlException(id, place, msg, name, oth) {}
    };

    // Webapi主控插件 错误码定义
    enum EErrCodeDefineWebApiWork
    {
        ecd_ErrCode_KCWebApiWork_NoKCFile = ecd_ErrCode_KCWebApiWork + 1,           // 无KC文件
        ecd_ErrCode_KCWebApiWork_NullPointActObj,                                   // 活动对象为空指针
        ecd_ErrCode_KCWebApiWork_RepeatActObj,                                      // 重复定义活动对象
        ecd_ErrCode_KCWebApiWork_InvalidRespondRoot,                                // 无效根返回结果
        ecd_ErrCode_KCWebApiWork_InvalidRespondSub,                                 // 无效子返回结果
        ecd_ErrCode_KCWebApiWork_RequestFail,                                       // 请求失败
        ecd_ErrCode_KCWebApiWork_ApiNoAccess,                                       // api未授权
        ecd_ErrCode_KCWebApiWork_KCFileFmtErr,                                      // KC文件格式错误
        ecd_ErrCode_KCWebApiWork_JsonFmtErr,                                        // Json格式错误
        ecd_ErrCode_KCWebApiWork_JsonItemTypeErr,                                   // Json元素类型错误
        ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied,                              // Json方法拒绝访问
    };
}

