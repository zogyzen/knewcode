#pragma once

namespace KC
{
    class CFastCGIWork
    {
    public:
        CFastCGIWork(const char* path);
        ~CFastCGIWork();

        // ��ʼ��
        void Init(void);
        // ��������
        void Work(void);

    private:
        // ��ȡ����
        bool Accept(FCGX_Request&);

    private:
        // ��ʼ����
        boost::mutex m_mtxInit;
        // ���·����ģ��
        string m_FxPath, m_FxDll;
        // ����ܶ�̬��
        KLoadLibrary<> m_framework_dll;
        // ����ܽӿ�
        IFramework* m_FrameworkIF = nullptr;
        // �����������
        IBundleContext* m_BundleContextIF = nullptr;
        // ��Ӧ������
        IServiceReference* m_WebMainRef = nullptr;
        // ��
        boost::mutex m_mtx;
    };
}
