#pragma once

namespace KC
{
    class CISAPIWork;
    // �����Ӧ��
    class CKCRequestRespond : public IWebRequestRespond
    {
    public:
        CKCRequestRespond(CISAPIWork& Parent, EXTENSION_CONTROL_BLOCK& ECB);
        // �õ���������ȫ����Ϣ
        virtual string CALL_TYPE GetAllInfo(const char* = "\n");

        ////////////////////////////���󲿷�///////////////////////////
        // �õ����������ļ���
        virtual string CALL_TYPE GetLocalFilename(void);
        // �õ������ļ���
        virtual string CALL_TYPE GetUriFilename(void);
        // Э��
        virtual string CALL_TYPE GetProtocol(void);
        // ������
        virtual string CALL_TYPE GetHostName(void);
        // �˿�
        virtual int CALL_TYPE GetPort(void);
        // GET�Ĳ���
        virtual string CALL_TYPE GetGetArgStr(void);
        // �õ���վ����������Ŀ¼
        virtual string CALL_TYPE GetLocalRootPath(void);

        ////////////////////////////Ӧ�𲿷�///////////////////////////
        // �����ҳ����
        virtual bool CALL_TYPE OutputHTML(const char* html);

    protected:
        CISAPIWork& m_Parent;
        EXTENSION_CONTROL_BLOCK& m_ECB;
    };
}
