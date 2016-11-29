#pragma once

namespace KC
{
    class CKCSession : public IKCSession
    {
    public:
        CKCSession(IActionData&, IKCSessionWork&);

		// ����Sessionֵ
		virtual bool CALL_TYPE Set(const char*, const char*);
		// ��ȡSessionֵ
		virtual const char* CALL_TYPE Get(const char*);
		// �Ƴ�Session
		virtual bool CALL_TYPE Remove(const char*);
		// �Ƿ����Session
		virtual bool CALL_TYPE Exists(const char*);

    public:
        // �������Session
        void ClearUp(void);

    private:
        // �õ�session���
        string GetSessionID(void);

    private:
        IActionData& m_act;
        IKCSessionWork& m_work;
        IServiceReferenceEx& m_invar;
        string m_tmpStr;
    };
}
