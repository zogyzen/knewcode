#include "nginx_work.h"

////////////////////////////////////////////////////////////////////////////////
// nginx 工作类
CNginxWork::CNginxWork(void)
    // nginx主目录
    : m_NgxPath([&](){
        string sPth = CNginxHelper::s_strServerRootPath;
        if (boost::filesystem::exists(sPth)) sPth = boost::filesystem::canonical(sPth).string();
        return sPth;
    }())
    // knewcode模块所在目录
    , m_FxPath([&](){
        string sPth = CNginxHelper::s_strKCSoPath;
        sPth = boost::filesystem::path(CUtilFunc::ToAbsPath(sPth, m_NgxPath)).parent_path().string();
        return sPth;
    }())
    // 网站或应用配置文件
    , m_CfgFile([&](){
        string sPth = CNginxHelper::s_strKCConfigFile;
        sPth = CUtilFunc::ToAbsPath(sPth, m_NgxPath);
        return sPth;
    }())
    // 主页目录
    , m_PgPath([&](){
        string sPth = CNginxHelper::s_strPageRootFullPath;
        sPth = CUtilFunc::ToAbsPath(sPth, m_NgxPath);
        return sPth;
    }())
    // 虚拟目录
    , m_VPath([&](){
        string sResult;
        vector<string> vctItm;
        boost::algorithm::split(vctItm, CNginxHelper::s_strVPathConfig, boost::is_any_of("\n"));
        for (auto itm : vctItm)
        {
            vector<string> vctVP;
            boost::algorithm::split(vctVP, itm, boost::is_any_of("\t"));
            string sUri = boost::algorithm::trim_copy(vctVP[0]);
            if ("/" != sUri) sUri = boost::trim_right_copy_if(sUri, boost::is_any_of("/\\"));
            if (vctVP.size() == 2 && !sUri.empty())
            {
                string sLocal = boost::algorithm::trim_copy(vctVP[1]);
                if ("/" != sLocal) sLocal = boost::trim_right_copy_if(sLocal, boost::is_any_of("/\\"));
                string sLocalFull = boost::filesystem::path(sLocal).is_absolute() ? sLocal : CUtilFunc::ToAbsPath(sLocal, m_NgxPath);
                sResult += sUri + "\t" + sLocalFull + "\n";
            }
        }
        return sResult;
    }())
    // 主平台目录，默认为“knewcode模块所在目录”的上一层目录
    , m_PlatformPath(boost::filesystem::path(m_FxPath).parent_path().string())
    // 网站或应用的根目录，默认为“网站或应用配置文件”所在目录
    , m_WebsitePath(boost::filesystem::path(m_CfgFile).parent_path().string())
{
    cout << "*[knewcode] load knewcode mod \n\t" << m_NgxPath << "\n\t" << m_FxPath << "\n\t" << m_CfgFile << "\n\t"
         << m_PgPath << "\n\t" << m_PlatformPath << "\n\t" << m_WebsitePath << "\nVirtual Url Path:\n" << CNginxHelper::s_strVPathConfig << endl;
}
CNginxWork::~CNginxWork(void)
{
    Free();
}

// 主框架
std::shared_ptr<CNginxWork> g_work;
std::weak_ptr<CNginxWork> CNginxWork::m_self(g_work);

// 初始化
void CNginxWork::Init(void)
{
    m_load.Init(m_FxPath);
    m_load.WriteLog(0, (boost::format("Nginx    Path: %s (%s) \nKnewcode Path: %s (%s) \nConfig   Path: %s (%s) \nMainPage Path: %s (%s) \nPlatform Path: %s \nWebsite  Path: %s \n\nVirtual  Path:\n%s\n\n%s")
                        % m_NgxPath % CNginxHelper::s_strServerRootPath
                        % m_FxPath % CNginxHelper::s_strKCSoPath
                        % m_CfgFile % CNginxHelper::s_strKCConfigFile
                        % m_PgPath % CNginxHelper::s_strPageRootFullPath
                        % m_PlatformPath % m_WebsitePath
                        % m_VPath % CNginxHelper::s_strVPathConfig).str().c_str(), __CURR_CODE_PLACE_C__);
}

// 释放
void CNginxWork::Free(void)
{
    m_load.Free();
}

