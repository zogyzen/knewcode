#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // ���繦�ܻ�ӿ�
    class IKCNetAction
    {
    public:
        // ��ȡ��ҳ
		virtual const char* CALL_TYPE SyncHttpGet(const char*, const char*, int = 80) = 0;
    };

    // ���繦�ܹ����ӿ�
    class IKCNetWork
    {
    public:
        // �õ�ҳ���Ӧ���繦�ܻ�ӿ�
		virtual IKCNetAction& CALL_TYPE GetNetAction(IActionData&) = 0;
    };
}
