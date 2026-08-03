#include "apache_work.h"

////////////////////////////////////////////////////////////////////////////////
// apache工作类
CApacheWork::CApacheWork(module& m, server_rec& srv)
    : m_mod(m), m_srv(srv)
    // apache主目录
    , m_ApPath([&](){
        string sPth = g_strServerRootPath;
        if (boost::filesystem::exists(sPth)) sPth = boost::filesystem::canonical(sPth).string();
        return sPth;
    }())
    // knewcode模块所在目录
    , m_FxPath([&](){
        string sPth = g_strKCFullPath;
        sPth = CUtilFunc::ToAbsPath(sPth, m_ApPath);
        return sPth;
    }())
    // 网站或应用配置文件
    , m_CfgFile([&](){
        string sPth = g_strKCConfigFile;
        sPth = CUtilFunc::ToAbsPath(sPth, m_ApPath);
        return sPth;
    }())
    // 主页目录
    , m_PgPath([&](){
        string sPth = g_strPageRootFullPath;
        sPth = CUtilFunc::ToAbsPath(sPth, m_ApPath);
        return sPth;
    }())
    // 虚拟目录
    , m_VPath([&](){
        struct alias_entry
        {
            const char *real;
            const char *fake;
            char *handler;
            ap_regex_t *regexp;
            int redir_status;                /* 301, 302, 303, 410, etc */
        };
        struct alias_server_conf
        {
            apr_array_header_t *aliases;
            apr_array_header_t *redirects;
        };

        string sResult;
        module *m = ap_find_linked_module("mod_alias.c");
        alias_server_conf *sconf = (alias_server_conf*)ap_get_module_config(srv.module_config, m);
        alias_entry *modie = (alias_entry *)sconf->aliases->elts;
        for (int i = 0; i < sconf->aliases->nelts; i++)
        {
            alias_entry *modi = &modie[i];
            try
            {
                string sRealPath = modi->real;
                string sFullPath = algorithm::replace_first_copy(sRealPath, "${SRVROOT}", m_ApPath);
                if (!boost::filesystem::path(sFullPath).is_absolute())
                    sFullPath = CUtilFunc::FormatPath(boost::filesystem::canonical(m_ApPath + "/" + sFullPath).string());
                sResult +=  modi->fake + string("\t") + sFullPath + "\n";
                m_VPathLogs += (boost::format("\t%-20s\t%s (%S)\n") % modi->fake % sFullPath % sRealPath).str();
            }
            catch (std::exception &ex)
            {
                if (m_load.WriteLog(4, (format("Exception: [%s] %s") % typeid(ex).name() % ex.what()).str().c_str(), __CURR_CODE_PLACE_C__, (string(modi->fake) + " : " + modi->real).c_str()) != 0)
                    CTempLog::WriteInDir(m_FxPath, (format("Exception: [%s] %s") % typeid(ex).name() % ex.what()).str(), __CURR_CODE_PLACE_C__, string(modi->fake) + " : " + modi->real);
            }
            catch (...)
            {
                if (m_load.WriteLog(4, "Exception: [unknown]", __CURR_CODE_PLACE_C__, (string(modi->fake) + " : " + modi->real).c_str()) != 0)
                    CTempLog::WriteInDir(m_FxPath, "Exception: [unknown]", __CURR_CODE_PLACE_C__, string(modi->fake) + " : " + modi->real);
            }
        }
        cout << sResult << endl;
        return sResult;
    }())
    // 主平台目录，默认为“knewcode模块所在目录”的上一层目录
    , m_PlatformPath(boost::filesystem::path(m_FxPath).parent_path().string())
    // 网站或应用的根目录，默认为“网站或应用配置文件”所在目录
    , m_WebsitePath(boost::filesystem::path(m_CfgFile).parent_path().string())
{
    cout << "*[knewcode] load knewcode mod \n\t" << m_ApPath << "\n\t" << m_FxPath << "\n\t" << m_CfgFile << "\n\t"
         << m_PgPath << "\n\t" << m_PlatformPath << "\n\t" << m_WebsitePath << "\nVirtual Url Path:\n" << m_VPathLogs << endl;
}
CApacheWork::~CApacheWork()
{
    Free();
}
std::shared_ptr<CApacheWork> g_work;     // 主框架
std::weak_ptr<CApacheWork> CApacheWork::m_self(g_work);