// 处理请求
int CNginxWork::Work(TNgxRequestData &r)
{
    t_ngxReqData = &r;

    // m_load.WriteLog(0, (boost::format("Begin Request - %s") % (char*)(r.unparsed_uri.data)).str().c_str(), __CURR_CODE_PLACE_C__, std::to_string(r.method).c_str());
    // CAutoRelease _auto([&](){ m_load.WriteLog(0, (boost::format("End Request - %s") % (char*)(r.unparsed_uri.data)).str().c_str(), __CURR_CODE_PLACE_C__, std::to_string(r.method).c_str()); });

    // string s = R"({"a":123})";
    // CNginxHelper::FAddResponseBody(&r, s.c_str(), s.size());
    // return;

    // 带请求体的请求（post、put）
    if (r.m_methodIsPost)
    {
        if (nullptr == CNginxHelper::NgxInfo().ngx_http_read_client_request_body)
            throw std::runtime_error("Uninitialized - " + __CURR_CODE_PLACE__);
        // intptr_t iResult = 0;
        // for (int i = 0; i <= 3; ++i)
        // {
        //     boost::this_thread::sleep(boost::posix_time::milliseconds((int)std::pow(8, i)));
        //     iResult = CNginxHelper::NgxInfo().ngx_http_read_client_request_body(r.ngx_request_s, &GetClientBodyHandler);
        //     if (0 == iResult) break;
        //     cout << "\t" << i << ". Can't Get Request Body: " << iResult << "\t" << CUtilFunc::GetCurrentStampMS() << endl;
        //     if (nullptr != r.ngx_request_body) *r.ngx_request_body = nullptr;
        // }
        int iLoop = 0;
        const int iAgainSleepMS = 199;
        const int iMaxLoop = std::max(static_cast<int>(std::ceil(CNginxHelper::NgxInfo().m_client_body_timeout * 1.0 / iAgainSleepMS)), 10);
        bool bNgxAgain = false;
        intptr_t iResult = 0;
        do
        {
            cout << "20.[request post body get] " << reinterpret_cast<intptr_t>(&r) << " <" << CUtilFunc::CurrThreadID()
                 << "> (" << CNginxHelper::NgxStrToStdStr(t_ngxReqData->content_length, 12) << ") - " << CNginxHelper::NgxStrToStdStr(t_ngxReqData->unparsed_uri) << endl;
            iResult = CNginxHelper::NgxInfo().ngx_http_read_client_request_body(r.ngx_request_s, &GetClientBodyHandler);
            // if (-2 == iResult)
                return /*NGX_OK*/0;



            if (-2 != iResult || ++iLoop > iMaxLoop) break;
            // 等待再次尝试
            bNgxAgain = true;
            if (nullptr != r.ngx_request_body) *r.ngx_request_body = nullptr;
            // boost::this_thread::sleep(boost::posix_time::milliseconds((int)std::pow(2, iLoop)));
            boost::this_thread::sleep(boost::posix_time::milliseconds(iAgainSleepMS));
            cout << "Request Post, Read Client Request Body NGX_AGAIN: " << iLoop << endl;
        }
        while (-2 == iResult);
        // 其他错误
        if (0 != iResult)
        {
            string sErr = (boost::format("Request Post Fail, Can't Get Request Body: %d (%s)\n\t %s") % iResult
                        % CNginxHelper::NgxStrToStdStr(r.content_length) % CNginxHelper::NgxStrToStdStr(r.unparsed_uri)).str();
            m_load.WriteLog(4, sErr.c_str(), __CURR_CODE_PLACE_C__, "Post");
            if (bNgxAgain && nullptr != CNginxHelper::NgxInfo().ngx_http_finalize_request)
                CNginxHelper::NgxInfo().ngx_http_finalize_request(r.ngx_request_s, -1);
            throw std::runtime_error(sErr);
        }
        // 读取成功
        else
        {
            // 获取输入参数
            CNginxHelper::NgxInfo().GetRequestBody(r.ngx_request_s, &r);
            // string sTmpFileSrc = CNginxHelper::NgxStrToStdStr(r.m_tempFile);
            // string sTmpFile = CUtilFunc::ToAbsPath(sTmpFileSrc, m_NgxPath);
            // cout << (boost::filesystem::exists(sTmpFile) ? "" : "Not ") << "Exists - " << sTmpFile << endl;
            // 处理
            auto const iResult = g_work->m_load.Request(reinterpret_cast<intptr_t>(&r));
            if (bNgxAgain && nullptr != CNginxHelper::NgxInfo().ngx_http_finalize_request)
                CNginxHelper::NgxInfo().ngx_http_finalize_request(r.ngx_request_s, -4);
            return iResult;
        }

        // auto buf = r.request_body->bufs->buf;
        // if (buf->last - buf->start >= r.headers_in.content_length_n)
        // {
        //     // m_load.Request(reinterpret_cast<intptr_t>(&r));
        //     return NGX_DONE;
        // }
        // else return NGX_AGAIN;

        //ngx_int_t rc = ngx_http_read_unbuffered_request_body(r);
        //if (rc >= NGX_HTTP_SPECIAL_RESPONSE) return rc;
        //else return m_load.Request(reinterpret_cast<intptr_t>(&r));

        //return m_load.Request(reinterpret_cast<intptr_t>(&r));
        return static_cast<int>(iResult);
    }
    // GET请求
    else return m_load.Request(reinterpret_cast<intptr_t>(&r));
}
void CNginxWork::GetClientBodyHandler(void *rSrc)
{
    // boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    cout << "21.[request post body work] " << reinterpret_cast<intptr_t>(rSrc) << " <" << CUtilFunc::CurrThreadID() << ">" << endl;
    // printf("21.[request post body work] %d %s\n", CUtilFunc::CurrThreadID(), CNginxHelper::NgxStrToStdStr(t_ngxReqData->unparsed_uri).c_str());

    // CNginxHelper::NgxInfo().GetRequestBody(rSrc, t_ngxReqData);
    // printf("21.[request post work] %s\n", CNginxHelper::NgxStrToStdStr(t_ngxReqData->unparsed_uri).c_str());
    // auto const iResult = g_work->m_load.Request(reinterpret_cast<intptr_t>(t_ngxReqData));
    // if (iResult != -2 && nullptr != CNginxHelper::NgxInfo().ngx_http_finalize_request)
    //     CNginxHelper::NgxInfo().ngx_http_finalize_request(rSrc, iResult);



    // 获取输入参数
    CNginxHelper::NgxInfo().GetRequestBody(rSrc, t_ngxReqData);
    // string sTmpFileSrc = CNginxHelper::NgxStrToStdStr(r.m_tempFile);
    // string sTmpFile = CUtilFunc::ToAbsPath(sTmpFileSrc, m_NgxPath);
    // cout << (boost::filesystem::exists(sTmpFile) ? "" : "Not ") << "Exists - " << sTmpFile << endl;

    // 处理
    g_work->m_load.Request(reinterpret_cast<intptr_t>(t_ngxReqData));
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));

    // if (nullptr != CNginxHelper::NgxInfo().ngx_http_finalize_request)
    //     CNginxHelper::NgxInfo().ngx_http_finalize_request(rSrc, /*NGX_DONE*/-4);

}

