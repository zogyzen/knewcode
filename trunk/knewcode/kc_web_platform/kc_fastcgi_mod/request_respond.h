#pragma once

namespace KC
{
    class CFastCGIWork;
    // 请求和应答
    class CWebRequestRespond : public IWebRequestRespond
    {
    public:
        CWebRequestRespond(CFastCGIWork&, FCGX_Request&);
        virtual CALL_TYPE ~CWebRequestRespond() = default;

        // 得到服务器端全部信息
        virtual const char* CALL_TYPE GetAllInfo(const char* = "\n");

        ////////////////////////////请求部分///////////////////////////
        // 得到本地完整文件名
        virtual const char* CALL_TYPE GetLocalFilename(void);
        // 得到网络文件名
        virtual const char* CALL_TYPE GetUriFilename(void);
        // 协议
        virtual const char* CALL_TYPE GetProtocol(void);
        // 主机名
        virtual const char* CALL_TYPE GetHostName(void);
        // 端口
        virtual int CALL_TYPE GetPort(void);
        // 客户端信息
        virtual const char* CALL_TYPE GetUserAgent(void);
        // 客户端IP
        virtual const char* CALL_TYPE GetClientIP(void);
        // 接收的信息
        virtual const char* CALL_TYPE GetAccept(const char* = "");
        // GET的参数
        virtual const char* CALL_TYPE GetGetArgStr(void);
        // Post的参数
        virtual const char* CALL_TYPE GetPostArgType(void);
        virtual int CALL_TYPE GetPostArgLength(void);
        virtual int CALL_TYPE GetPostArgBuffer(char*, int);
        // 得到网站本地完整根目录
        virtual const char* CALL_TYPE GetLocalRootPath(void);
        // 按名称得到请求头
        virtual const char* CALL_TYPE GetRequestHeader(const char*);
        // 得到Cookie
        virtual const char* CALL_TYPE GetCookieStr(void);

        ////////////////////////////应答部分///////////////////////////
        // 响应文本类型
        virtual bool CALL_TYPE SetResponseContentType(const char*);
        virtual const char* CALL_TYPE GetResponseContentType(void);
        // 输出网页内容
        virtual bool CALL_TYPE AddResponseBody(const char*, int = 0);
        // 按名称添加响应头
        virtual void CALL_TYPE AddResponseHeader(const char*, const char*);

    public:
        // 提交应答
        void CommitResponse(void);

    protected:
        CFastCGIWork& m_Parent;
        FCGX_Request& m_re;
        // 页信息列表
        map<string, string> m_PageInfoList;
        // 响应头和体
        string m_ResponseHead = "", m_ResponseBody = "";

    private:
        string m_AllInfo;                       // 全部请求信息
        string m_ContentType = "text/html";     // 应答类型
        string m_PostArgStr;                    // post参数串
    };
}
