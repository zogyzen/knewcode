#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // Session�ӿ�
    class IKCSession
    {
    public:
		// ����Sessionֵ
		virtual bool CALL_TYPE Set(const char*, const char*) = 0;
		// ��ȡSessionֵ
		virtual const char* CALL_TYPE Get(const char*) = 0;
		// �Ƴ�Session
		virtual bool CALL_TYPE Remove(const char*) = 0;
		// �Ƿ����Session
		virtual bool CALL_TYPE Exists(const char*) = 0;
    };

    // Session����
    class IKCSessionWork
    {
    public:
        // ��ȡSession�ӿ�
		virtual IKCSession& CALL_TYPE GetSession(IActionData&) = 0;
    };
}