// 判断是否ssl
bool CNginxWork::IsSSL(TNgxConnection& conn)
{
    return conn.m_isSSL;
}

// 获取服务器配置信息
const char* CNginxWork::GetSrvInfo(string sName)
{
    const char* pResult = nullptr;
    if ("Name" == sName) pResult = "nginx";
    else if ("Version" == sName)            // nginx版本
        pResult = CNginxHelper::NgxStrToStd<0>(CNginxHelper::NgxInfo().m_strNginxVersion);
    else if ("MainExeModRoot" == sName)     // nginx主目录
        pResult = m_NgxPath.c_str();
    else if ("KnewcodeRoot" == sName)       // knewcode模块所在目录，nginx配置
        pResult = m_FxPath.c_str();
    else if ("PlatformRoot" == sName)       // 主平台目录，默认为“knewcode模块所在目录”的上一层目录
        pResult = m_PlatformPath.c_str();
    else if ("KnewcodeCfgFile" == sName)    // 网站或应用配置文件，nginx配置
        pResult = m_CfgFile.c_str();
    else if ("WebsiteRoot" == sName)        // 网站或应用的根目录，默认为“网站或应用配置文件”所在目录
        pResult = m_WebsitePath.c_str();
    else if ("DocumentRoot" == sName)       // 主页目录，nginx配置
        pResult = m_PgPath.c_str();
    else if ("VirtualPath" == sName)        // 虚拟目录，nginx配置
        pResult = m_VPath.c_str();

    // else if ("server_defn_name" == sName) pResult = m_srv.defn_name;
    // else if ("server_path" == sName) pResult = m_srv.path;
    // else if ("server_admin" == sName) pResult = m_srv.server_admin;
    // else if ("server_hostname" == sName) pResult = m_srv.server_hostname;
    // else if ("server_scheme" == sName) pResult = m_srv.server_scheme;
    // else if ("server_timeout" == sName) pResult = CUtilFunc::ToKeepStr<2>(m_srv.timeout);
    // else if ("server_keep_alive_timeout" == sName) pResult = CUtilFunc::ToKeepStr<3>(m_srv.keep_alive_timeout);
    // else if ("server_keep_alive" == sName) pResult = CUtilFunc::ToKeepStr<4>(m_srv.keep_alive);
    return pResult;
}
// 获取链接信息
const char* CNginxWork::GetConnInfo(TNgxConnection& conn, string sName)
{
    const char* pResult = nullptr;
    if ("ProtocolType" == sName) pResult = IsSSL(conn) ? "https" : "http";
    else if ("client_ip" == sName) pResult = CNginxHelper::NgxStrToStd<50>(conn.m_clientIP);
    else if ("local_ip" == sName)
    {
        string sLocal = CNginxHelper::NgxStrToStdStr(conn.m_localIP);
        std::vector<string> vctLocal;
        boost::algorithm::split(vctLocal, sLocal, boost::is_any_of(":"));
        pResult = CUtilFunc::ToKeepStr<51>(vctLocal[0]);
    }
    else if ("client_port" == sName)
    {
        unsigned short iPort = conn.m_clientPort;
        iPort = CUtilFunc::RotateT(iPort);
        pResult = CUtilFunc::ToKeepStr<52>(iPort);
    }
    else if ("local_port" == sName)
    {
        unsigned short iPort = conn.m_localPort;
        iPort = CUtilFunc::RotateT(iPort);
        pResult = CUtilFunc::ToKeepStr<53>(iPort);
    }
    else if ("connection_id" == sName) pResult = CUtilFunc::ToKeepStr<54>((format("%lld") % conn.m_id).str());
    else if ("connection_keepalive" == sName) pResult = CUtilFunc::ToKeepStr<55>(conn.ngx_keepalive);
    else if ("connection_keepalives" == sName) pResult = CUtilFunc::ToKeepStr<56>(conn.ngx_keepalive);

    // else if ("connection_handle" == sName) pResult = CUtilFunc::ToKeepStr<24>(hConn);
    // else if ("connection_remote_host" == sName) pResult = conn.remote_host;
    // else if ("connection_remote_logname" == sName) pResult = conn.remote_logname;
    // else if ("connection_local_host" == sName) pResult = conn.local_host;
    // else if ("connection_local_addr" == sName) pResult = CUtilFunc::ToKeepStr<25>((format("%X") % conn.local_addr).str());
    // else if ("connection_socket" == sName) pResult = CUtilFunc::ToKeepStr<26>((format("%X") % ap_get_conn_socket(&conn)).str());
    // else if ("connection_local_ipaddr_ptr" == sName && nullptr != conn.local_addr)
    //     pResult = CUtilFunc::ToKeepStr<27>((format("%X") % conn.local_addr->ipaddr_ptr).str());
    // else if ("connection_client_addr" == sName) pResult = CUtilFunc::ToKeepStr<28>((format("%X") % conn.client_addr).str());
    // else if ("connection_client_ipaddr_ptr" == sName && nullptr != conn.client_addr)
    //     pResult = CUtilFunc::ToKeepStr<29>((format("%X") % conn.client_addr->ipaddr_ptr).str());
    return pResult;
}

