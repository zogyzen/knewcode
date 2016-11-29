#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/web_struct.h"

namespace KC
{
    // �ڲ������ӿ�
    class IInnerVar : public IServiceEx
    {
    public:
        // �Ƿ����
        virtual bool Exists(IActionData&, string, TAnyTypeValList&) const = 0;
        // �õ��ڲ�����ֵ
        virtual boost::any GetValue(IActionData&, string, TAnyTypeValList&) = 0;
        // �����ڲ�����ֵ
        virtual void SetValue(IActionData&, string, TAnyTypeValList&, boost::any) = 0;

    protected:
        virtual CALL_TYPE ~IInnerVar(void) = default;
    };

    constexpr const char c_InnerVarSrvGUID[] = "IInnerVar_5BE5B3A3-A6C2-76F8-51D7-0C04C055BDEE";

    class TInnerVarException : public TFWSrvException
	{
	public:
		TInnerVarException(int id, string place, string msg, string name)
				: TFWSrvException(id, place, msg, name, c_InnerVarSrvGUID) {}
		TInnerVarException(int id, string place, string msg, IInnerVar& srv)
				: TFWSrvException(id, place, msg, srv) {}
	};
}