// 初始化
void CApacheWork::Init(void)
{
    m_load.Init(m_FxPath);
    m_load.WriteLog(0, (boost::format("Apache   Path: %s (%s) \nKnewcode Path: %s (%s) \nConfig   Path: %s (%s) \nMainPage Path: %s (%s) \nPlatform Path: %s \nWebsite  Path: %s \n\nVirtual  Path:\n%s")
                        % m_ApPath % g_strServerRootPath
                        % m_FxPath % g_strKCFullPath
                        % m_CfgFile % g_strKCConfigFile
                        % m_PgPath % g_strPageRootFullPath
                        % m_PlatformPath % m_WebsitePath
                        % m_VPathLogs).str().c_str(), __CURR_CODE_PLACE_C__);
    m_VPathLogs.clear();
}

// 释放
void CApacheWork::Free(void)
{
    m_load.Free();
}

// 处理请求
int CApacheWork::Work(request_rec& r)
{
    return m_load.Request(reinterpret_cast<intptr_t>(&r));
}

// 判断是否ssl
bool CApacheWork::IsSSL(conn_rec& conn)
{
    // 获取mod_ssl模块里的可选函数
    APR_OPTIONAL_FN_TYPE(ssl_is_https) *get_optional_func = APR_RETRIEVE_OPTIONAL_FN(ssl_is_https);
    // 判断请求是否ssl
    if (nullptr != get_optional_func) return get_optional_func(&conn);
    return false;
}
// 按名称得到请求头
const char* CApacheWork::GetRequestHeader(request_rec& re, const char* name) const
{
    return apr_table_get(re.headers_in, name);
}
// 获取客户端post输入
const char* CApacheWork::GetClientBlock(request_rec& re) const
{
    // long long iPostLen = re.clength;
    long long iPostLen = atoi(GetRequestHeader(re, c_WebHeader_ContentLength));
    char* pBuf = new char[iPostLen + 1]{ 0 };
    boost::shared_array<char> _autoDel(pBuf);
    if(M_POST == re.method_number && OK == ap_setup_client_block(&re, REQUEST_CHUNKED_DECHUNK) && 1 == ap_should_client_block(&re))
        for (int i = 0, j = 0; j < iPostLen && (i = ap_get_client_block(&re, pBuf + j, iPostLen - j)) > 0; j += i);
    static thread_local string sResult;
    sResult.clear();
    sResult.append(pBuf, iPostLen);
    return sResult.c_str();
}
// 按名称添加、删除响应头
void CApacheWork::AddResponseHeader(request_rec& re, const char* name, const char* val)
{
    if (nullptr != name && nullptr != val)
    {
        string sName(name);
        if ("Set-Cookie" == sName)
            apr_table_add(re.headers_out, name, val);
        else
            apr_table_set(re.headers_out, name, val);
        // if (c_WebHeader_ContentType == sName)
        //     ap_set_content_type(&re, val);
    }
}
void CApacheWork::DelResponseHeader(request_rec& re, const char* name)
{
    if (nullptr != name)
    {
        apr_table_unset(re.headers_out, name);
        string sName(name);
        if (c_WebHeader_ContentType == sName)
        {
            ap_make_content_type(&re, NULL);
            //re.content_type = nullptr;
        }
    }
}
// 输出网页内容
bool CApacheWork::AddResponseBody(request_rec& re, const char* buf, int nbyte)
{
    bool bResult = false;
    if (nullptr != buf && nbyte > 0)
        bResult = ap_rwrite(buf, nbyte, &re) > 0;
    else if (nullptr != buf)
        bResult = ap_rputs(buf, &re) > 0;
    // websocket
    if (101 == re.status)
    {
        re.clength = 0;
        re.bytes_sent = 0;
        re.read_chunked = 0;
    }
    return bResult;
}
// 获取链接
apr_socket_t& CApacheWork::Sock(conn_rec& conn)
{
    apr_socket_t *sock = ap_get_conn_socket(&conn);
    if (nullptr == sock)
        throw std::runtime_error("nullptr apr_socket_t");
    return *sock;
}
// 设置长连接
void CApacheWork::SetKeepalive(conn_rec& conn, bool b)
{
    apr_socket_t& sck = Sock(conn);
    if (b)
    {
        conn.keepalive = AP_CONN_KEEPALIVE;
        apr_socket_opt_set(&sck, APR_SO_KEEPALIVE, 1);
        // // 设置永不超时
        // apr_socket_timeout_set(&sck, -1);
        // apr_socket_opt_set(&sck, APR_SO_NONBLOCK, 0);
        // // 非ssl，去掉输入输出过滤
        // if (!IsSSL())
        // {
        //     //m_r->output_filters = m_r->input_filters = nullptr;
        // }
        // m_conr.aborted = 0;
        // ap_rflush(&m_r);
    }
    else
    {
        conn.keepalive = AP_CONN_CLOSE;
        apr_socket_opt_set(ap_get_conn_socket(&conn), APR_SO_KEEPALIVE, 0);
        // // 取消永不超时
        // apr_socket_timeout_set(&sck, 30);
        // apr_socket_opt_set(&sck, APR_SO_NONBLOCK, 1);
    }
}
apr_status_t CApacheWork::DisconCB(TBreakCB* brcb)
{
    if (nullptr == brcb || nullptr == brcb->fBreakCB || nullptr == brcb->cdb)
        return APR_OS_START_SYSERR;
    FBreakConnCBFromOwn fBreakCB = brcb->fBreakCB;
    void *cdb = brcb->cdb;
    delete brcb;
    return fBreakCB(reinterpret_cast<intptr_t>(cdb)) ? APR_SUCCESS : APR_OS_START_USERERR;
}
// 设置断开的回调函数
void CApacheWork::SetBreakCB(conn_rec& conn, void* cdb, FBreakConnCBFromOwn fcb)
{
    TBreakCB *pBrcb = new TBreakCB;
    pBrcb->cdb = cdb;
    pBrcb->fBreakCB = fcb;
    apr_socket_data_set(&Sock(conn), pBrcb, (format("c%d") % conn.id).str().c_str(), reinterpret_cast<apr_status_t(*)(void*)>(DisconCB));
}
// websocket接收
int CApacheWork::WscRecv(request_rec& re, char* buf, unsigned len)
{
    size_t sz = len;
    int stt = APR_ENOSTAT;
    conn_rec& conn = *re.connection;
    // 刷新链接时间
    re.request_time = apr_time_now() - apr_time_make(0, 6666);
    CAutoRelease _auto([&](){ re.request_time = re.mtime = apr_time_now() + apr_time_make(6, 666666); });
    // 非ssl
    if (!IsSSL(conn))
        stt = apr_socket_recv(&Sock(conn), buf, &sz);
    // 是ssl
    else
    {
        apr_bucket_brigade *brigade = apr_brigade_create(conn.pool, conn.bucket_alloc);
        CAutoRelease _auto([&](){ apr_brigade_destroy(brigade); });
        stt = ap_get_brigade(conn.input_filters, brigade, AP_MODE_READBYTES, APR_BLOCK_READ, 120);
        cout << stt << endl;
        if (APR_SUCCESS == stt && !APR_BRIGADE_EMPTY(brigade))
        {
            apr_bucket* bucket = APR_BRIGADE_FIRST(brigade);
            const char* data = nullptr;
            size_t data_length = 0;
            stt = apr_bucket_read(bucket, &data, &data_length, APR_BLOCK_READ);
            sz = min(sz, data_length);
            CAutoRelease _auto2([&](){ apr_bucket_delete(bucket); });
            cout << data << endl;
            if (APR_SUCCESS == stt) memcpy(buf, data, sz);
        }
    }
    return stt;
}