// 按名称得到请求头
const char* CNginxWork::GetRequestHeader(TNgxRequestData& re, string sName) const
{
    const char* pResult = nullptr;
    if ("Host" == sName)
        pResult = CNginxHelper::NgxStrToStd<101>(re.host);
    else if ("Connection" == sName)
        pResult = CNginxHelper::NgxStrToStd<102>(re.connection);
    else if ("If-Modified-Since" == sName)
        pResult = CNginxHelper::NgxStrToStd<103>(re.if_modified_since);
    else if ("If-Unmodified-Since" == sName)
        pResult = CNginxHelper::NgxStrToStd<104>(re.if_unmodified_since);
    else if ("If-Match" == sName)
        pResult = CNginxHelper::NgxStrToStd<105>(re.if_match);
    else if ("If-None-Match" == sName)
        pResult = CNginxHelper::NgxStrToStd<106>(re.if_none_match);
    else if (c_WebHeader_UserAgent == sName)
        pResult = CNginxHelper::NgxStrToStd<107>(re.user_agent);
    else if ("Referer" == sName)
        pResult = CNginxHelper::NgxStrToStd<108>(re.referer);
    else if (c_WebHeader_ContentLength == sName)
        pResult = CNginxHelper::NgxStrToStd<109>(re.content_length);
    else if ("Content-Range" == sName)
        pResult = CNginxHelper::NgxStrToStd<110>(re.content_range);
    else if (c_WebHeader_ContentType == sName)
        pResult = CNginxHelper::NgxStrToStd<111>(re.content_type);
    else if ("Range" == sName)
        pResult = CNginxHelper::NgxStrToStd<112>(re.range);
    else if ("If-Range" == sName)
        pResult = CNginxHelper::NgxStrToStd<113>(re.if_range);
    else if ("Transfer-Encoding" == sName)
        pResult = CNginxHelper::NgxStrToStd<114>(re.transfer_encoding);
    else if ("TE" == sName)
        pResult = CNginxHelper::NgxStrToStd<115>(re.te);
    else if ("Expect" == sName)
        pResult = CNginxHelper::NgxStrToStd<116>(re.expect);
    else if ("Upgrade" == sName)
        pResult = CNginxHelper::NgxStrToStd<117>(re.upgrade);
    else if ("Accept-Encoding" == sName)
        pResult = CNginxHelper::NgxStrToStd<118>(re.accept_encoding);
    else if ("Via" == sName)
        pResult = CNginxHelper::NgxStrToStd<119>(re.via);
    else if ("Authorization" == sName)
        pResult = CNginxHelper::NgxStrToStd<120>(re.authorization);
    else if ("Keep-Alive" == sName)
        pResult = CNginxHelper::NgxStrToStd<121>(re.keep_alive);
    else if ("X-Forwarded-For" == sName)
        pResult = CNginxHelper::NgxStrToStd<122>(re.x_forwarded_for);
    /*
    else if ("X-Real-IP" == sName)
        pResult = NgxTeltToStdStr<123>(re.headers_in.x_real_ip);
    else if ("Accept" == sName)
        pResult = NgxTeltToStdStr<124>(re.headers_in.accept);
    else if ("Accept-Language" == sName)
        pResult = NgxTeltToStdStr<125>(re.headers_in.accept_language);
    else if ("Depth" == sName)
        pResult = NgxTeltToStdStr<126>(re.headers_in.depth);
    else if ("Destination" == sName)
        pResult = NgxTeltToStdStr<127>(re.headers_in.destination);
    else if ("Overwrite" == sName)
        pResult = NgxTeltToStdStr<128>(re.headers_in.overwrite);
    else if ("Date" == sName)
        pResult = NgxTeltToStdStr<129>(re.headers_in.date);
    */
    else if ("Cookie" == sName)
        pResult = CNginxHelper::NgxStrToStd<130>(re.cookie);
    // 其他请求头，遍历请求头链表
    else
    {
        // 请求头
        for (unsigned i = 0; i < re.m_headersInEltsCount; ++i)
        {
            TNgxTableElt &reqh = re.m_headersInElts[i];
            const string sKey = CNginxHelper::NgxStrToStdStr(reqh.key);
            if (sKey == sName)
            {
                pResult = CNginxHelper::NgxTeltToStdStr<199>(&reqh);
                break;
            }
        }
    }
    cout << "[GetRequestHeader] " << sName << " = " << CUtilFunc::PCharSafeToStr(pResult) << endl;
    return pResult;
}
const char* CNginxWork::GetRequesOther(TNgxRequestData& re, string sName) const
{
    const char* pResult = nullptr;
    // 请求的首行
    string sRequestLine = CNginxHelper::NgxStrToStdStr(re.request_line);
    // 请求类型
    auto iPos = sRequestLine.find(' ');
    string sResTp = string::npos != iPos ? sRequestLine.substr(0, iPos) : sRequestLine;
    // 其他请求属性
    if ("uri" == sName) pResult = CUtilFunc::ToKeepStr<201>(CNginxHelper::NgxStrToStd(re.uri));
    else if ("filename" == sName) pResult = CUtilFunc::ToKeepStr<202>(CNginxHelper::FMapUriToLocalPath(re));
    else if ("args" == sName) pResult = CUtilFunc::ToKeepStr<203>(CNginxHelper::NgxStrToStd(re.args));
    else if ("method" == sName) pResult = CUtilFunc::ToKeepStr<204>(sResTp);
    else if ("the_request" == sName) pResult = CUtilFunc::ToKeepStr<205>(sRequestLine);
    else if ("content_type" == sName) pResult = CNginxHelper::NgxStrToStd<206>(re.content_type);
    else if ("content_length" == sName) pResult = CNginxHelper::NgxStrToStd<207>(re.content_length);

    // else if ("canonical_filename" == sName) pResult = re.canonical_filename;
    else if ("unparsed_uri" == sName) pResult = CUtilFunc::ToKeepStr<206>(CNginxHelper::NgxStrToStd(re.unparsed_uri));
    // else if ("hostname" == sName) pResult = re.hostname;
    // else if ("protocol" == sName) pResult = re.protocol;
    // else if ("proto_num" == sName) pResult = CUtilFunc::ToKeepStr<50>(re.proto_num);
    // else if ("ap_auth_type" == sName) pResult = re.ap_auth_type;
    // else if ("user" == sName) pResult = re.user;
    // else if ("vlist_validator" == sName) pResult = re.vlist_validator;
    // else if ("content_encoding" == sName) pResult = re.content_encoding;
    // else if ("handler" == sName) pResult = re.handler;
    // else if ("range" == sName) pResult = re.range;
    // else if ("content_type" == sName) pResult = re.content_type;
    // else if ("content_length" == sName) pResult = CUtilFunc::ToKeepStr<51>(re.clength);
    // else if ("status_line" == sName) pResult = re.status_line;
    // else if ("finfo_fname" == sName) pResult = re.finfo.fname;
    // else if ("finfo_name" == sName) pResult = re.finfo.name;
    // else if ("parsed_uri_scheme" == sName) pResult = re.parsed_uri.scheme;
    // else if ("parsed_uri_hostinfo" == sName) pResult = re.parsed_uri.hostinfo;
    // else if ("parsed_uri_user" == sName) pResult = re.parsed_uri.user;
    // else if ("parsed_uri_password" == sName) pResult = re.parsed_uri.password;
    // else if ("parsed_uri_hostname" == sName) pResult = re.parsed_uri.hostname;
    // else if ("parsed_uri_port_str" == sName) pResult = re.parsed_uri.port_str;
    // else if ("parsed_uri_path" == sName) pResult = re.parsed_uri.path;
    // else if ("parsed_uri_query" == sName) pResult = re.parsed_uri.query;
    // else if ("parsed_uri_fragment" == sName) pResult = re.parsed_uri.fragment;
    // else if ("path_info" == sName) pResult = re.path_info;
    // else if ("useragent_ip" == sName) pResult = re.useragent_ip;
    // else if ("useragent_host" == sName) pResult = re.hostname;
    // else if ("useragent_addr" == sName) pResult = CUtilFunc::ToKeepStr<52>((format("%X") % re.useragent_addr).str());
    // else if ("parsed_uri_port" == sName)
    // {
    //     int result = IsSSL(*re.connection) ? 443 : 80;
    //     if (strlen(re.parsed_uri.port_str) > 0)
    //     try
    //     {
    //         result = lexical_cast<int>(re.parsed_uri.port_str);
    //     }
    //     catch(...) {}
    //     pResult = CUtilFunc::ToKeepStr<53>(result);
    // }
    return pResult;
}
// 获取客户端post输入
const char* CNginxWork::GetClientBlock(TNgxRequestData& re) const
{
    static thread_local std::string sResult;
    sResult.clear();
    // 从缓冲区
    for (auto pStep = &re.m_requestBody; nullptr != pStep; pStep = pStep->next)
        if (nullptr != pStep->buf.data && pStep->buf.len > 0)
            sResult.append(pStep->buf.data, pStep->buf.len);
    // 从临时文件
    if (re.m_tempFile.len > 0 && nullptr != re.m_tempFile.data)
    {
        // boost::this_thread::sleep(boost::posix_time::milliseconds(500));

        /*
        string sTmpFileSrc = CNginxHelper::NgxStrToStdStr(re.m_tempFile);
        string sTmpFile = CUtilFunc::ToAbsPath(sTmpFileSrc, m_NgxPath);
        string sLogMsg = "Read Nginx Temp File: " + sTmpFileSrc + "\n\t\t" + sTmpFile;
        WriteLog(0, sLogMsg, CNginxHelper::NgxStrToStdStr(re.unparsed_uri), __CURR_CODE_PLACE_C__);
        // 读临时文件
        if (boost::filesystem::exists(sTmpFile) && boost::filesystem::is_regular_file(sTmpFile))
        {
            string sCloneFile = sTmpFile + ".clone";
            boost::filesystem::copy_file(sTmpFile, sCloneFile, boost::filesystem::copy_options::overwrite_existing);
            CAutoRelease _auto([&](){ boost::filesystem::remove(sCloneFile); });
            string sContent = CUtilFunc::ReadFile3(sCloneFile);
            sResult.append(sContent);
        }
        else
        {
            string sErr = "Not Exists - " + sTmpFile;
            WriteLog(3, sErr, CNginxHelper::NgxStrToStdStr(re.unparsed_uri), __CURR_CODE_PLACE_C__);
        }
        */

        // 读临时文件
        if (nullptr != CNginxHelper::NgxInfo().ngx_read_file)
        {
            unsigned iSize = atoi(CNginxHelper::NgxStrToStdStr(re.content_length).c_str());
            unsigned char *pBuf = new unsigned char[iSize + 1]{ 0 };
            boost::shared_array<unsigned char> autoDel(pBuf);
            CNginxHelper::NgxInfo().ngx_read_file(re.m_tempFileHandle, pBuf, iSize, 0);
            sResult.append(reinterpret_cast<char*>(pBuf), iSize);
        }
    }
    if (sResult.empty()) cout << __CURR_CODE_PLACE_C__ << ": Read Nothing" << endl;
    return sResult.c_str();
}

