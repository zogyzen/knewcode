#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    constexpr const char c_KCSqlOracleSrvGUID[] = "IKCSqlOracle_A5E289BB-ADB8-92F8-CF31-BD411D2F52BA";
    class TKCSqlOracleException;

    // Oracle数据库控制器接口
    class IKCSqlOracle : public IKCSql
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSqlOracleSrvGUID;
        }

        typedef TKCSqlOracleException TCtrlException;

    protected:
        ~IKCSqlOracle() override = default;
    };

    // 异常
    class TKCSqlOracleException : public TKCSqlException
	{
	public:
                TKCSqlOracleException(int id, std::string place, std::string msg, std::string name)
                                : TKCSqlException(id, place, msg, name, c_KCSqlOracleSrvGUID) {}
                TKCSqlOracleException(int id, std::string place, std::string msg, IKCSqlOracle& srv)
                                : TKCSqlException(id, place, msg, srv) {}
	};
}

