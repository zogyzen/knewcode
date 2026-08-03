#pragma once

#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <ngx_struct_foruse.h>

// #include "util/nginx_struct.h"
#include "util/util_funcs.h"
#include "util/cross_platform.h"

namespace KC
{
    // nginx帮助类
    class CNginxHelper
    {
    public:
        // nginx内置函数（内置全局变量）
        // static inline TNgxInnerFunc s_ngxInnerFunc;

        // nginx服务信息
        static inline TNgxSrvInfo* s_ngxInfo = nullptr;
        static TNgxSrvInfo NgxInfo(void)
        {
            if (nullptr == s_ngxInfo) throw std::runtime_error("Uninitialized - " + __CURR_CODE_PLACE__);
            return *s_ngxInfo;
        }

        // 各路径
        static inline string s_strServerRootPath = CrossPlatform::GetExecutablePath();             // nginx 主目录
        static inline string s_strKCSoPath = "";                   // knewcode动态库模块路径
        static inline string s_strKCConfigFile = "";               // 配置文件完整路径，默认为当前“KC系统路径”下的“config.xml”
        static inline string s_strPageRootFullPath = "";           // 主页路径
        static inline string s_strVPathConfig = "";                // 虚拟路径配置（\t服务器名称/端口| ssl标志*、虚拟路径\t本地路径）。服务器用\r分隔，虚拟目录用\n分隔

    public:
        // “模块初始化”引出函数
        static void ModInit(TNgxSrvInfo &ngxInfo)
        {
            s_ngxInfo = &ngxInfo;
            // 获取各目录配置
            CNginxHelper::s_strServerRootPath = NgxStrToStdStr(ngxInfo.m_strServerRootPath);
            CNginxHelper::s_strKCSoPath = NgxStrToStdStr(ngxInfo.m_strSoPath);
            CNginxHelper::s_strKCConfigFile = NgxStrToStdStr(ngxInfo.m_strKCConfigFile);
            CNginxHelper::s_strPageRootFullPath = NgxStrToStdStr(ngxInfo.m_strPageRootFullPath);
            // 虚拟目录
            CNginxHelper::s_strVPathConfig = NgxStrToStdStr(ngxInfo.m_strVPathConfig);
        }

    public:
        // 在内存池里，生成ngx字符串
        static TNgxStr MakeNgxStr(void* pool, const char* srcBuf, unsigned iLen = 0)
        {
            TNgxStr result;
            if (0 == iLen && nullptr != srcBuf) iLen = static_cast<unsigned>(strlen(srcBuf));
            char* dstBuf = CUtilFunc::PCharSafeToStr(srcBuf).empty() ? nullptr : (char*)NgxInfo().ngx_pnalloc(pool, iLen + 1);
            if (dstBuf == nullptr)
            {
                result.data = nullptr;
                result.len = 0;
            }
            else
            {
                memset(dstBuf, 0, iLen + 1);
                memcpy(dstBuf, srcBuf, iLen);
                result.data = dstBuf;
                result.len = iLen;
            }
            return result;
        }

        // nginx字符串转换为std::string
        static std::string NgxStrToStdStr(const TNgxStr& src, const unsigned iMaxLen = 0)
        {
            return nullptr == src.data || 0 == src.len || (iMaxLen > 0 && src.len > iMaxLen)  ? std::string() : std::string((char*)src.data, src.len);
        };
        template<int flag = 0>
        static const char* NgxStrToStd(const TNgxStr& src)
        {
            static thread_local string sResult;
            return (sResult = NgxStrToStdStr(src)).c_str();
        };
        template<int flag>
        static const char* NgxTeltToStdStr(const TNgxTableElt* telt)
        {
            return nullptr == telt ? nullptr : CUtilFunc::ToKeepStr<flag>(NgxStrToStd(telt->value));
        }

        // 获取uri对应的本地路径
        static std::string FMapUriToLocalPath(TNgxRequestData& r)
        {
            std::string sResult;
            if (nullptr != r.ngx_request_s)
            {
                TNgxStr path;
                size_t root = 0;
                char* last = (char*)NgxInfo().ngx_http_map_uri_to_path(r.ngx_request_s, &path, &root, 0);
                if (nullptr != path.data && nullptr  != last) sResult.append(path.data, last);
            }
            return sResult;
        }

