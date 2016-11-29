#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // webservice接口
    class IKCWebService
    {
    public:
        // 执行soap方法
		virtual bool CALL_TYPE RunSoap(IActionData&) = 0;
    };
}
