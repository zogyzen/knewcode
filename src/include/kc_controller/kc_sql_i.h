#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"

namespace KC
{
    // 数据库控制器接口
    class IKCSql : public IKCCtrlWork
    {
    protected:
        ~IKCSql() override = default;
    };

    constexpr const char c_KCSqlSrvGUID[] = "IKCSql_A5E289BB-A2B8-9268-C331-BD411D2F52BA";

    class TKCSqlException : public TFWSrvException
	{
	public:
                TKCSqlException(int id, std::string place, std::string msg, std::string name, std::string guid = c_KCSqlSrvGUID, std::string oth = "")
                                : TFWSrvException(id, place, msg, name, guid, oth) {}
                TKCSqlException(int id, std::string place, std::string msg, IKCSql& srv, std::string oth = "")
                : TFWSrvException(id, place, msg, srv, oth) {}
	};
}
