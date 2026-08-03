#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "for_user/kc_controller_i.h"
#include "kc_controller/kc_ctrl_work_i.h"

namespace KC
{
    constexpr const char c_KCChaiScriptSrvGUID[] = "IKCChaiScript_35235CE7-AC63-9C97-A35F-C9ADB22568DD";

    // 错误码
    enum EErrCodeChaiScript
    {
        ecsChai_ErrCode_eval_error = ecd_ErrCode_KCChaiScript,
        ecsChai_ErrCode_load_module_error,
        ecsChai_ErrCode_file_not_found_error,
        ecsChai_ErrCode_UnknownCpp,
        ecsChai_ErrCode_UnknownOther
    };

    class TKCChaiScriptException;
    // 脚本控制器管理接口
    class IKCChaiScript : public IKCCtrlWork
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCChaiScriptSrvGUID;
        }

        typedef TKCChaiScriptException TCtrlException;

    protected:
        ~IKCChaiScript() override = default;
    };

    // 脚本控制器接口
    class ICtrlChai : public IKCController
    {
    public:
        // 宿主
        virtual IKCChaiScript& Own(void) = 0;
    };

    class TKCChaiScriptException : public TFWSrvException
    {
    public:
        TKCChaiScriptException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
            : TFWSrvException(id, place, msg, name, c_KCChaiScriptSrvGUID, oth) {}
        TKCChaiScriptException(int id, std::string place, std::string msg, IKCChaiScript& srv, std::string oth = "")
            : TFWSrvException(id, place, msg, srv, oth) {}
    };
}