// 获取服务器配置信息的回调函数指针类型。参数依次为：3；"FGetSrvInfoCB"；信息名称。返回信息内容。
const char* CApacheWork::GetSrvInfoCB(string sName)
{
    const char* pResult = nullptr;
    if ("Name" == sName) pResult = "Apache";
    else if ("Version" == sName)            // apache版本
    {
        // ap_version_t version;
        // ap_get_server_revision(&version);
        // pResult = CUtilFunc::ToKeepStr<0>((boost::format("Apache v%d.%d.%d") % version.major % version.minor % version.patch).str());
        pResult = ap_get_server_description();
    }
    else if ("MainExeModRoot" == sName)     // apache主目录
        pResult = m_ApPath.c_str();
    else if ("KnewcodeRoot" == sName)       // knewcode模块所在目录，apache配置
        pResult = m_FxPath.c_str();
    else if ("PlatformRoot" == sName)       // 主平台目录，默认为“knewcode模块所在目录”的上一层目录
        pResult = m_PlatformPath.c_str();
    else if ("KnewcodeCfgFile" == sName)    // 网站或应用配置文件，apache配置
        pResult = m_CfgFile.c_str();
    else if ("WebsiteRoot" == sName)        // 网站或应用的根目录，默认为“网站或应用配置文件”所在目录
        pResult = m_WebsitePath.c_str();
    else if ("DocumentRoot" == sName)       // 主页目录，apache配置
        pResult = m_PgPath.c_str();
    else if ("VirtualPath" == sName)        // 虚拟目录，apache配置
        pResult = m_VPath.c_str();

    else if ("server_defn_name" == sName) pResult = m_srv.defn_name;
    else if ("server_path" == sName) pResult = m_srv.path;
    else if ("server_admin" == sName) pResult = m_srv.server_admin;
    else if ("server_hostname" == sName) pResult = m_srv.server_hostname;
    else if ("server_scheme" == sName) pResult = m_srv.server_scheme;
    else if ("server_timeout" == sName) pResult = CUtilFunc::ToKeepStr<3>(m_srv.timeout);
    else if ("server_keep_alive_timeout" == sName) pResult = CUtilFunc::ToKeepStr<4>(m_srv.keep_alive_timeout);
    else if ("server_keep_alive" == sName) pResult = CUtilFunc::ToKeepStr<5>(m_srv.keep_alive);
    return pResult;
}

