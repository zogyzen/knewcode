#pragma once

#include "framework/service_i.h"
#include "for_user/page_interface.h"

namespace KC
{
    // KC控制器接口
    class IKCController : public IKCObject
    {
    public:
        virtual ~IKCController() = default;

        // 附加参数
        struct IAttachParm
        {
            virtual ~IAttachParm() = default;

            // 传递数据
            virtual bool GetBool(const char*) const = 0;
            virtual int GetInt(const char*) const = 0;
            virtual double GetDouble(const char*) const = 0;
            virtual const char* GetStr(const char*) const = 0;
            virtual const char* GetBuf(const char*, unsigned& len) const = 0;
            virtual void Set(const char*, bool) = 0;
            virtual void Set(const char*, int) = 0;
            virtual void Set(const char*, double) = 0;
            virtual void Set(const char*, const char*) = 0;
            virtual void Set(const char*, const char*, unsigned /*len*/) = 0;
        };

    public:
        // 开始/停止
        virtual bool start(void) { return true; }
        virtual bool stop(void) { return true; }

        // 执行控制器
        virtual void CALL_TYPE Perform(ICtrlApiData&, IAttachParm&) = 0;
    };
}