// 按名称设置响应头（值为空，删除）
void CNginxWork::SetResponseHeader(TNgxRequestData& re, const char* name, const char* val)
{
    CNginxHelper::FSetResponseHeader(re, name, val);
}
// 输出网页内容
bool CNginxWork::AddResponseBody(TNgxRequestData& re, const char* buf, int nbyte)
{
    bool bResult = false;
    try
    {
        bResult = CNginxHelper::FAddResponseBody(re, buf, nbyte) == 0;
    }
    catch (std::exception& ex)
    {
        m_load.WriteLog(4, (boost::format("FAddResponseBody fail: <%s> %s") % typeid(ex).name() % ex.what()).str().c_str(), __CURR_CODE_PLACE_C__, CNginxHelper::NgxStrToStdStr(re.unparsed_uri).c_str());
    }
    catch (...)
    {
        m_load.WriteLog(4, (boost::format("FAddResponseBody fail: \n%s") % (buf != nullptr && nbyte > 0 ? string(buf, nbyte) : "")).str().c_str(), __CURR_CODE_PLACE_C__, CNginxHelper::NgxStrToStdStr(re.unparsed_uri).c_str());
    }
    return bResult;
}
// 获取链接
// apr_socket_t& CNginxWork::Sock(conn_rec& conn)
// {
//     apr_socket_t *sock = ap_get_conn_socket(&conn);
//     if (nullptr == sock)
//         throw std::runtime_error("nullptr apr_socket_t");
//     return *sock;
// }
// 设置长连接
void CNginxWork::SetKeepalive(/*conn_rec& conn, */bool /*b*/)
{
    // apr_socket_t& sck = Sock(conn);
    // if (b)
    // {
    //     conn.keepalive = AP_CONN_KEEPALIVE;
    //     apr_socket_opt_set(&sck, APR_SO_KEEPALIVE, 1);
    //     // // 设置永不超时
    //     // apr_socket_timeout_set(&sck, -1);
    //     // apr_socket_opt_set(&sck, APR_SO_NONBLOCK, 0);
    //     // // 非ssl，去掉输入输出过滤
    //     // if (!IsSSL())
    //     // {
    //     //     //m_r->output_filters = m_r->input_filters = nullptr;
    //     // }
    //     // m_conr.aborted = 0;
    //     // ap_rflush(&m_r);
    // }
    // else
    // {
    //     conn.keepalive = AP_CONN_CLOSE;
    //     apr_socket_opt_set(ap_get_conn_socket(&conn), APR_SO_KEEPALIVE, 0);
    //     // // 取消永不超时
    //     // apr_socket_timeout_set(&sck, 30);
    //     // apr_socket_opt_set(&sck, APR_SO_NONBLOCK, 1);
    // }
}
// apr_status_t CNginxWork::DisconCB(TBreakCB* brcb)
// {
//     if (nullptr == brcb || nullptr == brcb->fBreakCB || nullptr == brcb->cdb)
//         return APR_OS_START_SYSERR;
//     FBreakConnCBFromOwn fBreakCB = brcb->fBreakCB;
//     void *cdb = brcb->cdb;
//     delete brcb;
//     return fBreakCB(reinterpret_cast<intptr_t>(cdb)) ? APR_SUCCESS : APR_OS_START_USERERR;
// }
// 设置断开的回调函数
void CNginxWork::SetBreakCB(/*conn_rec& conn, */void* /*cdb*/, FBreakConnCBFromOwn /*fcb*/)
{
    // TBreakCB *pBrcb = new TBreakCB;
    // pBrcb->cdb = cdb;
    // pBrcb->fBreakCB = fcb;
    // apr_socket_data_set(&Sock(conn), pBrcb, (format("c%d") % conn.id).str().c_str(), reinterpret_cast<apr_status_t(*)(void*)>(DisconCB));
}
// websocket接收
int CNginxWork::WscRecv(/*request_rec& re, */char* /*buf*/, unsigned /*len*/)
{
    // size_t sz = len;
    // int stt = APR_ENOSTAT;
    // conn_rec& conn = *re.connection;
    // // 刷新链接时间
    // re.request_time = apr_time_now() - apr_time_make(0, 6666);
    // CAutoRelease _auto([&](){ re.request_time = re.mtime = apr_time_now() + apr_time_make(6, 666666); });
    // // 非ssl
    // if (!IsSSL(conn))
    //     stt = apr_socket_recv(&Sock(conn), buf, &sz);
    // // 是ssl
    // else
    // {
    //     apr_bucket_brigade *brigade = apr_brigade_create(conn.pool, conn.bucket_alloc);
    //     CAutoRelease _auto([&](){ apr_brigade_destroy(brigade); });
    //     stt = ap_get_brigade(conn.input_filters, brigade, AP_MODE_READBYTES, APR_BLOCK_READ, 120);
    //     cout << stt << endl;
    //     if (APR_SUCCESS == stt && !APR_BRIGADE_EMPTY(brigade))
    //     {
    //         apr_bucket* bucket = APR_BRIGADE_FIRST(brigade);
    //         const char* data = nullptr;
    //         size_t data_length = 0;
    //         stt = apr_bucket_read(bucket, &data, &data_length, APR_BLOCK_READ);
    //         sz = min(sz, data_length);
    //         CAutoRelease _auto2([&](){ apr_bucket_delete(bucket); });
    //         cout << data << endl;
    //         if (APR_SUCCESS == stt) memcpy(buf, data, sz);
    //     }
    // }
    // return stt;
    return 0;
}

