#pragma once

namespace KC
{
    class CISAPIWork;
    // 请求和应答
    class CKCRequestRespond : public IWebRequestRespond
    {
    public:
        CKCRequestRespond(CISAPIWork& Parent, EXTENSION_CONTROL_BLOCK& ECB);
        // 得到服务器端全部信息
        virtual string CALL_TYPE GetAllInfo(const char* = "\n");

        ////////////////////////////请求部分///////////////////////////
        // 得到本地完整文件名
        virtual string CALL_TYPE GetLocalFilename(void);
        // 得到网络文件名
        virtual string CALL_TYPE GetUriFilename(void);
        // 协议
        virtual string CALL_TYPE GetProtocol(void);
        // 主机名
        virtual string CALL_TYPE GetHostName(void);
        // 端口
        virtual int CALL_TYPE GetPort(void);
        // GET的参数
        virtual string CALL_TYPE GetGetArgStr(void);
        // 得到网站本地完整根目录
        virtual string CALL_TYPE GetLocalRootPath(void);

        ////////////////////////////应答部分///////////////////////////
        // 输出网页内容
        virtual bool CALL_TYPE OutputHTML(const char* html);

    protected:
        CISAPIWork& m_Parent;
        EXTENSION_CONTROL_BLOCK& m_ECB;
    };
}