// 获取链接信息
const char* CApacheWork::GetConnInfoCB(intptr_t hConn, string sName)
{
    conn_rec& conn = *reinterpret_cast<conn_rec*>(hConn);
    const char* pResult = nullptr;
    if ("ProtocolType" == sName) pResult = IsSSL(conn) ? "https" : "http";
    else if ("client_ip" == sName) pResult = conn.client_ip;
    else if ("local_ip" == sName) pResult = conn.local_ip;
    else if ("client_port" == sName && nullptr != conn.client_addr)
        pResult = CUtilFunc::ToKeepStr<20>(conn.client_addr->port);
    else if ("local_port" == sName && nullptr != conn.local_addr)
        pResult = CUtilFunc::ToKeepStr<21>(conn.local_addr->port);
    else if ("connection_id" == sName) pResult = CUtilFunc::ToKeepStr<22>((format("%ld") % conn.id).str());
    else if ("connection_keepalive" == sName) pResult = CUtilFunc::ToKeepStr<23>(conn.keepalive);
    else if ("connection_keepalives" == sName) pResult = CUtilFunc::ToKeepStr<24>(conn.keepalives);

    else if ("connection_handle" == sName) pResult = CUtilFunc::ToKeepStr<25>(hConn);
    else if ("connection_remote_host" == sName) pResult = conn.remote_host;
    else if ("connection_remote_logname" == sName) pResult = conn.remote_logname;
    else if ("connection_local_host" == sName) pResult = conn.local_host;
    else if ("connection_local_addr" == sName) pResult = CUtilFunc::ToKeepStr<26>((format("%X") % conn.local_addr).str());
    else if ("connection_socket" == sName) pResult = CUtilFunc::ToKeepStr<27>((format("%X") % ap_get_conn_socket(&conn)).str());
    else if ("connection_local_ipaddr_ptr" == sName && nullptr != conn.local_addr)
        pResult = CUtilFunc::ToKeepStr<28>((format("%X") % conn.local_addr->ipaddr_ptr).str());
    else if ("connection_client_addr" == sName) pResult = CUtilFunc::ToKeepStr<29>((format("%X") % conn.client_addr).str());
    else if ("connection_client_ipaddr_ptr" == sName && nullptr != conn.client_addr)
        pResult = CUtilFunc::ToKeepStr<30>((format("%X") % conn.client_addr->ipaddr_ptr).str());
    return pResult;
}

