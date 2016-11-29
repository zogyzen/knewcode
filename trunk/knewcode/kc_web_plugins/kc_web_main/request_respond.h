#pragma once

namespace KC
{
    // 请求和应答
    class CKCRequestRespond : public IKCRequestRespondData
    {
    public:
        // 信号处理实例接口
        virtual ISignoCatch& CALL_TYPE SignoCatch(void);
        // 锁
        virtual IKcLockWork& CALL_TYPE LockWork(void);
        // 超时时间
        virtual int CALL_TYPE GetTimeOutSeconds(void) const;
        // 执行中的页面数据接口
        virtual IActionData* CALL_TYPE GetActionData(void);
        virtual void CALL_TYPE SetActionData(IActionData* = nullptr);

        ////////////////////////////请求部分///////////////////////////
        // 得到服务器端全部信息
        virtual const char* CALL_TYPE GetAllInfo(const char* = "\n");
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
        // 得到当前页本地完整根目录
        virtual const char* CALL_TYPE GetLocalPagePath(void);
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
        // 添加Cookie
        virtual bool CALL_TYPE AddCookie(const char*, const char*, const char* = "", const char* = "/", const char* = "");
        // 输出网页内容
        virtual bool CALL_TYPE OutputData(const char*, const char*, int = 0);
        virtual bool CALL_TYPE OutputHTML(const char*);

        ////////////////////////请求过程中的数据///////////////////////
        // 判断某名称数据是否存在
        virtual bool CALL_TYPE ExistsReStepData(const char*);
        // 得到某名称数据
        virtual IReStepData& CALL_TYPE GetReStepData(const char*);
        // 添加某名称数据
        virtual bool CALL_TYPE AddReStepData(const char*, IReStepData*&);

    public:
        CKCRequestRespond(IWebRequestRespond&, IKCWebWork&);
        virtual CALL_TYPE ~CKCRequestRespond();
        // 服务接口
        IKCWebWork& GetWork(void);
        // 释放
        void Release(void);
        // 提交
        void CommitResponseBody(void);

    private:
        IWebRequestRespond& m_re;
        IKCWebWork& m_webWork;
        IActionData* m_ActionData = nullptr;
        string m_LocalPagePath;     // 页面文件本地物理路径
        string m_Protocol;          // 协议
        // 响应的内容
        std::map<string, string> m_ResponseBodys;
        // 请求过程中的数据
        typedef boost::shared_ptr<IReStepData> TReStepDataPtr;
        typedef map<string, TReStepDataPtr> TReStepDataPtrList;
        TReStepDataPtrList m_ReStepDataPtrList;
    };
}
