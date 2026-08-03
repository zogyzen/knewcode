#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    constexpr const char c_KCSqDMSrvGUID[] = "IKCSqlDM_41E276DC-DD1F-42C2-879D-C73EED0745FB";
    class TKCSqlDMException;

    // 达梦数据库控制器接口
    class IKCSqlDM : public IKCSql
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSqDMSrvGUID;
        }

        typedef TKCSqlDMException TCtrlException;

    protected:
        ~IKCSqlDM() override = default;
    };

    class TKCSqlDMException : public TKCSqlException
	{
	public:
                TKCSqlDMException(int id, std::string place, std::string msg, std::string name)
                                : TKCSqlException(id, place, msg, name, c_KCSqDMSrvGUID) {}
                TKCSqlDMException(int id, std::string place, std::string msg, IKCSqlDM& srv)
                                : TKCSqlException(id, place, msg, srv) {}
	};
}

