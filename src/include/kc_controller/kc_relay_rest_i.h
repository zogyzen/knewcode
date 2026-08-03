#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"

namespace KC
{
    constexpr const char c_KCRelayRESTSrvGUID[] = "IKCRelayREST_A5E289BB-AD99-92F8-CF31-BD411D2F52BA";
    class TKCRelayRESTException;

    // 转发控制器接口
    class IKCRelayREST : public IKCCtrlWork
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCRelayRESTSrvGUID;
        }

        typedef TKCRelayRESTException TCtrlException;

    protected:
        ~IKCRelayREST() override = default;
    };

    class TKCRelayRESTException : public TFWSrvException
	{
	public:
        TKCRelayRESTException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                        : TFWSrvException(id, place, msg, name, c_KCRelayRESTSrvGUID, oth) {}
        TKCRelayRESTException(int id, std::string place, std::string msg, IKCRelayREST& srv, std::string oth = "")
                        : TFWSrvException(id, place, msg, srv, oth) {}
	};
}

