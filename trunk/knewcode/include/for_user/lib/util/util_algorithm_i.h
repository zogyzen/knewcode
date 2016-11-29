#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    // �������ܻ�ӿ�
    class IKCAlgorithmAction
    {
    public:
        // ��ȡ��ǰʱ���ַ���
		virtual const char* CALL_TYPE GetNowString(void) = 0;
        // �ַ�������
		virtual bool CALL_TYPE SetStringAgent(const char*, const char*) = 0;
		virtual const char* CALL_TYPE GetStringAgent(const char*) = 0;
		// ����ת��
		virtual const char* CALL_TYPE GBK2utf8(const char*) = 0;
		virtual const char* CALL_TYPE utf82GBK(const char*) = 0;
    };

    // �������ܹ����ӿ�
    class IKCUtilWork
    {
    public:
        // �õ�ҳ���Ӧ�������ܻ�ӿ�
		virtual IKCAlgorithmAction& CALL_TYPE GetAlgorithmAction(IActionData&) = 0;
    };
}
