#pragma once

namespace KC
{
    class CAPACHEWork
    {
    public:
        CAPACHEWork(string, string);
        ~CAPACHEWork();

        // ��ʼ��
        void Init(void);
        // ��������
        void Work(request_rec& r);

    private:
        // ��ʼ����
        boost::mutex m_mtxInit;
        // ���·����ģ�顢��ҳĿ¼
        string m_FxPath, m_FxDll, m_PgPath;
        // ����ܶ�̬��
        KLoadLibrary<> m_framework_dll;
        // ����ܽӿ�
        IFramework* m_FrameworkIF = nullptr;
        // �����������
        IBundleContext* m_BundleContextIF = nullptr;
        // ��Ӧ������
        IServiceReference* m_WebMainRef = nullptr;
    };
}