// 获取服务器配置信息
const char* CNginxWork::GetSrvInfoCB(string sName)
{
    return GetSrvInfo(sName);
}
// 获取链接信息
const char* CNginxWork::GetConnInfoCB(intptr_t hConn, string sName)
{
    return GetConnInfo(*reinterpret_cast<TNgxConnection*>(hConn), sName);
}

// 获取请求头信息
const char* CNginxWork::GetRequestHeadCB(intptr_t hRequest, string sName)
{
    return GetRequestHeader(*reinterpret_cast<TNgxRequestData*>(hRequest), sName);
}
// 获取请求内容（post参数、多表单数据、其他二进制流等）
const char* CNginxWork::GetRequestBodyCB(intptr_t hRequest)
{
    return GetClientBlock(*reinterpret_cast<TNgxRequestData*>(hRequest));
}
// 获取请其他求信息（get参数、本地文件等）
const char* CNginxWork::GetRequestOtherCB(intptr_t hRequest, string sName)
{
    return GetRequesOther(*reinterpret_cast<TNgxRequestData*>(hRequest), sName);
}
// 获取请求的链接句柄
intptr_t CNginxWork::GetRequestConnCB(intptr_t hRequest)
{
    auto& re = *reinterpret_cast<TNgxRequestData*>(hRequest);
    return reinterpret_cast<intptr_t>(&re.m_connection);
}

