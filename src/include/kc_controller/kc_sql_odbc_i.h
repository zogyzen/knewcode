#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    constexpr const char c_KCSqlODBCSrvGUID[] = "IKCSqlODBC_A5F2FFBB-ADB8-92F8-2131-BD41157F52FF";
    class TKCSqlODBCException;

    // odbc 数据库控制器接口
    class IKCSqlODBC : public IKCSql
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSqlODBCSrvGUID;
        }

        typedef TKCSqlODBCException TCtrlException;

    protected:
        ~IKCSqlODBC() override = default;
    };

    class TKCSqlODBCException : public TKCSqlException
	{
	public:
                TKCSqlODBCException(int id, std::string place, std::string msg, std::string name)
                                : TKCSqlException(id, place, msg, name, c_KCSqlODBCSrvGUID) {}
                TKCSqlODBCException(int id, std::string place, std::string msg, IKCSqlODBC& srv)
                                : TKCSqlException(id, place, msg, srv) {}
	};
}

