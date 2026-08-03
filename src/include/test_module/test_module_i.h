#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"

namespace KC
{
    class IKCTestMod : public IServiceEx
    {
    public:
        virtual void run(void) = 0;

    protected:
        virtual CALL_TYPE ~IKCTestMod() = default;
    };

    constexpr const char c_KCTestModSrvGUID[] = "IKCTestMod_A5E289BB-ADB8-9FF8-CF31-BD411D2F52BF";

    class TKCTestModException : public TFWSrvException
    {
    public:
            TKCTestModException(int id, std::string place, std::string msg, std::string name)
                            : TFWSrvException(id, place, msg, name, c_KCTestModSrvGUID) {}
            TKCTestModException(int id, std::string place, std::string msg, IKCTestMod& srv)
                            : TFWSrvException(id, place, msg, srv) {}
    };
}
