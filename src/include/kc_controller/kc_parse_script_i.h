#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"

namespace KC
{
    // 脚本解析控制器接口
    class IKCParseScript : public IKCCtrlWork
    {
    protected:
        ~IKCParseScript() override = default;
    };

    constexpr const char c_KCParseScriptSrvGUID[] = "IKCParseScript_A5E28966-AD99-92F8-CF31-BD411D2F52BA";

    class TKCParseScriptException : public TFWSrvException
	{
	public:
                TKCParseScriptException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                                : TFWSrvException(id, place, msg, name, c_KCParseScriptSrvGUID, oth) {}
                TKCParseScriptException(int id, std::string place, std::string msg, IKCParseScript& srv, std::string oth = "")
                : TFWSrvException(id, place, msg, srv, oth) {}
	};
}