// 设置应答头信息
int CNginxWork::SetRespondHeadCB(intptr_t hRequest, string name, string val)
{
    auto& re = *reinterpret_cast<TNgxRequestData*>(hRequest);
    SetResponseHeader(re, name.c_str(), val.c_str());
    return 0;
}
// 设置应答内容
int CNginxWork::SetRespondBodyCB(intptr_t hRequest, string content)
{
    return AddResponseBody(*reinterpret_cast<TNgxRequestData*>(hRequest), content.c_str(), static_cast<int>(content.size())) ? 0 : 1;
}
// 设置其他应答信息（200或500状态等）
int CNginxWork::SetRespondOtherCB(intptr_t hRequest, string name, string val)
{
    auto& re = *reinterpret_cast<TNgxRequestData*>(hRequest);
    if ("respond_status" == name) *re.m_responseStatus = atoi(val.c_str());
    return 0;
}
// 提交应答
int CNginxWork::CommitResponseCB(intptr_t hRequest)
{
    // return ap_rflush(reinterpret_cast<request_rec*>(hRequest));
    auto& re = *reinterpret_cast<TNgxRequestData*>(hRequest);
    CNginxHelper::NgxInfo().ngx_http_finalize_request(re.ngx_request_s, /*NGX_DONE*/-4);
    // CNginxHelper::NgxInfo().ngx_http_send_special(re.ngx_request_s, /*NGX_HTTP_LAST*/1);
    return 0;
}

