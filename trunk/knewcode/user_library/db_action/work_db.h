#pragma once

namespace KC
{
    class CKCDBWork : public IKCDBWork
    {
    public:
        CKCDBWork(string);

		// �������ݿ�����������Ϊ�������ơ������������Ӵ���
		virtual IDBAction& CALL_TYPE CreateDBAction(IActionData&, const char*, const char*, const char*);
		// �������ݿ����
		virtual IDBParms& CALL_TYPE NewParms(IActionData&);

    private:
        // ��ȡ���ݿ����ҳ�����ݽӿ�
        IKCRequestRespond::IReStepData& GetKCDBWorkStepData(IActionData&);

    private:
        // Ĭ��Ŀ¼
        string m_dir;
    };
}
