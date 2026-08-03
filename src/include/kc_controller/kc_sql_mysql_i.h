#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    constexpr const char c_KCSqlMySQLSrvGUID[] = "IKCSqlMySQL_18D60A3A-B5B9-49B6-42AC-8C32CBFCF23A";
    class TKCSqlMySQLException;

    // mysql数据库控制器接口
    class IKCSqlMySQL : public IKCSql
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSqlMySQLSrvGUID;
        }

        typedef TKCSqlMySQLException TCtrlException;

    protected:
        ~IKCSqlMySQL() override = default;
    };

    class TKCSqlMySQLException : public TKCSqlException
	{
	public:
                TKCSqlMySQLException(int id, std::string place, std::string msg, std::string name)
                                : TKCSqlException(id, place, msg, name, c_KCSqlMySQLSrvGUID) {}
                TKCSqlMySQLException(int id, std::string place, std::string msg, IKCSqlMySQL& srv)
                                : TKCSqlException(id, place, msg, srv) {}
	};
}

