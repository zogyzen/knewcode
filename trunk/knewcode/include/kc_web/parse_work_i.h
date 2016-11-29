#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/page_data_i.h"

namespace KC
{
    // 解析kc标记的接口
    class IParseWork : public IServiceEx
    {
    public:
        // 解析：参数为页数据
        virtual bool Parse(IWebPageData&) = 0;

    protected:
        virtual CALL_TYPE ~IParseWork(void) = default;
   };

    constexpr const char c_ParseWorkSrvGUID[] = "IParseWork_F6FA32AB-E4A8-49C8-AC89-737B0EB62D29";

    class TParseWorkException : public TFWSrvException
	{
	public:
		TParseWorkException(int id, string place, string msg, string name)
				: TFWSrvException(id, place, msg, name, c_ParseWorkSrvGUID) {}
		TParseWorkException(int id, string place, string msg, IParseWork& srv)
				: TFWSrvException(id, place, msg, srv) {}
	};
}

