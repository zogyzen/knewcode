#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    // SQL Server 数据库控制器接口
    class IKCSqlMssql : public IKCSql
    {
    protected:
        ~IKCSqlMssql() override = default;
    };

    constexpr const char c_KCSqlMssqlSrvGUID[] = "IKCSqlMssql_A5E2FFBB-ADB8-92F8-4831-BD411D2F52BA";

    class TKCSqlMssqlException : public TKCSqlException
	{
	public:
                TKCSqlMssqlException(int id, std::string place, std::string msg, std::string name)
                                : TKCSqlException(id, place, msg, name, c_KCSqlMssqlSrvGUID) {}
                TKCSqlMssqlException(int id, std::string place, std::string msg, IKCSqlMssql& srv)
                                : TKCSqlException(id, place, msg, srv) {}
	};
}

