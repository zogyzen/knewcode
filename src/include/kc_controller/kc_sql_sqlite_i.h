#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    constexpr const char c_KCSqliteSrvGUID[] = "IKCSqlite_22E27B4D-EB83-4FD8-6B5C-E042BC6F3058";
    class TKCSqliteException;

    // SQLite 数据库控制器接口
    class IKCSqlite : public IKCSql
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSqliteSrvGUID;
        }

        typedef TKCSqliteException TCtrlException;

    protected:
        ~IKCSqlite() override = default;
    };

    class TKCSqliteException : public TKCSqlException
	{
	public:
                TKCSqliteException(int id, std::string place, std::string msg, std::string name)
                                : TKCSqlException(id, place, msg, name, c_KCSqliteSrvGUID) {}
                TKCSqliteException(int id, std::string place, std::string msg, IKCSqlite& srv)
                                : TKCSqlException(id, place, msg, srv) {}
	};
}
