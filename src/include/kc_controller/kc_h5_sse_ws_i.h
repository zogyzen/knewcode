#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "for_user/page_interface.h"

namespace KC
{
    constexpr const char c_KCSSESWSrvGUID[] = "IKCSSEWS_123289CC-FD48-55F8-CF31-BD411D2F52BF";
    class TKCSSEWSException;

    // sse和websocket控制器接口
    class IKCSSEWS : public IKCCtrlWork
    {
	public:
        // 发送消息
        virtual bool CALL_TYPE Send(const char* wsid, const char* type, const char* msg, const char* ctrl = nullptr) const = 0;

        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSSESWSrvGUID;
        }

        typedef TKCSSEWSException TCtrlException;

    protected:
        ~IKCSSEWS() override = default;
    };

    class TKCSSEWSException : public TFWSrvException
	{
	public:
        TKCSSEWSException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                        : TFWSrvException(id, place, msg, name, c_KCSSESWSrvGUID, oth) {}
        TKCSSEWSException(int id, std::string place, std::string msg, IKCSSEWS& srv, std::string oth = "")
                        : TFWSrvException(id, place, msg, srv, oth) {}
	};
}

