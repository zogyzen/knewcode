#pragma once

#include <iostream>
#include <string>
#include <deque>
#include <map>
#include <set>
#include <atomic>

#include <boost/system/error_code.hpp>
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
        <div style="text-align: center;"><a target="_blank" href="https://kc.gl">%5%</a></div>
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
        // 信息响应
        {100, "Continue"},                              // 初始请求已接受，客户端应继续发送剩余部分
        {101, "Switching Protocols"},                   // 服务器同意切换协议（如升级为 WebSocket）
        {102, "Processing"},                            // 处理将被继续执行（由WebDAV（RFC 2518）扩展的状态码）
        // 成功
        {200, "OK"},                                    // 请求成功
        {201, "Created"},                               // 请求成功且服务器创建了新的资源（常见于 POST）
        {202, "Accepted"},                              // 请求已接受，但处理尚未完成
        {203, "Non-Authoritative Information"},         // 服务器已成功处理了请求，但返回的实体头部元信息不是在原始服务器上有效的确定集合，而是来自本地或者第三方的拷贝
        {204, "No Content"},                            // 请求成功，但响应体无内容（常见于 DELETE）
        {205, "Reset Content"},                         // 服务器成功处理了请求，且没有返回任何内容
        {206, "Partial Content"},                       // 只返回文件部分内容（请求头Range的范围），用于断点续传
        {207, "Multi-Status"},                          // 之后的消息体将是一个XML消息，并且可能依照之前子请求数量的不同，包含一系列独立的响应代码（由WebDAV（RFC 2518）扩展的状态码）
        // 重定向
        {300, "Multiple Choices"},                      // 被请求的资源有一系列可供选择的回馈信息，每个都有自己特定的地址和浏览器驱动的商议信息
        {301, "Moved Permanently"},                     // 资源‌永久‌移动到新 URL，浏览器会自动跳转
        {302, "Found"},                                 // 资源‌临时‌移动到新 URL，客户端应继续使用原 URL
        {303, "See Other"},                             // 应使用 GET 方法访问新地址（常用于表单提交后跳转）
        {304, "Not Modified"},                          // 资源未修改，客户端可使用本地缓存
        {305, "Use Proxy"},                             // 被请求的资源必须通过指定的代理才能被访问
        {307, "Temporary Redirect"},                    // 临时重定向，保留原始请求方法
        {308, "Permanent Redirect"},                    // 永久重定向，保留原始请求方法
        // 客户端错误
        {400, "Bad Request"},                           // 请求语法错误或参数无效
        {401, "Unauthorized"},                          // 需要身份验证，未提供或无效
        {403, "Forbidden"},                             // 服务器理解请求，但‌拒绝执行‌（权限不足）
        {404, "Not Found"},                             // 服务器找不到请求的资源
        {405, "Method Not Allowed"},                    // 请求方法不被该资源支持
        {406, "Not Acceptable"},                        // 请求的资源的内容特性无法满足请求头中的条件，因而无法生成响应实体
        {407, "Proxy Authentication Required"},         // 客户端必须在代理服务器上进行身份验证
        {408, "Request Timeout"},                       // 请求超时
        {409, "Conflict"},                              // 由于和被请求的资源的当前状态之间存在冲突，请求无法完成
        {410, "Gone"},                                  // 资源已永久删除，且无转发地址
        {411, "Length Required"},                       // 服务器拒绝在没有定义 Content-Length 头的情况下接受请求
        {412, "Precondition Failed"},                   // 服务器在验证在请求的头字段中给出先决条件时，没能满足其中的一个或多个
        {413, "Request Entity Too Large"},              // 请求提交的实体数据大小超过了服务器愿意或者能够处理的范围
        {414, "Request-URI Too Long"},                  // 请求的URI长度超过了服务器能够解释的长度
        {415, "Unsupported Media Type"},                // 请求中提交的实体并不是服务器中所支持的格式
        {416, "Range Not Satisfiable"},                 // 在断点续传时，请求头Range的范围超出文件大小
        {417, "Expectation Failed"},                    // 请求头Expect中指定的预期内容无法被服务器满足
        {421, "Misdirected Request"},                   // 请求被指向到无法生成响应的服务器（比如由于连接重复使用）
        {423, "Locked"},                                // 当前资源被锁定。（RFC 4918 WebDAV）
        {424, "Failed Dependency"},                     // 由于之前的某个请求发生的错误，导致当前请求失败，例如 PROPPATCH。（RFC 4918 WebDAV）
        {425, "Too Early"},                             // 代表服务器不愿意冒风险来处理该请求，原因是处理该请求可能会被“重放”，从而造成潜在的重放攻击。（RFC 8470）
        {426, "Upgrade Required"},                      // 客户端应当切换到TLS/1.0。（RFC 2817）
        {429, "Too Many Requests"},                     // 请求频率过高，触发了速率限制
        {449, "Retry With"},                            // 由微软扩展，代表请求应当在执行完适当的操作后进行重试
        {451, "Unavailable For Legal Reasons"},         // 该请求因法律原因不可用。（RFC 7725）
        // 服务器错误
        {500, "Internal Server Error"},                 // 服务器遇到意外情况，无法完成请求
        {501, "Not Implemented"},                       // 服务器不支持请求的功能
        {502, "Bad Gateway"},                           // 作为网关的服务器收到上游服务器的无效响应
        {503, "Service Unavailable"},                   // 服务器暂时过载或正在维护
        {504, "Gateway Timeout"},                       // 网关服务器未及时从上游服务器获得响应
        {505, "HTTP Version Not Supported"},            // 服务器不支持请求的 HTTP 协议版本
        {506, "Variant Also Negotiates"},               // 服务器存在内部配置错误：被请求的协商变元资源被配置为在透明内容协商中使用自己，因此在一个协商处理中不是一个合适的重点。由《透明内容协商协议》（RFC 2295）扩展
        {507, "Insufficient Storage"},                  // 服务器无法存储完成请求所必须的内容。这个状况被认为是临时的。WebDAV (RFC 4918)
        {509, "Bandwidth Limit Exceeded"},              // 服务器达到带宽限制。（不是一个官方的状态码）
        {510, "Not Extended"},                          // 获取资源所需要的策略并没有被满足。（RFC 2774）
        {600, "Unparseable Response Headers"}           // 源站没有返回响应头部，只返回实体内容
    };

    // 用于启动内置web服务器的基本参数
    struct TParmKcSrv
    {
        // 线程数量
        unsigned threadCount = 32;
        // 端口
        unsigned short portHttp = 0, portHttps = 0;
        // 是否启动
        bool portHttpIsStart = false, portHttpsIsStart = false;
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
                // 去掉多余的斜杠
                while (m_unparsed_uri.find("//") != std::string::npos)
                    m_unparsed_uri = boost::algorithm::replace_all_copy(m_unparsed_uri, "//", "/");
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
                            if (iEnd >= static_cast<long long>(iSizeFile))
                                throw std::runtime_error(itRange->second + " Error. " + std::to_string(iEnd));
                            // 实际读取的大小
                            m_readSize = iEnd - m_readBegin + 1;
                            if (m_readSize <= 0) throw std::runtime_error(itRange->second + " Error. " + std::to_string(m_readSize));
                            SetHead(c_WebHeader_ContentRange, (boost::format("bytes %lld-%lld/%lld") % m_readBegin % iEnd % iSizeFile).str());
                            m_status = m_readSize < static_cast<long long>(iSizeFile) ? 206 : 200;
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
            if (!boost::filesystem::exists(sFile))
                SetErrorPage(sFile, 404, fErr);
            else if (!boost::filesystem::is_regular_file(sFile))
                SetErrorPage(sFile, 400, fErr);
            else
            {
                std::string extName = boost::algorithm::to_lower_copy(boost::filesystem::path(sFile).extension().string());
                // 文件的mime类型
                std::string sMimeType = !m_request->m_extName.empty() ? m_request->m_extName.substr(1) : (!extName.empty() ? extName.substr(1) : "");
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
                m_body = (boost::format(c_modPageErr) % ("❌" + std::to_string(errCode) + ": " + sFileLeaf + " - " + sSysName)
                          % ("\"" + sFileLeaf + "\" " + sStatus)
                          % errCode % sOther
                          % m_request->m_connect->GetSrvHttp().KnewcodeVersion()).str();
                m_filename.clear();
            }
            SetHead(c_WebHeader_ContentType, "text/html");
        }
    };
    typedef std::shared_ptr<KcSrvRespond> KcSrvRespondPtr;
    // 客户端连接的回调函数
    typedef std::function<bool(long, std::string, KcSrvConnectPtr)> FClientConnStart;
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
        virtual bool Start(void)
        {
            std::cout << "[" << this->GetID() << " / " << this->ClientIP() << "] Client Connect: \t" << std::hex << &this->m_socket << std::endl;
            return m_own.m_own.ClientConnStart(this->GetID(), this->ClientIP(), this->shared_from_this());
        }
        // 读请求
        void do_read(void)
        {
            try
            {
                if (!this->m_own.m_own.IsRunning()) return;
                auto self(this->shared_from_this());
                // 读请求头，使用动态自动扩展缓冲区
                std::shared_ptr<boost::asio::streambuf> bufReadPtr(new boost::asio::streambuf);
                // 读请求头，直到指定标记。（可能会多读一些数据）
                boost::asio::async_read_until(this->m_socket, *bufReadPtr, "\r\n\r\n",
                    [this, self, bufReadPtr](const boost::system::error_code& ec, std::size_t length)
                    {
                        try
                        {
                            if (!this->m_own.m_own.IsRunning()) return;
                            if (!ec)
                            {
                                this->m_readErrorCount = 0;
                                // boost::asio::streambuf::const_buffers_type bufReadData = bufReadPtr->data();
                                std::string dataHeader(buffers_begin(bufReadPtr->data()), buffers_begin(bufReadPtr->data()) + length);
                                bufReadPtr->consume(length);    // 消费掉请求头这部分数据
                                // bufReadPtr->commit(length);
                                  std::cout << "[" << this->GetID() << "] read: " << std::dec << bufReadPtr->size() << " / " << length
                                          << std::endl << dataHeader << std::endl;
                                KcSrvRequest *pReq = new KcSrvRequest(self, dataHeader);
                                KcSrvRequestPtr reqPtr(pReq);
                                // 如果存在多读数据，先放到请求体里
                                if (bufReadPtr->size() > 0)
                                    pReq->m_body.append(buffers_begin(bufReadPtr->data()), buffers_end(bufReadPtr->data()));
                                // 剩余未读数据
                                long long iResidue = reqPtr->m_ContentLength - bufReadPtr->size();
                                // 读请求体
                                if (iResidue > 0)
                                {
                                    boost::shared_array<char> strResidue(new char[iResidue + 1]{ 0 });
                                    boost::asio::async_read(this->m_socket, boost::asio::buffer(strResidue.get(), iResidue),
                                        [this, self, pReq, reqPtr, strResidue, iResidue](const boost::system::error_code& ec, std::size_t length)
                                        {
                                            try
                                            {
                                                if (!this->m_own.m_own.IsRunning()) return;
                                                if (!ec)
                                                {
                                                    this->m_readErrorCount = 0;
                                                    if (iResidue != length)
                                                        std::cout << (boost::format("? [%d / %s] Read Body: %d != %d") % this->GetID() % this->ClientIP() % iResidue % length).str() << std::endl;
                                                    // 拼请求体
                                                    pReq->m_body.append(strResidue.get(), length);
                                                    // 处理
                                                    this->Deal(reqPtr);
                                                }
                                                else
                                                {
                                                    std::cout << "[" << this->GetID() << " / " << ++this->m_readErrorCount << " / " << this->ClientIP() << "] Read Body: " << ec.message() << std::endl;
                                                    WaitNextRequest(self, ec, 555);
                                                }
                                            }
                                            catch (...)
                                            {
                                                std::cout << "[" << this->GetID() << " / " << this->ClientIP() << "] Read Body: Unknown Error" << std::endl;
                                                WaitNextRequest(self, ec, 555);
                                            }
                                        });
                                }
                                else this->Deal(reqPtr);
                            }
                            else
                            {
                                std::cout << "[" << this->GetID() << " / " << ++this->m_readErrorCount << " / " << this->ClientIP() << "] async_read_until Error: " << ec.value() << "-" << ec.message() << std::endl;
                                WaitNextRequest(self, ec, 555);
                            }
                        }
                        catch (...)
                        {
                            std::cout << "[" << this->GetID() << " / " << this->ClientIP() << "] async_read_until Error: Unknown Error" << std::endl;
                            WaitNextRequest(self, ec, 555);
                        }
                    });
            }
            catch (...)
            {
                std::cout << "[" << this->GetID() << "] Lost Connection" << std::endl;
                this->CloseConn();
            }
        }

        // 写应答
        void do_write(KcSrvRespondPtr res)
        {
            if (!m_own.m_own.IsRunning()) return;
            // char sBuf[] = "HTTP/1.1 200\r\nContent-Type: text/html\r\nContent-Length: 8\r\n\r\nhello kc";
            // char sBuf[] = "HTTP/1.1 200\r\nContent-Length: 0\r\n\r\n";
            // 判断是否重定向
            const bool bIsReLocation = res->m_status / 100 == 3;
            // 应答状态
            auto itStatus = c_mapStatus.find(res->m_status);
            std::string sStatus = c_mapStatus.end() != itStatus ? " " + itStatus->second : "";
            // 应答首行
            std::string sDataHead = (boost::format("%s %d%s") % res->m_httpVersion % res->m_status % sStatus).str();
            // 判断那些应答头不通过集合返回（需要根据后续条件返回）
            auto fCheckMapHead = [&](std::string sName) -> bool
            {
                // 返回的字节数通过应答体计算
                if (c_WebHeader_ContentLength == sName) return false;
                // 重定向涉及到的响应头，动态生成
                if (bIsReLocation && ("Connection" == sName)) return false;
                // 其余响应头通过集合返回
                return true;
            };
            // 应答头
            for (auto &h : res->m_mapHeader)
                if (fCheckMapHead(h.first)) sDataHead += "\r\n" + h.first + ": " + h.second;
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
                        std::cout << "[" << this->GetID() << "] Respond Error: " << res->m_request->m_the_request << std::endl << len << std::endl << ec.message() << std::endl;
                        WaitNextRequest(self, ec, 555);
                    }
                }
                catch (...)
                {
                    std::cout << "[" << this->GetID() << "] Respond Error - " << res->m_request->m_the_request << std::endl << len << std::endl;
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
            auto fSendStr2 = [&](std::string sHead, std::string sBody = "")
            {
                fSendStr(sHead, sBody, fWaitNextRequest,
                    [&](size_t size){
                        std::cout << "[" << this->GetID() << "] Respond: " << res->m_request->m_the_request << std::dec << " \t" << res->m_status << " \t" << size << std::endl;
                    });
            };
            // 重定向
            auto fReLocation = [&](void)
            {
                // 发送重定向
                sDataHead += "\r\nConnection: Close";
                sDataHead += "\r\nContent-Length: 0\r\n\r\n";
                fSendStr2(sDataHead);
            };
            // 返回应答体
            auto fRespondBody = [&](std::size_t iSize)
            {
                // 发送http头和体
                sDataHead += (boost::format("\r\n%s: %d\r\n\r\n") % c_WebHeader_ContentLength % iSize).str();
                fSendStr2(sDataHead, c_RequestMethod_head == res->m_request->m_method ? "" : res->m_body);
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
            // 重定向
            if (bIsReLocation) fReLocation();
            // 如果指定文件，通过文件返回
            else if (!res->m_filename.empty())
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
                if (!bIsBreakEC && m_readErrorCount < 60 && this->IsOpen())
                {
                    std::cout << "[" << this->GetID() << "] Wait Next Request - " << ims << std::endl;
                    boost::this_thread::sleep(boost::posix_time::milliseconds(ims));
                    this->do_read();
                }
                else
                {
                    std::cout << "[" << this->GetID() << " / " << this->m_readErrorCount << " / " << ec.value() << "] Lost Connection" << std::endl;
                    this->m_readErrorCount = 0;
                    this->CloseConn();
                }
            }
            catch (...)
            {
                this->CloseConn();
                std::cout << "[" << this->GetID() << " / " << this->m_readErrorCount << " / " << ec.value() << "] Lost Connection" << std::endl;
                this->m_readErrorCount = 0;
            }
        }

        // 代表连接断开的错误码
        virtual bool IsBreakErrCode(const boost::system::error_code& ec) const
        {
            return boost::asio::error::eof == ec                            // 错误码2。远端正常关闭连接（调用 close() 时触发）
                    || boost::asio::error::connection_aborted == ec         // 本地系统终止连接
                    || boost::asio::error::connection_reset == ec           // windows下10054，linux下104。远端TCP层发送RST暴力断开
                    || boost::asio::error::bad_descriptor == ec             // windows下10009，linux下9。在已关闭的套接字上执行读写操作
            ;
        }

    protected:
        TSock m_socket;
        static inline long s_id = 0;
        const long m_id = ++s_id;
        int m_readErrorCount = 0;

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

        bool Start(void) override
        {
            bool bResult = TParentClass::Start();
            if (bResult) this->do_read();
            return bResult;
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
            // ctx->set_password_callback(std::bind([&](void){ return "123"; }));
            if (this->m_own.m_own.m_fGetSSLPass) ctx->set_password_callback(this->m_own.m_own.m_fGetSSLPass);

            // ctx->load_verify_file("./ssl/fullchain.pem");

            // socket.set_verify_mode(boost::asio::ssl::verify_peer);
            // socket.set_verify_callback(boost::asio::ssl::host_name_verification("host.name"));
        }

        bool Start(void) override
        {
            bool bResult = TParentClass::Start();
            if (bResult) do_handshake();
            return bResult;
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
            // std::cout << "SSL Handshake" << std::endl;
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

        // 代表连接断开的错误码
        bool IsBreakErrCode(const boost::system::error_code& ec) const override
        {
            return TParentClass::IsBreakErrCode(ec)
                    || boost::asio::ssl::error::stream_truncated == ec
                    || boost::asio::ssl::error::unspecified_system_error == ec
            ;
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
            typedef KcSrvConnectTCP<decltype(*this)> TSrvConnTCP;
            TSrvConnTCP *pConn = nullptr;
            auto fFlag = [&](void)
            {
                return (boost::format("%X-%X") % (ptrdiff_t)&socket % (ptrdiff_t)pConn).str();
            };
            try
            {
                auto self(this->shared_from_this());
                auto conn = std::make_shared<TSrvConnTCP>(*this, self, std::move(socket));
                pConn = conn.get();
                conn->Start();
            }
            catch (std::exception &ex)
            {
                std::cout << ex.what() << std::endl;
                m_own.m_own.WriteLogError(ex.what(), __FUNCTION__, fFlag().c_str());
                if (nullptr != pConn) pConn->CloseConn();
            }
            catch (...)
            {
                std::cout << "unknown error" << std::endl;
                m_own.m_own.WriteLogError("unknown error", __FUNCTION__, fFlag().c_str());
                if (nullptr != pConn) pConn->CloseConn();
            }
        }

        void do_accept()
        {
            try
            {
                if (!this->m_own.IsRunning()) return;
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
            catch (std::exception &ex)
            {
                std::cout << ex.what() << std::endl;
                m_own.m_own.WriteLogError(ex.what(), __FUNCTION__);
                boost::this_thread::sleep(boost::posix_time::milliseconds(666));
                this->do_accept();
            }
            catch (...)
            {
                std::cout << "unknown error" << std::endl;
                m_own.m_own.WriteLogError("unknown error", __FUNCTION__);
                boost::this_thread::sleep(boost::posix_time::milliseconds(666));
                this->do_accept();
            }
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
            typedef KcSrvConnectSSL<decltype(*this)> TSrvConnSSL;
            TSrvConnSSL *pConn = nullptr;
            auto fFlag = [&](void)
            {
                return (boost::format("%X-%X") % &socket % pConn).str();
            };
            try
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
                auto conn = std::make_shared<TSrvConnSSL>(*this, self, boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(std::move(socket), *ctx), ctx);
                pConn = conn.get();
                conn->Start();
            }
            catch (std::exception &ex)
            {
                std::cout << ex.what() << std::endl;
                KcSrvHttp<TOwn>::m_own.m_own.WriteLogError(ex.what(), __FUNCTION__, fFlag().c_str());
                if (nullptr != pConn) pConn->CloseConn();
            }
            catch (...)
            {
                std::cout << "unknown error" << std::endl;
                KcSrvHttp<TOwn>::m_own.m_own.WriteLogError("unknown error", __FUNCTION__, fFlag().c_str());
                if (nullptr != pConn) pConn->CloseConn();
            }
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
        std::function<std::string(std::size_t, boost::asio::ssl::context_base::password_purpose)> m_fGetSSLPass;

        KcSrvMainExec(TOwn &own, std::string sVersion, FRequestRespond frr, FClientConnStart fcc = [](long, std::string, KcSrvConnectPtr){ return true; })
            : m_own(own), m_version(sVersion), m_frr(frr), m_fcc(fcc)
        {
        }

        void Start(void)
        {
            auto self(this->shared_from_this());
            bool bRunHttp = false, bRunHttps = false;
            m_parm.portHttpIsStart = m_parm.portHttp > 0;
            m_running = true;
            if (m_parm.portHttpIsStart)
            {
                try
                {
                    m_srvHttp.reset(new KcSrvHttp(self, m_ioContext, m_parm.portHttp));
                    m_srvHttp->Start();
                    bRunHttp = true;
                }
                catch (std::exception &ex)
                {
                    std::cout << "Can't Run Http: " << m_parm.portHttp << ". \t" << ex.what() << std::endl;
                }
                catch (...)
                {
                    std::cout << "Can't Run Http: " << m_parm.portHttp << std::endl;
                }
            }
            m_parm.portHttpIsStart = bRunHttp;
            m_parm.portHttpsIsStart = m_parm.portHttps > 0 && boost::filesystem::exists(m_parm.sslCert) && boost::filesystem::exists(m_parm.sslKey);
            if (m_parm.portHttpsIsStart)
            {
                try
                {
                    m_srvHttps.reset(new KcSrvHttps(self, m_ioContext, m_parm.portHttps));
                    m_srvHttps->Start();
                    bRunHttps = true;
                }
                catch (std::exception &ex)
                {
                    std::cout << "Can't Run Https: " << m_parm.portHttps << ". \t" << ex.what() << std::endl;
                }
                catch (...)
                {
                    std::cout << "Can't Run Https: " << m_parm.portHttps << std::endl;
                }
            }
            m_parm.portHttpsIsStart = bRunHttps;
            m_running = bRunHttp || bRunHttps;
            if (m_running)
            {
                m_thrdIoCtx.resize(m_parm.threadCount);
                for (auto &thrd : m_thrdIoCtx)
                    thrd.reset(new boost::thread([this, self](){ this->Block(); }));
            }
        }
        void Stop(void)
        {
            m_running = false;
            if (!m_ioContext.stopped()) m_ioContext.stop();
            for (auto &thrd : m_thrdIoCtx)
                if (thrd->joinable()) thrd->timed_join(boost::posix_time::milliseconds(66));
            boost::this_thread::sleep(boost::posix_time::milliseconds(66));
            m_thrdIoCtx.clear();
            m_srvHttp.reset();
            m_srvHttps.reset();
        }

        void Block(void)
        {
            while (m_running)
            try
            {
                m_ioContext.run();
                boost::this_thread::sleep(boost::posix_time::milliseconds(66));
            }
            catch (std::exception &ex)
            {
                std::string sErr = std::string("[boost::asio::io_context] Run Exception - ") + ex.what();
                std::cout << sErr << std::endl;
                m_own.WriteLogError(sErr.c_str(), __FUNCTION__);
                boost::this_thread::sleep(boost::posix_time::milliseconds(6666));
            }
            catch (...)
            {
                std::string sErr = "[boost::asio::io_context] Run Exception";
                std::cout << sErr << std::endl;
                m_own.WriteLogError(sErr.c_str(), __FUNCTION__);
                boost::this_thread::sleep(boost::posix_time::milliseconds(6666));
            }
        }

        bool IsRunning(void) { return m_running; }

        void RequestRespond(KcSrvRespondPtr res)
        {
            if (res.get() != nullptr)
            try
            {
                m_frr(res);
            }
            catch (std::exception &ex)
            {
                std::cout << ex.what() << std::endl;
                m_own.WriteLogError(ex.what(), __FUNCTION__, res->m_request->m_unparsed_uri.c_str());
                res->SetErrorPage(res->m_request->m_uri, 500);
            }
            catch (...)
            {
                std::cout << "unknown error" << std::endl;
                m_own.WriteLogError("unknown error", __FUNCTION__, res->m_request->m_unparsed_uri.c_str());
                res->SetErrorPage(res->m_request->m_uri, 500);
            }
        }

        bool ClientConnStart(long id, std::string clnIP, KcSrvConnectPtr conn)
        {
            try
            {
                return m_fcc(id, clnIP, conn);
            }
            catch (std::exception &ex)
            {
                std::cout << ex.what() << std::endl;
                m_own.WriteLogError(ex.what(), __FUNCTION__);
            }
            catch (...)
            {
                std::cout << "unknown error" << std::endl;
                m_own.WriteLogError("unknown error", __FUNCTION__);
            }
        }

    protected:
        boost::asio::io_context m_ioContext;
        std::shared_ptr<KcSrvHttp<KcSrvMainExec<TOwn>>> m_srvHttp;
        std::shared_ptr<KcSrvHttps<KcSrvMainExec<TOwn>>> m_srvHttps;
        std::vector<std::shared_ptr<boost::thread>> m_thrdIoCtx;
        std::atomic_bool m_running = false;
        FRequestRespond m_frr;
        FClientConnStart m_fcc;
    };
    template<typename TOwn>
    using KcSrvMainExecPtr = std::shared_ptr<KcSrvMainExec<TOwn>>;
}
