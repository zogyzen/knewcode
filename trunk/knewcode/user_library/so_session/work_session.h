#pragma once

namespace KC
{
    // ���̼乲���ڴ����
    typedef CProcessNameList<g_SessionConnName, string> TProcNameList;
    struct IProcNameListWork
    {
        virtual ~IProcNameListWork(void) = default;
        virtual TProcNameList& GetProcMemList(void) = 0;
    };

    class CKCSessionWork : public IKCSessionWork
    {
    public:
        CKCSessionWork(void);
        ~CKCSessionWork(void);

        // ��ȡSession�ӿ�
		virtual IKCSession& CALL_TYPE GetSession(IActionData&);

        // ���̼乲���ڴ�
		IProcNameListWork& GetProcMemListWork(void);

    private:
        // ��ȡSessionҳ�����ݽӿ�
        IKCRequestRespond::IReStepData& GetKCSessionWorkStepData(IActionData&);

    private:
        // ���̼乲���ڴ����
        IProcNameListWork* m_procMemList = nullptr;
    };
}
