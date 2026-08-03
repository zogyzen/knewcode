#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/web_struct.h"

namespace KC
{
    // KC系统接口
    class IKCWebWork : public IServiceEx
    {
    public:
        // 展现KC请求：参数是请求和应答接口
        virtual int CALL_TYPE RenderKC(ISrcRequestRespond&) = 0;
        // 静态页面处理
        virtual void CALL_TYPE StaticPage(ISrcRequestRespond&) = 0;

    protected:
        ~IKCWebWork() override = default;
    };

    constexpr const char c_KCWebWorkSrvGUID[] = "IKCWebWork_B5E289BB-ADB8-92F8-CF31-BD411D2F52BF";

    class TKCWebWorkException : public TFWSrvException
	{
	public:
        TKCWebWorkException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
            : TFWSrvException(id, place, msg, name, c_KCWebWorkSrvGUID, oth) {}
        TKCWebWorkException(int id, std::string place, std::string msg, IKCWebWork& srv, std::string oth = "")
            : TFWSrvException(id, place, msg, srv, oth) {}
	};

    // Web主模块插件 错误码定义
    enum EErrCodeDefineWebMain
    {
        ecd_ErrCode_KCWebMain_NoAct = ecd_ErrCode_KCWebMain + 1,            // 无活动数据
        ecd_ErrCode_KCWebMain_NoKCFile,                                     // 无KC文件
        ecd_ErrCode_KCWebMain_NoData,                                       // 无数据
        ecd_ErrCode_KCWebMain_NullPointData,                                // 数据为空指针
        ecd_ErrCode_KCWebMain_RepeatData,                                   // 重复定义数据
        ecd_ErrCode_KCWebMain_MFD_SizeWrong,                                // 多表单数据，数量错误
    };
}

