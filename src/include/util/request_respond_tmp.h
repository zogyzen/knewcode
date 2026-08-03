#pragma once

#include <string>
#include <map>

#include "kc_web/web_struct.h"

namespace KC
{
    // 临时的请求和应答，用于临时调用控制器
    template<typename TWork>
    class CRequestRespondTmp : public ISrcRequestRespond
    {
    public:
        CRequestRespondTmp(TWork& wrk) : m_work(wrk) {}
        ~CRequestRespondTmp(void) override = default;

        // 是否子调用
        bool IsSubCall(void) override
        {
            return true;
        }

        // 得到服务器端全部信息
        const char* CALL_TYPE GetAllInfo(const char* endTag = "\n") override
        {
            static thread_local string sAllInfo;
            sAllInfo.clear();
            for (string sName : c_arrWebRequestNames)
                if ("KCAllConstInfo" != sName)
                    sAllInfo += sName + ": " + CUtilFunc::PCharSafeToStr(GetSingleInfo(sName.c_str())) + endTag;
            return sAllInfo.c_str();
        }
        // 得到服务器端各单个请求信息
        const char* CALL_TYPE GetSingleInfo(const char* pName = "the_request", const char* /*pDef*/ = "") override
        {
            auto it = m_exSingleInfo.find(pName);
            if (m_exSingleInfo.end() != it)
                return it->second.c_str();
            else return GetRequestHeader(pName);
        }

        ////////////////////////////请求部分///////////////////////////
        // 得到本地完整文件名
        const char* CALL_TYPE GetLocalFilename(void) override
        {
            return GetLocalFilename(GetUriFilename());
        }
        const char* CALL_TYPE GetLocalFilename(const char* uri) override
        {
            return m_work.GetUrlLocalPath(uri);
        }
        // 得到网络文件名
        const char* CALL_TYPE GetUriFilename(void) override
        {
            return GetSingleInfo("uri");
        }
        // 得到网络路径
        const char* CALL_TYPE GetUrlPagePath(void) override
        {
            return CUtilFunc::GetUrlPagePath(GetUriFilename());
        }
        // 得到网络根路径
        const char* CALL_TYPE GetUrlPageRootPath(void) override
        {
            return GetUrlPageRootPath(GetUrlPagePath());
        }
        const char* CALL_TYPE GetUrlPageRootPath(const char* uri) override
        {
            return m_work.GetUrlPageRootPath(uri);
        }
        // 端口
        int CALL_TYPE GetPort(void) override
        {
            int result = 80;
            try
            {
                result = boost::lexical_cast<int>(GetSingleInfo("parsed_uri_port_str"));
            }
            catch(...) {}
            return result;
        }
        // GET的参数
        const char* CALL_TYPE GetGetArgStr(void) override
        {
            return GetSingleInfo("args");
        }
        // Post的参数
        const char* CALL_TYPE GetPostArgType(void) override
        {
            if(boost::algorithm::to_upper_copy(std::string(GetSingleInfo("method"))) == "POST")
                return GetSingleInfo("content_type");
            return "";
        }
        int CALL_TYPE GetPostArgLength(void) override
        {
            if(boost::algorithm::to_upper_copy(std::string(GetSingleInfo("method"))) == "POST")
                return atoi(GetSingleInfo("content_length"));
            return 0;
        }
        int CALL_TYPE GetPostArgBuffer(char*& buf, int len) override
        {
            int iPostLen = GetPostArgLength();
            buf = const_cast<char*>(m_postBody.c_str());
            return len < 0 ? iPostLen : min(len, iPostLen);
        }
        const char* CALL_TYPE GetPostArgStr(void) override
        {
            return m_postBody.c_str();
        }
        // 得到网站本地完整根目录
        const char* CALL_TYPE GetLocalRootPath(void) override
        {
            return m_PgPath.c_str();
        }
        // 按名称得到请求头
        const char* CALL_TYPE GetRequestHeader(const char* name) override
        {
            return GetStrInfo(m_headerInfo, name);
        }
        // 虚拟目录
        unsigned CALL_TYPE VirtualPathCount(void) override
        {
            return m_work.VirtualPathCount();
        }
        const char* CALL_TYPE GetVirtualPath(unsigned pos) override
        {
            return m_work.GetVirtualPath(pos);
        }
        const char* CALL_TYPE GetVirtualPathUri(unsigned pos) override
        {
            return m_work.GetVirtualPathUri(pos);
        }

