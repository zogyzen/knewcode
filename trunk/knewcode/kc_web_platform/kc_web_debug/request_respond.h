#pragma once

namespace KC
{
    class CWebWork;
    // �����Ӧ��
    class CWebRequestRespond : public IWebRequestRespond
    {
        friend class CWebWork;

    public:
        CWebRequestRespond(CWebWork& Parent, const char* request, const char* path);
        virtual CALL_TYPE ~CWebRequestRespond() = default;

        // �õ���������ȫ����Ϣ
        virtual const char* CALL_TYPE GetAllInfo(const char* = "\n");

        ////////////////////////////���󲿷�///////////////////////////
        // �õ����������ļ���
        virtual const char* CALL_TYPE GetLocalFilename(void);
        // �õ������ļ���
        virtual const char* CALL_TYPE GetUriFilename(void);
        // Э��
        virtual const char* CALL_TYPE GetProtocol(void);
        // ������
        virtual const char* CALL_TYPE GetHostName(void);
        // �˿�
        virtual int CALL_TYPE GetPort(void);
        // �ͻ�����Ϣ
        virtual const char* CALL_TYPE GetUserAgent(void);
        // �ͻ���IP
        virtual const char* CALL_TYPE GetClientIP(void);
        // ���յ���Ϣ
        virtual const char* CALL_TYPE GetAccept(const char* = "");
        // GET�Ĳ���
        virtual const char* CALL_TYPE GetGetArgStr(void);
        // Post�Ĳ���
        virtual const char* CALL_TYPE GetPostArgType(void);
        virtual int CALL_TYPE GetPostArgLength(void);
        virtual int CALL_TYPE GetPostArgBuffer(char*, int);
        // �õ���վ����������Ŀ¼
        virtual const char* CALL_TYPE GetLocalRootPath(void);
        // �����Ƶõ�����ͷ
        virtual const char* CALL_TYPE GetRequestHeader(const char*);
        // �õ�Cookie
        virtual const char* CALL_TYPE GetCookieStr(void);

        ////////////////////////////Ӧ�𲿷�///////////////////////////
        // ������Ӧ�ı�����
        virtual bool CALL_TYPE SetResponseContentType(const char*);
        virtual const char* CALL_TYPE GetResponseContentType(void);
        // �����ҳ����
        virtual bool CALL_TYPE AddResponseBody(const char*, int);
        // �����������Ӧͷ
        virtual void CALL_TYPE AddResponseHeader(const char*, const char*);

    protected:
        CWebWork& m_Parent;
        // ����
        const char* m_request = nullptr;
        // Ӧ�𴮡�Ӧ������
        string m_response;
        string m_ContentType = "text/html";
        // ��Ŀ¼
        const char* m_path = nullptr;
    };
}
