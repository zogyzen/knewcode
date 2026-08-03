#pragma once

#include "framework_ex/service_ex_i.h"
#include "for_user/kc_controller_i.h"

namespace KC
{
    // KC控制器管理接口
    class IKCCtrlWork : public IServiceEx
    {
    public:
        // 控制器接口
        virtual bool CALL_TYPE hasCtrl(const char* = nullptr) const = 0;
        virtual IKCController& CALL_TYPE getCtrl(const char* = nullptr) = 0;

    protected:
        ~IKCCtrlWork() override = default;
    };
}