// 获取请求头信息
const char* CApacheWork::GetRequestHeadCB(intptr_t hRequest, string sName)
{
    return GetRequestHeader(*reinterpret_cast<request_rec*>(hRequest), sName.c_str());
}
// 获取请求内容（post参数、多表单数据、其他二进制流等）
const char* CApacheWork::GetRequestBodyCB(intptr_t hRequest)
{
    request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    return GetClientBlock(re);
}
// 获取请其他求信息（get参数、本地文件等）
const char* CApacheWork::GetRequestOtherCB(intptr_t hRequest, string sName)
{
    request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    const char* pResult = nullptr;
    if ("filename" == sName) pResult = re.filename;
    else if ("method" == sName) pResult = re.method;
    else if ("the_request" == sName) pResult = re.the_request;

    else if ("uri" == sName) pResult = re.uri;
    else if ("canonical_filename" == sName) pResult = re.canonical_filename;
    else if ("unparsed_uri" == sName) pResult = re.unparsed_uri;
    else if ("hostname" == sName) pResult = re.hostname;
    else if ("protocol" == sName) pResult = re.protocol;
    else if ("proto_num" == sName) pResult = CUtilFunc::ToKeepStr<50>(re.proto_num);
    else if ("ap_auth_type" == sName) pResult = re.ap_auth_type;
    else if ("user" == sName) pResult = re.user;
    else if ("vlist_validator" == sName) pResult = re.vlist_validator;
    else if ("content_encoding" == sName) pResult = re.content_encoding;
    else if ("handler" == sName) pResult = re.handler;
    else if ("range" == sName) pResult = re.range;
    else if ("content_type" == sName) pResult = re.content_type;
    else if ("content_length" == sName) pResult = CUtilFunc::ToKeepStr<51>(re.clength);
    else if ("status_line" == sName) pResult = re.status_line;
    else if ("args" == sName) pResult = re.args;
    else if ("finfo_fname" == sName) pResult = re.finfo.fname;
    else if ("finfo_name" == sName) pResult = re.finfo.name;
    else if ("parsed_uri_scheme" == sName) pResult = re.parsed_uri.scheme;
    else if ("parsed_uri_hostinfo" == sName) pResult = re.parsed_uri.hostinfo;
    else if ("parsed_uri_user" == sName) pResult = re.parsed_uri.user;
    else if ("parsed_uri_password" == sName) pResult = re.parsed_uri.password;
    else if ("parsed_uri_hostname" == sName) pResult = re.parsed_uri.hostname;
    else if ("parsed_uri_port_str" == sName) pResult = re.parsed_uri.port_str;
    else if ("parsed_uri_path" == sName) pResult = re.parsed_uri.path;
    else if ("parsed_uri_query" == sName) pResult = re.parsed_uri.query;
    else if ("parsed_uri_fragment" == sName) pResult = re.parsed_uri.fragment;
    else if ("path_info" == sName) pResult = re.path_info;
    else if ("useragent_ip" == sName) pResult = re.useragent_ip;
    else if ("useragent_host" == sName) pResult = re.hostname;
    else if ("useragent_addr" == sName) pResult = CUtilFunc::ToKeepStr<52>((format("%X") % re.useragent_addr).str());
    else if ("parsed_uri_port" == sName)
    {
        int result = IsSSL(*re.connection) ? 443 : 80;
        if (strlen(re.parsed_uri.port_str) > 0)
        try
        {
            result = lexical_cast<int>(re.parsed_uri.port_str);
        }
        catch(...) {}
        pResult = CUtilFunc::ToKeepStr<53>(result);
    }
    return pResult;
}
// 获取请求的链接句柄
intptr_t CApacheWork::GetRequestConnCB(intptr_t hRequest)
{
    request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    return reinterpret_cast<intptr_t>(re.connection);
}

