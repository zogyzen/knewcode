#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "for_user/page_interface.h"

namespace KC
{
    // websocket控制器接口
    class IKCWebSocket : public IKCCtrlWork
    {
	public:
            // 发送消息
            virtual bool CALL_TYPE Send(const char* wsid, const char* type, const char* msg, const char* ctrl = nullptr) const = 0;

    protected:
        ~IKCWebSocket() override = default;
    };

    constexpr const char c_KCWebSocketSrvGUID[] = "IKCWebSocket_666289BB-FD48-55F8-CF31-BD411D2F52BF";

    class TKCWebSocketException : public TFWSrvException
	{
	public:
                TKCWebSocketException(int id, std::string place, std::string msg, std::string name)
                                : TFWSrvException(id, place, msg, name, c_KCWebSocketSrvGUID) {}
                TKCWebSocketException(int id, std::string place, std::string msg, IKCWebSocket& srv)
								: TFWSrvException(id, place, msg, srv) {}
	};
}

