#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // webservice�ӿ�
    class IKCWebService
    {
    public:
        // ִ��soap����
		virtual bool CALL_TYPE RunSoap(IActionData&) = 0;
    };
}
