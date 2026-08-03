#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    constexpr const char c_KCSqlPgSQLSrvGUID[] = "IKCSqlPgSQL_A5E289BB-ADB8-92F8-AABB-BD411D2F5288";
    class TKCSqlPgSQLException;

    // postgresql数据库控制器接口
    class IKCSqlPgSQL : public IKCSql
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSqlPgSQLSrvGUID;
        }

        typedef TKCSqlPgSQLException TCtrlException;

    protected:
        ~IKCSqlPgSQL() override = default;
    };

    class TKCSqlPgSQLException : public TKCSqlException
	{
	public:
                TKCSqlPgSQLException(int id, std::string place, std::string msg, std::string name)
                                : TKCSqlException(id, place, msg, name, c_KCSqlPgSQLSrvGUID) {}
                TKCSqlPgSQLException(int id, std::string place, std::string msg, IKCSqlPgSQL& srv)
                                : TKCSqlException(id, place, msg, srv) {}
	};
}