        // 应答输出头
        static TNgxTableElt* FAddResponseHeader(TNgxRequestData& re, const char* name, const char* val, bool addNull = false)
        {
            std::string sName = CUtilFunc::PCharSafeToStr(name);

            // todo: nginx1.30以上，不需要返回响应头Content-Length
            if (c_WebHeader_ContentLength == sName) return nullptr;

            // 返回响应头
            TNgxTableElt* header = nullptr;
            if (nullptr != re.ngx_headers_out && !sName.empty() && (addNull || !CUtilFunc::PCharSafeToStr(val).empty()))
            {
                header = (TNgxTableElt*)NgxInfo().ngx_list_push(re.ngx_headers_out);
                if (header != 0)
                {
                    header->next = nullptr;
                    header->lowcase_key = nullptr;
                    // header->key = CUtilFunc::PCharSafeToStr(val).empty() ? MakeNgxStr(re.ngx_pool, nullptr) : MakeNgxStr(re.ngx_pool, name, static_cast<unsigned>(strlen(name)));
                    header->key = MakeNgxStr(re.ngx_pool, name, static_cast<unsigned>(strlen(name)));
                    header->value = CUtilFunc::PCharSafeToStr(val).empty() ? MakeNgxStr(re.ngx_pool, nullptr) : MakeNgxStr(re.ngx_pool, val, static_cast<unsigned>(strlen(val)));
                    header->hash = 1;
                }
            }
            return header;
        }
        // 设置响应头
        static TNgxTableElt* FSetResponseHeader(TNgxRequestData& re, const char* name, const char* val = nullptr)
        {
            string sName = CUtilFunc::PCharSafeToStr(name);
            // cookie
            if ("Set-Cookie" == sName)
                return FAddResponseHeader(re, name, val);
            // 设置响应头
            auto fSetResHeader = [&](TNgxTableElt &resh)
            {
                // 删除
                if (CUtilFunc::PCharSafeToStr(val).empty())
                {
                    // resh.key.data = nullptr;
                    // resh.key.len = 0;
                    resh.value.data = nullptr;
                    resh.value.len = 0;
                }
                // 设置
                else
                    resh.value = MakeNgxStr(re.ngx_pool, val, static_cast<unsigned>(strlen(val)));
                return &resh;
            };
            // 已存在的响应头
            TNgxTableElt *resHeader = re.m_headersOutElts;
            for (int i = 0; nullptr != re.m_headersOutEltsCount && i < *re.m_headersOutEltsCount && nullptr != resHeader; ++i)
            {
                TNgxTableElt &resh = resHeader[i];
                const string sKey = NgxStrToStdStr(resh.key);
                if (sKey == sName)
                {
                    fSetResHeader(resh);
                    return &resh;
                }
            }
            // 服务器名
            if ("Server" == sName && nullptr != re.m_responseServer)
            {
                if (nullptr == *re.m_responseServer)
                {
                    if (nullptr != val && strlen(val) > 0)
                        return *re.m_responseServer = FAddResponseHeader(re, name, val);
                    else
                    {
                        TNgxTableElt *resHeader = re.m_headersOutElts;
                        for (int i = 0; nullptr != re.m_headersOutEltsCount && i < *re.m_headersOutEltsCount && nullptr != resHeader; ++i)
                        {
                            TNgxTableElt &resh = resHeader[i];
                            const string sKey = NgxStrToStdStr(resh.key);
                            if (sKey == "Knewcode-Api-Ver" || sKey == "Server-Api-Ext")
                                return *re.m_responseServer = &resh;
                        }
                    }
                }
                else
                    return fSetResHeader(**re.m_responseServer);
            }
            // 其他新的响应头
            return FAddResponseHeader(re, name, val);
        }
        // 删除响应头
        static void FDelResponseHeader(TNgxRequestData& re, const char* name)
        {
            FSetResponseHeader(re, name);
        }

        // 应答输出体
        static int FAddResponseBody(TNgxRequestData& r, const void* buf, unsigned len)
        {
            // return NgxInfo().AddResponseBody(r, buf, len);
            if (nullptr != r.ngx_request_s)
            {
                *r.m_content_length_n_out = len;

                auto rc = NgxInfo().ngx_http_send_header(r.ngx_request_s);
                // if(rc == NGX_ERROR || rc > NGX_OK || r->header_only)
                if (rc != 0)
                {
                    NgxInfo().WriteNgxLog(4, r.m_connection.ngx_log, (boost::format("Response Fail (%d), Can't Send Header - %s") % rc % NgxStrToStdStr(r.unparsed_uri)).str().c_str());
                    return static_cast<int>(rc);
                }

                auto b = NgxInfo().ngx_create_temp_buf(r.ngx_pool, len);
                if (b == nullptr)
                {
                    NgxInfo().WriteNgxLog(4, r.m_connection.ngx_log, (boost::format("Response Fail, Can't Create Temp Buf - %s") % NgxStrToStdStr(r.unparsed_uri)).str().c_str());
                    return 500;
                }

                // 输出
                NgxInfo().WriteNgxBuf(b, buf, len);

                TNgxChain out;
                out.buf = b;
                out.next = nullptr;

                return static_cast<int>(NgxInfo().ngx_http_output_filter(r.ngx_request_s, &out));
            }
            else
            {
                NgxInfo().WriteNgxLog(4, r.m_connection.ngx_log, (boost::format("Response Fail - %s\n%s") % NgxStrToStdStr(r.unparsed_uri) % (char*)buf).str().c_str());
                return 500;
            }
        }
    };
}