// 通过错误码判断是否掉线
bool CNginxWork::IsDisconnByErrCodeCB(int /*stt*/)
{
    // 超时
    //bResult = 730060 == stt;
    //bResult = APR_OS_START_SYSERR + WSAETIMEDOUT == stt;
    //bResult = APR_STATUS_IS_TIMEUP(stt);
    //bResult = 730060 == stt || 730004 == stt;
    //bResult = APR_STATUS_IS_TIMEUP(stt) || APR_STATUS_IS_EINTR(stt) || APR_STATUS_IS_EOF(stt);
    // 断线
    // return APR_STATUS_IS_ENOTSOCK(stt) || APR_STATUS_IS_ENOSOCKET(stt) || APR_STATUS_IS_EINVALSOCK(stt) || APR_STATUS_IS_ECONNRESET(stt) || APR_STATUS_IS_ECONNABORTED(stt);
    return false;
}
// 通过错误码得到错误信息
const char* CNginxWork::GetStatusByErrCodeCB(int /*stt*/)
{
    static thread_local string sResult;
    sResult.clear();
    // if (APR_SUCCESS == stt) return nullptr;
    // char bufErr[1024] = { 0 };
    // apr_strerror(stt, bufErr, 1023);
    // sResult = bufErr;
    return sResult.c_str();
}

// 设置链接为长连接
int CNginxWork::SetKeepaliveCB(intptr_t /*hConn*/, bool /*enable*/)
{
    // SetKeepalive(*reinterpret_cast<conn_rec*>(hConn), enable);
    return 0;
}
// 设置连接断开事件
int CNginxWork::SetBreakConnCB(intptr_t /*hConn*/, intptr_t /*hInstance*/, FBreakConnCBFromOwn /*fcb*/)
{
    // SetBreakCB(*reinterpret_cast<conn_rec*>(hConn), reinterpret_cast<void*>(hInstance), fcb);
    return 0;
}
// 关闭连接
int CNginxWork::CloseConnCB(intptr_t /*hConn*/)
{
    // try
    // {
    //     apr_socket_close(&Sock(reinterpret_cast<conn_rec&>(hConn)));
    //     return 0;
    // }
    // catch (...) {}
    return 1;
}
// 得到是否关闭连接
bool CNginxWork::IsCloseConnCB(intptr_t /*hConn*/)
{
    // return 1 == reinterpret_cast<conn_rec*>(hConn)->aborted;
    return false;
}
// 重置链接时间
int CNginxWork::ResetConnTimeCB(intptr_t /*hRequest*/, unsigned /*ms*/)
{
    // request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    // apr_time_t newTime = apr_time_now() - apr_time_from_msec(ms);
    // if (re.mtime < newTime) re.mtime = newTime;
    // re.request_time = newTime;
    return 0;
}
// 通过链接发送数据
int CNginxWork::SendByConnCB(intptr_t /*hRequest*/, string /*buf*/)
{
    // request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    // int stt = ap_rwrite(buf.c_str(), static_cast<int>(buf.size()), &re);
    // if (APR_OS_START_ERROR > stt) stt = ap_rflush(&re);
    // return stt;
    return 0;
}
// 通过链接进行Socket发送数据
int CNginxWork::SockSendByConnCB(intptr_t /*hConn*/, string /*buf*/)
{
    // size_t sz = buf.size();
    // return apr_socket_send(&Sock(*reinterpret_cast<conn_rec*>(hConn)), buf.c_str(), &sz);
    return 0;
}
// 通过链接进行websocket接收数据
int CNginxWork::WscRecvByConnCB(intptr_t /*hRequest*/, char* /*buf*/, unsigned /*len*/)
{
    // return WscRecv(*reinterpret_cast<request_rec*>(hRequest), buf, len);
    return 0;
}