        ////////////////////////////应答部分///////////////////////////
        // 响应文本类型
        bool CALL_TYPE SetResponseContentType(const char* ct) override
        {
            SetStrInfo(m_headerInfo, "content_type", ct);
            return true;
        }
        const char* CALL_TYPE GetResponseContentType(void) override
        {
            return GetStrInfo(m_headerInfo, "content_type");
        }
        // 响应状态
        bool CALL_TYPE SetResponseStatus(int iStt = 200) override
        {
            m_responseStatus = iStt;
            return true;
        }
        // 按名称添加、删除响应头
        void CALL_TYPE AddResponseHeader(const char* name, const char* val) override
        {
            SetStrInfo(m_headerInfo, name, val);
        }
        void CALL_TYPE DelResponseHeader(const char* name) override
        {
            DelStrInfo(m_headerInfo, name);
        }
        // 输出网页内容
        bool CALL_TYPE AddResponseBody(const char* buf, int len = 0) override
        {
            if (len <= 0) m_responseBody += buf;
            else m_responseBody.append(buf, len);
            return true;
        }
        // 提交响应（将这段时间，服务器端的输出，同时推给客户端）
        void CALL_TYPE CommitResponse(void) override
        {
        }

    public:
        // 添加扩展参数
        void AddExSingleInfo(std::string sName, std::string sParm)
        {
            SetStrInfo(m_exSingleInfo, sName, sParm);
        }
        // 添加cookie
        void AddCookie(std::string sCookie)
        {
            AddResponseHeader("Cookie", sCookie.c_str());
        }
        // 添加uri
        void AddUri(std::string sUri)
        {
            AddResponseHeader("uri", sUri.c_str());
        }
        // 添加args
        void AddArgs(std::string sArgs)
        {
            AddResponseHeader("args", sArgs.c_str());
        }
        // 添加the_request
        void AddTheRequest(std::string sTheRequest)
        {
            AddResponseHeader("the_request", sTheRequest.c_str());
        }
        // 添加post请求body
        void AddPostBody(std::string sBody)
        {
            m_postBody = sBody;
            AddResponseHeader("content_length", std::to_string(sBody.size()).c_str());
        }
        // 得到响应body
        std::string GetRespondBody(void)
        {
            return m_responseBody;
        }

    protected:
        TWork &m_work;
        std::map<std::string, std::string> m_headerInfo;        // 头信息
        std::map<std::string, std::string> m_exSingleInfo;      // 扩展信息
        std::string m_PgPath;           // 主页目录
        std::string m_postBody;         // 请求信息
        int m_responseStatus = 200;     // 返回状态
        std::string m_responseBody;     // 返回信息

    protected:
        // 信息
        const char* GetStrInfo(std::map<std::string, std::string>& infos, std::string sName)
        {
            auto it = infos.find(sName);
            if (infos.end() != it)
                return it->second.c_str();
            return "";
        }
        void SetStrInfo(std::map<std::string, std::string>& infos, std::string sName, std::string sParm)
        {
            auto it = infos.find(sName);
            if (infos.end() != it)
                it->second = sParm;
            infos.insert(make_pair(sName, sParm));
        }
        void DelStrInfo(std::map<std::string, std::string>& infos, std::string sName)
        {
            auto it = infos.find(sName);
            if (infos.end() != it)
                infos.erase(it);
        }
    };

    // 请求和应答
    class CRequestRespondPack : public ISrcRequestRespond
    {
    public:
        CRequestRespondPack(ISrcRequestRespond& re) : m_re(re) {}
        ~CRequestRespondPack(void) override = default;

        // 是否子调用
        bool IsSubCall(void) override
        {
            return true;
        }

        // 得到服务器端全部信息
        const char* CALL_TYPE GetAllInfo(const char* endTag = "\n") override
        {
            return m_re.GetAllInfo(endTag);
        }
        // 得到服务器端各单个请求信息
        const char* CALL_TYPE GetSingleInfo(const char* pName = "the_request", const char* /*pDef*/ = "") override
        {
            auto it = m_exSingleInfo.find(pName);
            if (m_exSingleInfo.end() != it)
                return it->second.c_str();
            return m_re.GetSingleInfo(pName);
        }

