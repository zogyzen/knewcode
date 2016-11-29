#pragma once

namespace KC
{
    class CKCAlgorithmAction : public IKCAlgorithmAction
    {
    public:
        CKCAlgorithmAction(IActionData&);

        // ��ȡ��ǰʱ���ַ���
		virtual const char* CALL_TYPE GetNowString(void);
        // �ַ�������
		virtual bool CALL_TYPE SetStringAgent(const char*, const char*);
		virtual const char* CALL_TYPE GetStringAgent(const char*);
		// ����ת��
		virtual const char* CALL_TYPE GBK2utf8(const char*);
		virtual const char* CALL_TYPE utf82GBK(const char*);

    private:
        IActionData& m_act;
        string m_str;
    };
}
