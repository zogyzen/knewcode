#pragma once

#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <set>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/any.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/shared_array.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
// #include <boost/interprocess/managed_mapped_file.hpp>

namespace KCSrv
{
    // 内置错误页面模板
    const char c_modPageErr[] = R"(
<html>
    <head>
        <meta charset="utf-8">
        <title>%1%</title>
    </head>
    <body>
        <h1 style="text-align: center;">%2%</h1>
        <hr>
        <div style="height: calc(100vh - 200px); display: flex; justify-content: center; align-items: center;">
                <span style="font-size: min(50vw, 40vh);">%3%</span>
        </div>
        <span>%4%</span>
        <hr>
        <div style="text-align: center;"><a target="_blank" href="https://apikc.cc">%5%</a></div>
    </body>
</html>
    )";

    // 请求方法
    const char c_RequestMethod_head[]       = "HEAD";           // 类似 GET，但只返回响应头，不返回内容。
    const char c_RequestMethod_get[]        = "GET";            // 用于获取资源，数据附在 URL 后，有长度限制。
    const char c_RequestMethod_post[]       = "POST";           // 用于提交数据或创建资源，数据在请求体中，较安全。
    const char c_RequestMethod_put[]        = "PUT";            // 全量更新指定资源，若不存在则创建。
    const char c_RequestMethod_delete[]     = "DELETE";         // 请求删除服务器上的指定资源。
    const char c_RequestMethod_options[]    = "OPTIONS";        // 查询服务器支持的通信选项或方法。
    const char c_RequestMethod_patch[]      = "PATCH";          // 对资源进行部分修改。
    const char c_RequestMethod_connect[]    = "CONNECT";        // 建立隧道连接，常用于 HTTPS 代理。
    const char c_RequestMethod_trace[]      = "TRACE";          // 回显收到的请求，用于诊断测试。

    // 常用web头
    const char c_WebHeader_Accept[]         = "Accept";
    const char c_WebHeader_Host[]           = "Host";
    const char c_WebHeader_ContentType[]    = "Content-Type";
    const char c_WebHeader_ContentLength[]  = "Content-Length";
    const char c_WebHeader_SetCookie[]      = "Set-Cookie";

    // 断点续连的web请求头
    const char c_WebHeader_Range[]          = "Range";          // Range: bytes=<range-start>-<range-end>
    // 断点续连的web应答头
    const char c_WebHeader_ContentRange[]   = "Content-Range";  // Content-Range: bytes <range-start>-<range-end>/<size>
    const char c_WebHeader_AcceptRanges[]   = "Accept-Ranges";  // Accept-Ranges: bytes

    // 常见MIME类型
    const std::map<std::string, std::string> c_set_mime_text         = {{"txt","plain"}, {"html",""}, {"htm",""}, {"css",""}, {"xml",""}};
    const std::map<std::string, std::string> c_set_mime_image        = {{"png",""}, {"ico","x-icon"}, {"jpg",""}, {"jpeg",""}, {"gif",""}, {"webp",""}};
    const std::map<std::string, std::string> c_set_mime_audio        = {{"mp3",""}, {"ogg",""}, {"wav",""}};
    const std::map<std::string, std::string> c_set_mime_video        = {{"mp4",""}, {"webm",""}, {"mpeg",""}};
    const std::map<std::string, std::string> c_set_mime_application  = {{"js","javascript"}, {"json",""}, {"pdf",""}, {"zip",""}};

    // 应答状态列表
    const std::map<int, std::string> c_mapStatus = {
        {100, "Continue"},                  // 初始请求已接受，客户端应继续发送剩余部分
        {101, "Switching Protocols"},       // 服务器同意切换协议（如升级为 WebSocket）
        {200, "OK"},                        // 请求成功
        {201, "Created"},                   // 请求成功且服务器创建了新的资源（常见于 POST）
        {204, "No Content"},                // 请求成功，但响应体无内容（常见于 DELETE）
        {206, "Partial Content"},           // 只返回文件部分内容（请求头Range的范围），用于断点续传
        {301, "Moved Permanently"},         // 资源‌永久‌移动到新 URL，浏览器会自动跳转
        {302, "Found"},                     // 资源‌临时‌移动到新 URL，客户端应继续使用原 URL
        {304, "Not Modified"},              // 资源未修改，客户端可使用本地缓存
        {400, "Bad Request"},               // 请求语法错误或参数无效
        {401, "Unauthorized"},              // 需要身份验证，未提供或无效
        {403, "Forbidden"},                 // 服务器理解请求，但‌拒绝执行‌（权限不足）
        {404, "Not Found"},                 // 服务器找不到请求的资源
        {416, "Range Not Satisfiable"},     // 在断点续传时，请求头Range的范围超出文件大小
        {500, "Internal Server Error"},     // 服务器遇到意外情况，无法完成请求
        {502, "Bad Gateway"},               // 作为网关的服务器收到上游服务器的无效响应
        {503, "Service Unavailable"},       // 服务器暂时过载或正在维护
        {504, "Gateway Timeout"}            // 网关服务器未及时从上游服务器获得响应
    };

    // 用于启动内置web服务器的基本参数
    struct TParmKcSrv
    {
        // 线程数量
        unsigned threadCount = 32;
        // 端口
        unsigned short portHttp = 0, portHttps = 0;
        // ssl证书文件
        std::string sslKey = "./ssl/private.key", sslCert = "./ssl/fullchain.pem";
    };

    // 所有者的智能指针类型
    template<typename TOwn>
    using TShardPtrOwn = std::shared_ptr<TOwn>;

    // （接口）服务端
    class IKcSrvHttp : public std::enable_shared_from_this<IKcSrvHttp>
    {
    public:
        virtual bool IsSSL(void) const = 0;
        virtual std::string KnewcodeVersion(void) const = 0;
    };
    typedef std::shared_ptr<IKcSrvHttp> KcSrvHttpPtr;

    // （接口）连接
    class IKcSrvConnect : public std::enable_shared_from_this<IKcSrvConnect>
    {
    public:
        virtual IKcSrvHttp& GetSrvHttp(void) const = 0;

        virtual long GetID(void) const = 0;
        virtual bool IsOpen(void) const = 0;
        virtual void CloseConn(void) = 0;

        virtual std::string ClientIP(void) const = 0;
        virtual unsigned short ClientPort(void) const = 0;
        virtual std::string LocalIP(void) const = 0;
        virtual unsigned short LocalPort(void) const = 0;
    };
    typedef std::shared_ptr<IKcSrvConnect> KcSrvConnectPtr;

    // 请求
    struct KcSrvRequest
    {
        // 连接
        KcSrvConnectPtr m_connect;

        // 请求头
        std::map<std::string, std::string> m_mapHeader;
        // 请求体
        std::string m_body;
        // 其他参数
        std::string m_the_request, m_method, m_httpVersion;
        std::string m_unparsed_uri, m_uri, m_extName, m_args;
        std::string m_hostName;
        int m_hostPort = 0;
        std::string m_ContentType;
        unsigned m_ContentLength = 0;

        // 附加参数
        mutable boost::any m_attachParm;

        // 构造函数
        KcSrvRequest(KcSrvConnectPtr conn, std::string strHeader) : m_connect(conn)
        {
            std::deque<std::string> dpuHeader;
            // boost::algorithm::split_regex(dpuHeader, strHeader, boost::regex("(\r\n)"));
            boost::algorithm::split(dpuHeader, strHeader, boost::is_any_of("\n"));
            // 首行为请求信息
            m_the_request = boost::algorithm::trim_copy(dpuHeader.front());
            // auto ip = m_connect->ClientIP();
            // auto port = m_connect->ClientPort();
            // std::cout << ip << ":" << port << std::endl;
            // std::cout << sFirstLine << std::endl;
            dpuHeader.pop_front();
            std::vector<std::string> vctFirstLine;
            boost::algorithm::split(vctFirstLine, m_the_request, boost::is_any_of(" "));
            if (!vctFirstLine.empty()) m_method = boost::algorithm::to_upper_copy(boost::algorithm::trim_copy(vctFirstLine[0]));
            if (vctFirstLine.size() > 1)
            {
                m_unparsed_uri = boost::algorithm::trim_copy(vctFirstLine[1]);
                // 解析请求的uri
                std::vector<std::string> vctUrl;
                boost::algorithm::split(vctUrl, m_unparsed_uri, boost::is_any_of("?"));
                // url路径
                m_uri = vctUrl[0];
                // url扩展名
                m_extName = boost::algorithm::to_lower_copy(boost::filesystem::path(m_uri).extension().string());
                // url的get参数
                if (vctUrl.size() > 1) m_args = vctUrl[1];
            }
            if (vctFirstLine.size() > 2) m_httpVersion = boost::algorithm::trim_copy(vctFirstLine[2]);
            // 第2行以后，为请求头
            for (std::string str : dpuHeader)
            {
                std::size_t iPos = str.find(':');
                if (std::string::npos != iPos)
                {
                    std::string sName = boost::algorithm::trim_copy(str.substr(0, iPos));
                    std::string sVal = boost::algorithm::trim_copy(str.substr(iPos + 1));
                    if (m_mapHeader.find(sName) == m_mapHeader.end())
                        m_mapHeader.insert(std::make_pair(sName, sVal));
                    /// 常用头的值
                    // 请求的主机
                    if (c_WebHeader_Host == sName)
                    {
                        std::size_t iPos = sVal.find(':');
                        m_hostName = std::string::npos == iPos ? sVal : sVal.substr(0, iPos);
                        m_hostPort = std::string::npos != iPos ? atoi(sVal.substr(iPos + 1).c_str()) : (m_connect->GetSrvHttp().IsSSL() ? 443 : 80);
                    }
                    // 请求的类型
                    else if (c_WebHeader_Accept == sName)
                    {
                        auto iPos = sVal.find(",");
                        m_ContentType = std::string::npos == iPos ? sVal : sVal.substr(0, iPos);
                    }
                    // 请求体的尺寸
                    if (c_WebHeader_ContentLength == sName) m_ContentLength = atoi(sVal.c_str());
                }
            }
        }

        // 是否带请求体
        bool IsPost(void) const
        {
            return c_RequestMethod_post == m_method || c_RequestMethod_put == m_method || c_RequestMethod_delete == m_method;
        }

        // 获取请求头
        const char* GetHead(std::string sName) const
        {
            auto it = m_mapHeader.find(sName);
            if (m_mapHeader.end() != it) return it->second.c_str();
            return nullptr;
        }
    };
    typedef std::shared_ptr<const KcSrvRequest> KcSrvRequestPtr;

    // 应答
    struct KcSrvRespond
    {
        // 请求
        const KcSrvRequestPtr m_request;

        // 应答头
        std::map<std::string, std::string> m_mapHeader;
        // cookie
        struct KcCookie
        {
            // 名称和值
            std::string m_name, m_str;

            KcCookie(std::string name, std::string val) : m_name(name), m_str(val)
            {
            }
            KcCookie(std::string name, std::string val, std::time_t expires, std::string path = "/", std::string domain = "")
                : m_name(name)
                , m_str((boost::format("%s; expires=%s%s%s") % val % KcCookie::StdTimeToGMT(expires)
                         % (path.empty() ? "" : "; path=" + path)
                         % (domain.empty() ? "" : "; domain=" + domain)).str())
            {
            }
            static std::string StdTimeToGMT(time_t tm)
            {
                std::ostringstream ss;
                ss << std::put_time(gmtime(&tm), "%F %T");
                return ss.str();
            }
        };
        std::map<std::string, KcCookie> m_mapCookie;
        // 其他参数
        int m_status = 200;
        std::string m_httpVersion = "http/1.1";
        // 应答体
        std::string m_body;
        // 映射文件
        // std::shared_ptr<boost::interprocess::file_mapping> m_file;
        // 应答文件
        std::string m_filename;
        // 断点续传时，读取应答文件的起始位置和尺寸
        long long m_readBegin = 0, m_readSize = 0;
        // 每次发送文件的大小
        constexpr static long long c_iStepSize = 1024 * 1024;

        KcSrvRespond(const KcSrvRequestPtr req) : m_request(req), m_httpVersion(req->m_httpVersion)
        {
            SetHead(c_WebHeader_ContentType, req->m_ContentType);
        }

        // 设置响应头
        void SetHead(std::string sName, std::string sVal)
        {
            auto it = m_mapHeader.find(sName);
            if (m_mapHeader.end() == it) m_mapHeader.insert(std::make_pair(sName, sVal));
            else it->second = sVal;
        }
        // 删除响应头
        void DelHead(std::string sName)
        {
            auto it = m_mapHeader.find(sName);
            if (m_mapHeader.end() != it) m_mapHeader.erase(it);
        }
        // 添加cookie
        void AddCookie(std::string sName, std::string sVal)
        {
            KcCookie ck(sName, sVal);
            auto it = m_mapCookie.find(sName);
            if (m_mapCookie.end() == it) m_mapCookie.insert(std::make_pair(sName, ck));
            else it->second = ck;
        }

        // 断点续传的请求
        bool RangsRequest(std::function<std::tuple<std::string, std::string>(const int)> fErr =
                [](const int iErrCode){ return std::make_tuple(std::to_string(iErrCode) + ".html", ""); })
        {
            bool bResult = true;
            // 文件总长度
            const std::size_t iSizeFile = boost::filesystem::file_size(m_filename);
            m_readSize = iSizeFile;
            // 大于1兆的文件才需要断点续传
            if (iSizeFile > c_iStepSize)
            {
                // 允许断点续传的应答头
                SetHead(c_WebHeader_AcceptRanges, "bytes");
                // 断点续传请求
                auto itRange = m_request->m_mapHeader.find(c_WebHeader_Range);
                if (m_request->m_mapHeader.end() != itRange)
                {
                    std::vector<std::string> vctRange;
                    boost::algorithm::split(vctRange, itRange->second, boost::is_any_of("="));
                    if (vctRange.size() == 2)
                    {
                        std::string sUnit = boost::algorithm::to_lower_copy(boost::algorithm::trim_copy(vctRange[0]));
                        std::string sRange = boost::algorithm::trim_copy(vctRange[1]);
                        try
                        {
                            if ("bytes" != sUnit) throw std::runtime_error(itRange->second + " Error. \"" + sUnit + "\"");
                            boost::algorithm::split(vctRange, sRange, boost::is_any_of("-"));
                            // 开始位置
                            std::string sBegin = boost::algorithm::trim_copy(vctRange[0]);
                            if (!sBegin.empty()) m_readBegin = boost::lexical_cast<long long>(vctRange[0]);
                            if (m_readBegin < 0) throw std::runtime_error(itRange->second + " Error. " + std::to_string(m_readBegin));
                            // 结束位置
                            long long iEnd = iSizeFile - 1;
                            if (vctRange.size() == 2)
                            {
                                std::string sEnd = boost::algorithm::trim_copy(vctRange[1]);
                                if (!sEnd.empty()) iEnd = boost::lexical_cast<long long>(sEnd);
                            }
                            if (iEnd >= iSizeFile) throw std::runtime_error(itRange->second + " Error. " + std::to_string(iEnd));
                            // 实际读取的大小
                            m_readSize = iEnd - m_readBegin + 1;
                            if (m_readSize <= 0) throw std::runtime_error(itRange->second + " Error. " + std::to_string(m_readSize));
                            SetHead(c_WebHeader_ContentRange, (boost::format("bytes %lld-%lld/%lld") % m_readBegin % iEnd % iSizeFile).str());
                            m_status = m_readSize < iSizeFile ? 206 : 200;
                        }
                        catch (std::exception &ex)
                        {
                            bResult = false;
                            std::cout << "[Range] Request Error: " << itRange->second << std::endl;
                            SetHead(c_WebHeader_ContentRange, (boost::format("%s %s/%lld") % sUnit % sRange % iSizeFile).str());
                            SetErrorPage(m_filename, 416, fErr, ex.what());
                        }
                        catch (...)
                        {
                            bResult = false;
                            std::cout << "[Range] Request Error: " << itRange->second << std::endl;
                            SetHead(c_WebHeader_ContentRange, (boost::format("%s %s/%lld") % sUnit % sRange % iSizeFile).str());
                            SetErrorPage(m_filename, 416, fErr);
                        }
                    }
                }
            }
            return bResult;
        }

        // 返回静态页面
        void SetStaticPage(std::string sFile, std::function<std::tuple<std::string, std::string>(const int)> fErr =
                [](const int iErrCode){ return std::make_tuple(std::to_string(iErrCode) + ".html", ""); })
        {
            if (!boost::filesystem::exists(sFile)) SetErrorPage(sFile, 404, fErr);
            else
            {
                std::string extName = boost::algorithm::to_lower_copy(boost::filesystem::path(sFile).extension().string());
                // 文件的mime类型
                std::string sMimeType = m_request->m_extName.empty() ? extName.substr(1) : m_request->m_extName.substr(1);
                // 判断mime类型
                auto fCheckMime = [&](const std::map<std::string, std::string>& mapMime)
                {
                    auto it = mapMime.find(sMimeType);
                    bool bExists = mapMime.end() != it;
                    if (bExists && !it->second.empty()) sMimeType = it->second;
                    return bExists;
                };
                // 文本
                if (fCheckMime(c_set_mime_text)) SetHead(c_WebHeader_ContentType, "text/" + sMimeType);
                // 图片
                else if (fCheckMime(c_set_mime_image)) SetHead(c_WebHeader_ContentType, "image/" + sMimeType);
                // 视频
                else if (fCheckMime(c_set_mime_video)) SetHead(c_WebHeader_ContentType, "video/" + sMimeType);
                // 音频
                else if (fCheckMime(c_set_mime_audio)) SetHead(c_WebHeader_ContentType, "audio/" + sMimeType);
                // 应用
                else if (fCheckMime(c_set_mime_application)) SetHead(c_WebHeader_ContentType, "application/" + sMimeType);
                // 其余为下载文件
                else SetHead(c_WebHeader_ContentType, "application/octet-stream");
                std::cout << "Static Page - " << sMimeType << ": " << sFile << std::endl;

                // 映射文件
                // boost::iostreams::mapped_file_source mfile(sFile);
                // m_body = std::string(static_cast<const char*>(mfile.data()), mfile.size());
                // m_file.reset(new boost::interprocess::file_mapping(sFile.c_str(), boost::interprocess::read_only));
                // boost::interprocess::mapped_region region(*m_file, boost::interprocess::read_only);
                // char* pData = static_cast<char*>(region.get_address());
                // int size = region.get_size();
                // std::cout << pData << "\t" << size << std::endl;


                // 设置应答文件
                m_filename = sFile;

                // 判断断点续传的请求是否合法
                if (!RangsRequest(fErr))
                {
                    m_readBegin = m_readSize = 0;
                    if (!m_filename.empty() && boost::filesystem::exists(m_filename) && !boost::filesystem::is_directory(m_filename))
                        m_readSize = boost::filesystem::file_size(m_filename);
                }
            }
        }
        // 返回错误页面
        void SetErrorPage(std::string sFile, const int errCode, std::function<std::tuple<std::string, std::string>(const int)> fErr =
                [](const int iErrCode){ return std::make_tuple(std::to_string(iErrCode) + ".html", ""); }, std::string sOther = " ")
        {
            m_status = errCode;
            std::string sFileLeaf = boost::filesystem::path(sFile).filename().string();
            auto [sErrPage, sSysName] = fErr(errCode);
            if (boost::filesystem::exists(sErrPage))
            {
                // boost::iostreams::mapped_file_source file(sErrPage);
                // m_body = std::string(static_cast<const char*>(file.data()), file.size());
                m_filename = sErrPage;
            }
            else
            {
                auto itStatus = c_mapStatus.find(errCode);
                std::string sStatus = c_mapStatus.end() != itStatus ? itStatus->second : std::to_string(errCode);
                m_body = (boost::format(c_modPageErr) % ("❌" + std::to_string(errCode) + ": " + sSysName + " - " + sFileLeaf)
                          % ("\"" + sFileLeaf + "\" " + sStatus)
                          % errCode % sOther
                          % m_request->m_connect->GetSrvHttp().KnewcodeVersion()).str();
                m_filename.clear();
            }
            SetHead(c_WebHeader_ContentType, "text/html");
        }
    };
    typedef std::shared_ptr<KcSrvRespond> KcSrvRespondPtr;
    // 请求处理的回调函数
    typedef std::function<void(KcSrvRespondPtr)> FRequestRespond;

    // 连接基类
    template<typename TOwn, typename TSock>
    class KcSrvConnectBase : public IKcSrvConnect
    {
    public:
        TOwn &m_own;

        KcSrvConnectBase(TOwn &own, KcSrvHttpPtr keepOwn, TSock socket)
            : m_own(own), m_socket(std::move(socket)), m_keepOwn(keepOwn)
        {
        }

        IKcSrvHttp& GetSrvHttp(void) const override { return m_own; }
        long GetID(void) const override { return this->m_id; }

    protected:
        // 初始连接
        virtual void Start(void)
        {
            std::cout << "[" << this->GetID() << "] Client Connect: \t" << std::hex << &this->m_socket << std::endl;
        }
        // 读请求
        void do_read(void)
        {
            auto self(this->shared_from_this());
            // 读请求头
            std::shared_ptr<boost::asio::streambuf> bufReadPtr(new boost::asio::streambuf);
            boost::asio::async_read_until(this->m_socket, *bufReadPtr, "\r\n\r\n",
                [this, self, bufReadPtr](const boost::system::error_code& ec, std::size_t length)
                {
                    try
                    {
                        if (!this->m_own.m_own.IsRunning()) return;
                        if (!ec)
                        {
                            std::string dataHeader(buffers_begin(bufReadPtr->data()), buffers_begin(bufReadPtr->data()) + length);
                            bufReadPtr->consume(length);
                            bufReadPtr->commit(length);
                            std::cout << "[" << this->GetID() << "] read:" << std::endl << dataHeader << std::endl;
                            KcSrvRequest *pReq = new KcSrvRequest(self, dataHeader);
                            KcSrvRequestPtr reqPtr(pReq);
                            pReq->m_body.append(buffers_begin(bufReadPtr->data()), buffers_end(bufReadPtr->data()));
                            // 剩余未读数据
                            long long iResidue = reqPtr->m_ContentLength - bufReadPtr->size();
                            // 读请求体
                            if (iResidue > 0)
                            {
                                boost::shared_array<char> strResidue(new char[iResidue + 1]{ 0 });
                                boost::asio::async_read(this->m_socket, boost::asio::buffer(strResidue.get(), iResidue),
                                    [this, self, pReq, reqPtr, strResidue](const boost::system::error_code& ec, std::size_t length)
                                    {
                                        try
                                        {
                                            if (!this->m_own.m_own.IsRunning()) return;
                                            if (!ec)
                                            {
                                                pReq->m_body.append(strResidue.get(), length);
                                                // std::cout << "[" << this->GetID() << "]Read Body:\n" << pReq->m_body << std::endl;
                                                // 处理
                                                this->Deal(reqPtr);
                                            }
                                            else
                                            {
                                                std::cout << "[" << this->GetID() << "] Read Body: " << ec.message() << std::endl;
                                                WaitNextRequest(self, ec, 555);
                                            }
                                        }
                                        catch (...)
                                        {
                                            std::cout << "[" << this->GetID() << "] Read Body: Unknown Error" << std::endl;
                                            WaitNextRequest(self, ec, 555);
                                        }
                                    });
                            }
                            else this->Deal(reqPtr);
                        }
                        else
                        {
                            std::cout << "[" << this->GetID() << "] async_read_until Error: " << ec.value() << "-" << ec.message() << std::endl;
                            WaitNextRequest(self, ec, 555);
                        }
                    }
                    catch (...)
                    {
                        std::cout << "[" << this->GetID() << "] async_read_until Error: Unknown Error" << std::endl;
                        WaitNextRequest(self, ec, 555);
                    }
                });
        }

        // 写应答
        void do_write(KcSrvRespondPtr res)
        {
            if (!m_own.m_own.IsRunning()) return;
            // char sBuf[] = "HTTP/1.1 200\r\nContent-Type: text/html\r\nContent-Length: 8\r\n\r\nhello kc";
            // char sBuf[] = "HTTP/1.1 200\r\nContent-Length: 0\r\n\r\n";
            // 应答状态
            auto itStatus = c_mapStatus.find(res->m_status);
            std::string sStatus = c_mapStatus.end() != itStatus ? " " + itStatus->second : "";
            // 应答首行
            std::string sDataHead = (boost::format("%s %d%s") % res->m_httpVersion % res->m_status % sStatus).str();
            // 应答头
            for (auto &h : res->m_mapHeader)
                if (c_WebHeader_ContentLength != h.first) sDataHead += "\r\n" + h.first + ": " + h.second;
            // cookie
            for (auto &c : res->m_mapCookie)
                sDataHead += (boost::format("\r\n%s: %s=%s") % c_WebHeader_SetCookie % c.second.m_name % c.second.m_str).str();
            // 等待下一次请求
            auto self(this->shared_from_this());
            auto fWaitNextRequest = [this, self, res](const boost::system::error_code& ec, std::size_t len)
            {
                try
                {
                    if (!this->m_own.m_own.IsRunning()) return;
                    if (!ec)
                    {
                        // std::cout << "[" << this->GetID() << "] Respond End: " << res->m_request->m_the_request << " \t" << std::dec << len << std::endl;
                        WaitNextRequest(self, ec);
                    }
                    else
                    {
                        std::cout << "[" << this->GetID() << "] Respond Error: " << res->m_request->m_the_request << std::endl << ec.message() << std::endl;
                        WaitNextRequest(self, ec, 555);
                    }
                }
                catch (...)
                {
                    std::cout << "[" << this->GetID() << "] Respond Error - " << res->m_request->m_the_request << std::endl;
                    WaitNextRequest(self, ec, 555);
                }
            };
            // 发送字符串
            auto fSendStr = [&](std::string sHead, std::string sBody = "",
                    std::function<void(const boost::system::error_code&, std::size_t)> fEnd = [](const boost::system::error_code&, std::size_t){},
                    std::function<void(size_t)> fMsg = [](size_t){})
            {
                size_t iSize = sHead.size() + sBody.size();
                boost::shared_array<char> bufPtr(new char[iSize + 1]{ 0 });
                memcpy(bufPtr.get(), sHead.data(), sHead.size());
                if (!sBody.empty())
                    memcpy(bufPtr.get() + sHead.size(), sBody.data(), sBody.size());
                fMsg(iSize);
                boost::asio::async_write(this->m_socket, boost::asio::buffer(bufPtr.get(), iSize),
                    [bufPtr, fEnd](const boost::system::error_code& ec, std::size_t len) { fEnd(ec, len); });
            };
            // 返回应答体
            auto fRespondBody = [&](std::size_t iSize)
            {
                // 发送http头和体
                sDataHead += (boost::format("\r\n%s: %d\r\n\r\n") % c_WebHeader_ContentLength % iSize).str();
                fSendStr(sDataHead, c_RequestMethod_head == res->m_request->m_method ? "" : res->m_body,
                    [fWaitNextRequest](const boost::system::error_code& ec, std::size_t len){
                        fWaitNextRequest(ec, len);
                    },
                    [&](size_t size){
                        std::cout << "[" << this->GetID() << "] Respond: " << res->m_request->m_the_request << " \t" << std::dec << size << std::endl;
                    });
            };
            // 返回文件
            auto fRespondFile = [&](void)
            {
                /*
                // 映射文件
                std::shared_ptr<boost::interprocess::mapped_region> region(new boost::interprocess::mapped_region(*res->m_file, boost::interprocess::read_only));
                std::size_t iSize = region->get_size();
                // 先发送http头
                sDataHead += (boost::format("\r\n%s: %d\r\n\r\n") % c_WebHeader_ContentLength % iSize).str();
                std::cout << "[" << this->GetID() << "] Respond: " << res->m_request->m_the_request << " \t" << std::dec << iSize + sDataHead.size() << std::endl;
                fSendStr(sDataHead, "",
                    [fWaitNextRequest, region, iSize, this](const boost::system::error_code& ec, std::size_t len){
                        if (!ec)
                            this->SendMapFile(fWaitNextRequest, region, reinterpret_cast<char*>(region->get_address()), iSize);
                        else fWaitNextRequest(ec, len);
                    },
                    [&](std::size_t size){
                        std::cout << "[" << this->GetID() << "] Respond Head: " << std::dec << size << std::endl;
                    });
                */

                // 应答体长度
                sDataHead += (boost::format("\r\n%s: %d\r\n\r\n") % c_WebHeader_ContentLength % res->m_readSize).str();
                std::cout << "[" << this->GetID() << "] Respond: " << res->m_request->m_the_request << " \t" << std::dec << res->m_readSize + sDataHead.size() << std::endl;
                // 先发送http头
                fSendStr(sDataHead, "",
                    [fWaitNextRequest, res, this](const boost::system::error_code& ec, std::size_t len){
                        try
                        {
                            if (!ec && c_RequestMethod_head != res->m_request->m_method)
                            {
                                // 打开文件
                                std::shared_ptr<std::ifstream> ptrFin(new std::ifstream(res->m_filename, std::ios::binary));
                                if (ptrFin->is_open())
                                {
                                    // 调整读文件起始位置
                                    if (res->m_readBegin > 0) ptrFin->seekg(res->m_readBegin);
                                    // 发送文件内容
                                    this->SendFileBuf(fWaitNextRequest, ptrFin, res->m_readSize);
                                }
                                else throw std::runtime_error("File Busy - " + res->m_filename);

                            }
                            else fWaitNextRequest(ec, len);
                        }
                        catch (std::exception &ex)
                        {
                            std::cout << "[" << this->GetID() << "] " << ex.what() << std::endl;
                            fWaitNextRequest(boost::system::error_code(boost::system::errc::device_or_resource_busy, boost::system::generic_category()), 0);
                        }
                    },
                    [&](std::size_t size){
                        std::cout << "[" << this->GetID() << "] Respond Head: " << std::dec << size << std::endl;
                    });
            };
            // 如果指定文件，通过文件返回
            if (!res->m_filename.empty())
            {
                // 文件大小
                std::size_t iSize = boost::filesystem::file_size(res->m_filename);
                // 大于1兆的文件，循环读文件返回
                if (iSize > KcSrvRespond::c_iStepSize) fRespondFile();
                // 小于1兆的文件，通过应答体返回
                else
                {
                    if (c_RequestMethod_head != res->m_request->m_method)
                    {
                        boost::iostreams::mapped_file_source file(res->m_filename);
                        res->m_body = std::string(static_cast<const char*>(file.data()), file.size());
                    }
                    else res->m_body.clear();
                    fRespondBody(iSize);
                }
            }
            // 未指定文件，通过应答体返回
            else fRespondBody(res->m_body.size());
        }
        // 发送映射文件
        /*
        void SendMapFile(std::function<void(const boost::system::error_code&, std::size_t)> fWaitNextRequest,
                         std::shared_ptr<boost::interprocess::mapped_region> region, char *pBuf, std::size_t iSize)
        {
            try
            {
                boost::asio::async_write(this->m_socket, boost::asio::buffer(pBuf, std::min(c_iStepSize, iSize)),
                    [fWaitNextRequest, region, pBuf, iSize, this](const boost::system::error_code& ec, std::size_t len) {
                        std::cout << "[" << this->GetID() << "] Respond Body: " << std::dec << len << std::endl;
                        if (!ec && iSize > c_iStepSize)
                            this->SendMapFile(fWaitNextRequest, region, pBuf + c_iStepSize, iSize - c_iStepSize);
                        else fWaitNextRequest(ec, len);
                    });
            }
            catch (std::exception &ex)
            {
                boost::system::error_code ec = boost::system::error_code(boost::system::errc::resource_unavailable_try_again, boost::system::generic_category());
                fWaitNextRequest(ec, 0);
            }
            catch (...)
            {
                boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::resource_unavailable_try_again);
                fWaitNextRequest(ec, 0);
            }
        }
        */
        // 发送文件
        void SendFileBuf(std::function<void(const boost::system::error_code&, std::size_t)> fWaitNextRequest,
                         std::shared_ptr<std::ifstream> ptrFin, long long iSize)
        {
            try
            {
                // 读文件一段内容
                long long iMinReadSize = std::min(KcSrvRespond::c_iStepSize, iSize);
                boost::shared_array<char> ptrBuf(new char[iMinReadSize + 1] { 0 });
                ptrFin->read(ptrBuf.get(), iMinReadSize);
                // 发送这段内容
                boost::asio::async_write(this->m_socket, boost::asio::buffer(ptrBuf.get(), iMinReadSize),
                    [fWaitNextRequest, ptrFin, ptrBuf, iSize, this](const boost::system::error_code& ec, std::size_t len) {
                        std::cout << "[" << this->GetID() << "] Respond Body: " << std::dec << len << std::endl;
                        if (!ec && iSize > KcSrvRespond::c_iStepSize)
                            this->SendFileBuf(fWaitNextRequest, ptrFin, iSize - KcSrvRespond::c_iStepSize);
                        else fWaitNextRequest(ec, len);
                    });
            }
            catch (std::exception &ex)
            {
                boost::system::error_code ec = boost::system::error_code(boost::system::errc::resource_unavailable_try_again, boost::system::generic_category());
                fWaitNextRequest(ec, 0);
            }
            catch (...)
            {
                boost::system::error_code ec = boost::system::errc::make_error_code(boost::system::errc::resource_unavailable_try_again);
                fWaitNextRequest(ec, 0);
            }
        }

        // 处理
        void Deal(KcSrvRequestPtr req)
        {
            KcSrvRespondPtr res(new KcSrvRespond(req));
            m_own.m_own.RequestRespond(res);
            this->do_write(res);
        }

        // 等待下一个请求
        void WaitNextRequest(KcSrvConnectPtr, const boost::system::error_code& ec, int ims = 1)
        {
            try
            {
                bool bIsBreakEC = IsBreakErrCode(ec);
                if (!bIsBreakEC && this->IsOpen())
                {
                    std::cout << "[" << this->GetID() << "] Wait Next Request - " << ims << std::endl;
                    boost::this_thread::sleep(boost::posix_time::milliseconds(ims));
                    this->do_read();
                }
                else
                {
                    this->CloseConn();
                    std::cout << "[" << this->GetID() << "] Lost Connection" << std::endl;
                }
            }
            catch (...) {}
        }

        // 代表连接断开的错误码
        virtual bool IsBreakErrCode(const boost::system::error_code& ec) const
        {
            return boost::asio::error::connection_aborted == ec || boost::asio::error::eof == ec;
        }

    protected:
        TSock m_socket;
        static inline long s_id = 0;
        const long m_id = ++s_id;

    private:
        KcSrvHttpPtr m_keepOwn;
    };
    // 用于Http连接
    template<typename TOwn>
    using TKcSrvConnectTCPBase = KcSrvConnectBase<TOwn, boost::asio::ip::tcp::socket>;
    template<typename TOwn>
    class KcSrvConnectTCP : public TKcSrvConnectTCPBase<TOwn>
    {
    public:
        typedef TKcSrvConnectTCPBase<TOwn> TParentClass;
        // using TParentClass::KcSrvConnectBase;
        // using KcSrvConnectBase<TOwn, boost::asio::ip::tcp::socket>::KcSrvConnectBase;
        KcSrvConnectTCP(TOwn &own, KcSrvHttpPtr keepOwn, boost::asio::ip::tcp::socket socket)
            : TParentClass(own, keepOwn, std::move(socket))
        {
        }

        void Start(void) override
        {
            TParentClass::Start();
            this->do_read();
        }

        bool IsOpen(void) const override { return this->m_socket.is_open(); }
        void CloseConn(void) override
        {
            try
            {
                if (this->IsOpen()) this->m_socket.close();
            }
            catch (...) {}
        }

        std::string ClientIP(void) const override
        {
            return this->m_socket.remote_endpoint().address().to_string();
        }
        unsigned short ClientPort(void) const override
        {
            return this->m_socket.remote_endpoint().port();
        }
        std::string LocalIP(void) const override
        {
            return this->m_socket.local_endpoint().address().to_string();
        }
        unsigned short LocalPort(void) const override
        {
            return this->m_socket.local_endpoint().port();
        }
    };
    // 用于Https连接
    typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> T_SSl_Socket;
    template<typename TOwn>
    using TKcSrvConnectSSLBase = KcSrvConnectBase<TOwn, T_SSl_Socket>;
    template<typename TOwn>
    class KcSrvConnectSSL : public TKcSrvConnectSSLBase<TOwn>
    {
    public:
        typedef TKcSrvConnectSSLBase<TOwn> TParentClass;
        KcSrvConnectSSL(TOwn& own, KcSrvHttpPtr keepOwn, T_SSl_Socket socket, std::shared_ptr<boost::asio::ssl::context> ctx)
            : TParentClass(own, keepOwn, std::move(socket)), m_sslContext(ctx)
        {
            // ctx->set_options(
            //     boost::asio::ssl::context::default_workarounds |
            //     boost::asio::ssl::context::no_sslv2 |
            //     boost::asio::ssl::context::single_dh_use);
            // ctx->use_certificate_chain_file("./ssl/fullchain.pem");
            // ctx->use_private_key_file("./ssl/private.key", boost::asio::ssl::context::pem);
            // m_context.use_tmp_dh_file("dh4096.pem");
            ctx->set_password_callback(std::bind(&KcSrvConnectSSL::get_password, this));

            // ctx->load_verify_file("./ssl/fullchain.pem");

            // socket.set_verify_mode(boost::asio::ssl::verify_peer);
            // socket.set_verify_callback(boost::asio::ssl::host_name_verification("host.name"));
        }

        void Start(void) override
        {
            TParentClass::Start();
            do_handshake();
        }

        bool IsOpen(void) const override { return this->m_socket.lowest_layer().is_open(); }
        void CloseConn(void) override
        {
            try
            {
                this->m_socket.shutdown();
                if (this->IsOpen()) this->m_socket.lowest_layer().close();
            }
            catch (...) {}
        }

        std::string ClientIP(void) const override
        {
            return this->m_socket.lowest_layer().remote_endpoint().address().to_string();
        }
        unsigned short ClientPort(void) const override
        {
            return this->m_socket.lowest_layer().remote_endpoint().port();
        }
        std::string LocalIP(void) const override
        {
            return this->m_socket.lowest_layer().local_endpoint().address().to_string();
        }
        unsigned short LocalPort(void) const override
        {
            return this->m_socket.lowest_layer().local_endpoint().port();
        }

    protected:
        // ssl握手
        void do_handshake()
        {
            auto self(this->shared_from_this());
            this->m_socket.async_handshake(boost::asio::ssl::stream_base::server,
                [this, self](const boost::system::error_code& ec)
                {
                    if (!ec)
                    {
                        this->do_read();
                    }
                    else std::cout << "[" << this->GetID() << "] do_handshake Error: " << ec.message() << std::endl;
                });
        }

        std::string get_password() const
        {
            // return this->m_own.GetPassword();
            return "123";
        }

        // 代表连接断开的错误码
        bool IsBreakErrCode(const boost::system::error_code& ec) const override
        {
            return TParentClass::IsBreakErrCode(ec) || boost::asio::ssl::error::stream_truncated == ec;
        }

    protected:
        std::shared_ptr<boost::asio::ssl::context> m_sslContext;
    };

    // 服务端
    template<typename TOwn>
    class KcSrvHttp : public IKcSrvHttp
    {
    public:
        TOwn &m_own;

        KcSrvHttp(TShardPtrOwn<TOwn> own, boost::asio::io_context& io_context, unsigned short port)
            : m_own(*own.get()), m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), m_keepOwn(own)
        {
        }

        void Start(void)
        {
            this->do_accept();
        }

        bool IsSSL(void) const override { return false; }
        std::string KnewcodeVersion(void) const override { return m_own.m_version;}

    protected:
        virtual void DealAccept(boost::asio::ip::tcp::socket& socket)
        {
            auto self(this->shared_from_this());
            auto conn = std::make_shared<KcSrvConnectTCP<decltype(*this)>>(*this, self, std::move(socket));
            conn->Start();
        }

        void do_accept()
        {
            m_acceptor.async_accept(
                [this](const boost::system::error_code& ec, boost::asio::ip::tcp::socket socket)
                {
                    if (!this->m_own.IsRunning()) return;
                    if (!ec)
                        DealAccept(socket);
                    else
                        std::cout << ec.message() << std::endl;
                    this->do_accept();
                });
        }

    protected:
        boost::asio::ip::tcp::acceptor m_acceptor;

    private:
        TShardPtrOwn<TOwn> m_keepOwn;
    };
    template<typename TOwn>
    class KcSrvHttps : public KcSrvHttp<TOwn>
    {
    public:
        // using KcSrvHttp<TOwn>::KcSrvHttp;
        KcSrvHttps(TShardPtrOwn<TOwn> own, boost::asio::io_context& io_context, unsigned short port)
            : KcSrvHttp<TOwn>(own, io_context, port)
        {
        }

        bool IsSSL(void) const override { return true; }

    protected:
        void DealAccept(boost::asio::ip::tcp::socket& socket) override
        {
            std::shared_ptr<boost::asio::ssl::context> ctx(new boost::asio::ssl::context(boost::asio::ssl::context::sslv23));
            ctx->set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv2 |
                boost::asio::ssl::context::single_dh_use);

            // ssl握手验证回调
            ctx->set_verify_mode(boost::asio::ssl::context::verify_none);
            ctx->set_verify_callback(
                [ctx](bool preverified, boost::asio::ssl::verify_context& ctxVerify)
                {
                    boost::asio::ssl::host_name_verification host_verification("https://127.0.0.1:18011/");
                    bool bSucc = host_verification(preverified, ctxVerify);

                    char subject_name[256];
                    X509* cert = X509_STORE_CTX_get_current_cert(ctxVerify.native_handle());
                    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
                    std::cout << "Verifying " << bSucc << ": " << subject_name << "\n";
                    // 这里可以添加更多的验证逻辑，比如检查证书是否过期等。
                    // ctx->use_certificate_chain_file("./ssl2/fullchain.pem");
                    // ctx->use_private_key_file("./ssl2/private.key", boost::asio::ssl::context::pem);
                    return true; // 如果内置验证通过，或者你的自定义验证通过，返回 true。
                });

            // 证书文件
            ctx->use_certificate_chain_file(this->m_own.m_parm.sslCert);
            ctx->use_private_key_file(this->m_own.m_parm.sslKey, boost::asio::ssl::context::pem);
            // ctx->use_tmp_dh_file("./ssl/fullchain.pem");
            auto self(this->shared_from_this());
            auto conn = std::make_shared<KcSrvConnectSSL<decltype(*this)>>(*this, self, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(std::move(socket), *ctx), ctx);
            conn->Start();
        }
    };

    // 主控
    template<typename TOwn>
    class KcSrvMainExec : public std::enable_shared_from_this<KcSrvMainExec<TOwn>>
    {
    public:
        TOwn &m_own;
        TParmKcSrv m_parm;
        const std::string m_version = "Knewcode v1.2";

        KcSrvMainExec(TOwn &own, std::string sVersion, FRequestRespond frr) : m_own(own), m_version(sVersion), m_frr(frr)
        {
        }

        void Start(void)
        {
            auto self(this->shared_from_this());
            bool bRunHttp = false, bRunHttps = false;
            if (m_parm.portHttp > 0)
            {
                try
                {
                    m_srvHttp.reset(new KcSrvHttp(self, m_ioContext, m_parm.portHttp));
                    m_srvHttp->Start();
                    bRunHttp = true;
                }
                catch (...)
                {
                    std::cout << "Can't Run Http: " << m_parm.portHttp << std::endl;
                }
            }
            if (m_parm.portHttps > 0)
            {
                try
                {
                    m_srvHttps.reset(new KcSrvHttps(self, m_ioContext, m_parm.portHttps));
                    m_srvHttps->Start();
                    bRunHttps = true;
                }
                catch (...)
                {
                    std::cout << "Can't Run Https: " << m_parm.portHttps << std::endl;
                }
            }
            m_running = bRunHttp || bRunHttps;
            if (m_running)
            {
                m_thrdIoCtx.resize(m_parm.threadCount);
                for (auto &thrd : m_thrdIoCtx)
                    thrd.reset(new boost::thread([this, self](){ this->m_ioContext.run(); }));
            }
        }
        void Stop(void)
        {
            m_running = false;
            if (!m_ioContext.stopped()) m_ioContext.stop();
            for (auto &thrd : m_thrdIoCtx)
                if (thrd->joinable()) thrd->timed_join(boost::posix_time::milliseconds(555));
            m_thrdIoCtx.clear();
            m_srvHttp.reset();
            m_srvHttps.reset();
        }
        void Block(void)
        {
            m_ioContext.run();
        }

        bool IsRunning(void) { return m_running; }

        void RequestRespond(KcSrvRespondPtr res)
        {
            if (res.get() != nullptr) m_frr(res);
        }

    protected:
        boost::asio::io_context m_ioContext;
        std::shared_ptr<KcSrvHttp<KcSrvMainExec<TOwn>>> m_srvHttp;
        std::shared_ptr<KcSrvHttps<KcSrvMainExec<TOwn>>> m_srvHttps;
        std::vector<std::shared_ptr<boost::thread>> m_thrdIoCtx;
        std::atomic_bool m_running = false;
        FRequestRespond m_frr;
    };
    template<typename TOwn>
    using KcSrvMainExecPtr = std::shared_ptr<KcSrvMainExec<TOwn>>;
}