// 设置应答头信息
int CApacheWork::SetRespondHeadCB(intptr_t hRequest, string name, string val)
{
    request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    if (val.empty()) DelResponseHeader(re, name.c_str());
    else AddResponseHeader(re, name.c_str(), val.c_str());
    return 0;
}
// 设置应答内容
int CApacheWork::SetRespondBodyCB(intptr_t hRequest, string content)
{
    return AddResponseBody(*reinterpret_cast<request_rec*>(hRequest), content.c_str(), static_cast<int>(content.size())) ? 0 : 1;
}
// 设置其他应答信息（200或500状态等）
int CApacheWork::SetRespondOtherCB(intptr_t hRequest, string name, string val)
{
    request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    if ("respond_status" == name) re.status = atoi(val.c_str());
    return 0;
}
// 提交应答
int CApacheWork::CommitResponseCB(intptr_t hRequest)
{
    return ap_rflush(reinterpret_cast<request_rec*>(hRequest));
}

// 通过错误码判断是否掉线
bool CApacheWork::IsDisconnByErrCodeCB(int stt)
{
    // 超时
    //bResult = 730060 == stt;
    //bResult = APR_OS_START_SYSERR + WSAETIMEDOUT == stt;
    //bResult = APR_STATUS_IS_TIMEUP(stt);
    //bResult = 730060 == stt || 730004 == stt;
    //bResult = APR_STATUS_IS_TIMEUP(stt) || APR_STATUS_IS_EINTR(stt) || APR_STATUS_IS_EOF(stt);
    // 断线
    return APR_STATUS_IS_ENOTSOCK(stt) || APR_STATUS_IS_ENOSOCKET(stt) || APR_STATUS_IS_EINVALSOCK(stt) || APR_STATUS_IS_ECONNRESET(stt) || APR_STATUS_IS_ECONNABORTED(stt);
}
// 通过错误码得到错误信息
const char* CApacheWork::GetStatusByErrCodeCB(int stt)
{
    if (APR_SUCCESS == stt) return nullptr;
    static thread_local string sResult;
    sResult.clear();
    char bufErr[1024] = { 0 };
    apr_strerror(stt, bufErr, 1023);
    sResult = bufErr;
    return sResult.c_str();
}

// 设置链接为长连接
int CApacheWork::SetKeepaliveCB(intptr_t hConn, bool enable)
{
    SetKeepalive(*reinterpret_cast<conn_rec*>(hConn), enable);
    return 0;
}
// 设置连接断开事件
int CApacheWork::SetBreakConnCB(intptr_t hConn, intptr_t hInstance, FBreakConnCBFromOwn fcb)
{
    SetBreakCB(*reinterpret_cast<conn_rec*>(hConn), reinterpret_cast<void*>(hInstance), fcb);
    return 0;
}
// 关闭连接
int CApacheWork::CloseConnCB(intptr_t hConn)
{
    try
    {
        apr_socket_close(&Sock(reinterpret_cast<conn_rec&>(hConn)));
        return 0;
    }
    catch (...) {}
    return 1;
}
// 得到是否关闭连接
bool CApacheWork::IsCloseConnCB(intptr_t hConn)
{
    return 1 == reinterpret_cast<conn_rec*>(hConn)->aborted;
}
// 重置链接时间
int CApacheWork::ResetConnTimeCB(intptr_t hRequest, unsigned ms)
{
    request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    apr_time_t newTime = apr_time_now() - apr_time_from_msec(ms);
    if (re.mtime < newTime) re.mtime = newTime;
    re.request_time = newTime;
    return 0;
}
// 通过链接发送数据
int CApacheWork::SendByConnCB(intptr_t hRequest, string buf)
{
    request_rec& re = *reinterpret_cast<request_rec*>(hRequest);
    int stt = ap_rwrite(buf.c_str(), static_cast<int>(buf.size()), &re);
    if (APR_OS_START_ERROR > stt) stt = ap_rflush(&re);
    return stt;
}
// 通过链接进行Socket发送数据
int CApacheWork::SockSendByConnCB(intptr_t hConn, string buf)
{
    size_t sz = buf.size();
    return apr_socket_send(&Sock(*reinterpret_cast<conn_rec*>(hConn)), buf.c_str(), &sz);
}
// 通过链接进行websocket接收数据
int CApacheWork::WscRecvByConnCB(intptr_t hRequest, char* buf, unsigned len)
{
    return WscRecv(*reinterpret_cast<request_rec*>(hRequest), buf, len);
}