        ////////////////////////////请求部分///////////////////////////
        // 得到本地完整文件名
        const char* CALL_TYPE GetLocalFilename(void) override
        {
            return m_re.GetLocalFilename();
        }
        const char* CALL_TYPE GetLocalFilename(const char* uri) override
        {
            return m_re.GetLocalFilename(uri);
        }
        // 得到网络文件名
        const char* CALL_TYPE GetUriFilename(void) override
        {
            return m_re.GetUriFilename();
        }
        // 得到网络路径
        const char* CALL_TYPE GetUrlPagePath(void) override
        {
            return m_re.GetUrlPagePath();
        }
        // 得到网络根路径
        const char* CALL_TYPE GetUrlPageRootPath(void) override
        {
            return m_re.GetUrlPageRootPath();
        }
        const char* CALL_TYPE GetUrlPageRootPath(const char* uri) override
        {
            return m_re.GetUrlPageRootPath(uri);
        }
        // 端口
        int CALL_TYPE GetPort(void)
        {
            return m_re.GetPort();
        }
        // GET的参数
        const char* CALL_TYPE GetGetArgStr(void) override
        {
            return m_re.GetGetArgStr();
        }
        // Post的参数
        const char* CALL_TYPE GetPostArgType(void) override
        {
            return m_re.GetPostArgType();
        }
        int CALL_TYPE GetPostArgLength(void)
        {
            return m_re.GetPostArgLength();
        }
        int CALL_TYPE GetPostArgBuffer(char*& buf, int len) override
        {
            return m_re.GetPostArgBuffer(buf, len);
        }
        const char* CALL_TYPE GetPostArgStr(void) override
        {
            return m_re.GetPostArgStr();
        }
        // 得到网站本地完整根目录
        const char* CALL_TYPE GetLocalRootPath(void) override
        {
            return m_re.GetLocalRootPath();
        }
        // 按名称得到请求头
        const char* CALL_TYPE GetRequestHeader(const char* name) override
        {
            return m_re.GetRequestHeader(name);
        }
        // 虚拟目录
        unsigned CALL_TYPE VirtualPathCount(void) override
        {
            return m_re.VirtualPathCount();
        }
        const char* CALL_TYPE GetVirtualPath(unsigned pos) override
        {
            return m_re.GetVirtualPath(pos);
        }
        const char* CALL_TYPE GetVirtualPathUri(unsigned pos) override
        {
            return m_re.GetVirtualPathUri(pos);
        }

        ////////////////////////////应答部分///////////////////////////
        // 响应文本类型
        bool CALL_TYPE SetResponseContentType(const char* ct) override
        {
            return m_re.SetResponseContentType(ct);
        }
        const char* CALL_TYPE GetResponseContentType(void) override
        {
            return m_re.GetResponseContentType();
        }
        // 响应状态
        bool CALL_TYPE SetResponseStatus(int iStt = 200) override
        {
            return m_re.SetResponseStatus(iStt);
        }
        // 按名称添加、删除响应头
        void CALL_TYPE AddResponseHeader(const char*, const char*) override
        {
            //m_re.AddResponseHeader(name, val);
        }
        void CALL_TYPE DelResponseHeader(const char*) override
        {
            //m_re.DelResponseHeader(name);
        }
        // 输出网页内容
        bool CALL_TYPE AddResponseBody(const char*, int = 0) override
        {
            //return m_re.AddResponseBody(buf, nbyte);
            return true;
        }
        // 提交响应（将这段时间，服务器端的输出，同时推给客户端）
        void CALL_TYPE CommitResponse(void) override
        {
        }

    public:
        // 添加扩展参数
        void AddExSingleInfo(std::string sName, std::string sParm)
        {
            auto it = m_exSingleInfo.find(sName);
            if (m_exSingleInfo.end() != it)
                it->second = sParm;
            m_exSingleInfo.insert(make_pair(sName, sParm));
        }

    protected:
        ISrcRequestRespond& m_re;
        std::map<std::string, std::string> m_exSingleInfo;
    };
}
