#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/web_struct.h"

namespace KC
{
    // KCϵͳ�ӿ�
    class IKCWebWork : public IServiceEx
    {
    public:
        // ע���쳣��׽�ź�
        virtual void Signal(void) = 0;
        // չ��һ����ҳ�������������Ӧ��ӿ�
        virtual void RenderPage(IWebRequestRespond&) = 0;

    protected:
        virtual CALL_TYPE ~IKCWebWork() = default;
    };

    constexpr const char c_KCWebWorkSrvGUID[] = "IKCWebWork_B5E289BB-ADB8-92F8-CF31-BD411D2F52BF";

    class TKCWebWorkException : public TFWSrvException
	{
	public:
		TKCWebWorkException(int id, string place, string msg, string name)
				: TFWSrvException(id, place, msg, name, c_KCWebWorkSrvGUID) {}
		TKCWebWorkException(int id, string place, string msg, IKCWebWork& srv)
				: TFWSrvException(id, place, msg, srv) {}
	};
}

