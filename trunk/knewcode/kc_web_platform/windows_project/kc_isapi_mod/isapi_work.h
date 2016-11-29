#pragma once

namespace KC
{
    class CISAPIWork
    {
    public:
        CISAPIWork(const char* path);
        ~CISAPIWork();

        // ��������
        void Work(EXTENSION_CONTROL_BLOCK& ECB);

    protected:
        // ������Ϣ
        string m_LibError;
        // ������ģ�鶯̬����
        HINSTANCE m_kc_web_dll_h = NULL;
        // ����ܽӿ�
        IFramework* m_FrameworkIF = nullptr;
        // �����������
        IBundleContext* m_BundleContextIF = nullptr;
        // ��Ӧ������
        IServiceReference* m_WebMainRef = nullptr;
    };
}

