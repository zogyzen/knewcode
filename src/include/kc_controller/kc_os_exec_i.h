#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "for_user/page_interface.h"

namespace KC
{
    constexpr const char c_KCOSExecSrvGUID[] = "IKCOSExec_AAE289BB-ADB8-55F8-CF31-AA411D2F52BF";
    class TKCOSExecException;

    // Session控制器接口
    class IKCOSExec : public IKCCtrlWork
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCOSExecSrvGUID;
        }

        typedef TKCOSExecException TCtrlException;

    protected:
        ~IKCOSExec() override = default;
    };

    class TKCOSExecException : public TFWSrvException
	{
	public:
        TKCOSExecException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                        : TFWSrvException(id, place, msg, name, c_KCOSExecSrvGUID, oth) {}
        TKCOSExecException(int id, std::string place, std::string msg, IKCOSExec& srv, std::string oth = "")
                        : TFWSrvException(id, place, msg, srv, oth) {}
	};
}

