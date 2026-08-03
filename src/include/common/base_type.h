#pragma once

#include <stdexcept>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/process.hpp>

// 当前代码位置
#ifdef _MSC_VER     // icrosoft Visual C++
#define FUNCTION_DETAIL __FUNCSIG__
#else               // GCC、clang等
#define FUNCTION_DETAIL __PRETTY_FUNCTION__
#endif
#define __CURR_CODE_FLINE__ (__FILE__ + (":" + std::to_string(__LINE__)))
#define __CURR_CODE_PLACE__ (FUNCTION_DETAIL + (" \t@" + __CURR_CODE_FLINE__))
#define __CURR_CODE_PLACE_C__ __CURR_CODE_PLACE__.c_str()

namespace KC
{
    // 框架类型
    enum class EFrameworkType
    {
        eftContext = 0,     // 框架
        eftBundle,          // 模块
        eftService,         // 服务
        eftReference        // 引用
    };

    // 基础异常
	class TException : public std::runtime_error
	{
	public:
        TException(int id, std::string place, std::string msg, std::string name = "", std::string oth = "")
            : std::runtime_error(msg), m_id(id), m_place(place), m_name(name), m_OtherInfo(oth) {}
		virtual ~TException() throw () {}

        //virtual const char* what() const { return std::runtime_error::what(); }

        virtual std::string error_info(void) const
		{
            return std::string() + std::runtime_error::what()
               + "\r\n  🔔 [" + m_CurrPosInfo + "][" + boost::lexical_cast<std::string>(m_id)
               + "][" + ExceptType() + "]" + "][" + m_name + "] \r\n" + m_OtherInfo + "\n" + m_backtrace;
        }
		virtual int error_id(void) const { return m_id; }
        virtual std::string error_place(void) const { return "[" + m_place + " - " + std::to_string(m_lineCode) + "]"; }

    public:
        std::string& CurrPosInfo(void) { return m_CurrPosInfo; }
        std::string ExceptType(void) const { return typeid(*this).name(); }
        std::string& OtherInfo(void) { return m_OtherInfo; }
        std::string& BackTrace(void) { return m_backtrace; }
        int& LineCode(void) { return m_lineCode; }

	protected:
		const int m_id = 0;
        const std::string m_place = "", m_name = "";
        std::string m_CurrPosInfo = "", m_OtherInfo = "", m_backtrace = "";
        int m_lineCode = 0;
	};

    // 动态库扩展名
    #ifdef WIN32    // Windows环境
    constexpr char c_so_ext_name[] = ".dll";
    constexpr char c_LoadOutLibPrefixName[] = "";
    #else           // linux环境
    constexpr char c_so_ext_name[] = ".so";
    //constexpr char c_LoadOutLibPrefixName[] = "lib";
    constexpr char c_LoadOutLibPrefixName[] = "";
    #endif

    // web请求的名称组
    const std::string c_arrWebRequestNames[] =
    {
        "Version", "Protocol_Type", "KnewcodeRoot", "DocumentRoot", "PlatformRoot", "filename", "uri", "canonical_filename", "unparsed_uri", "Host", "hostname", "protocol",
        "proto_num", "ap_auth_type", "user", "vlist_validator", "content_encoding", "handler", "content_type", "content_length", "range", "status_line",
        "method", "the_request", "args", "finfo_fname", "finfo_name", "parsed_uri_scheme", "parsed_uri_hostinfo", "parsed_uri_user", "parsed_uri_password",
        "parsed_uri_hostname", "parsed_uri_port_str", "parsed_uri_path", "parsed_uri_query", "parsed_uri_fragment", "path_info", "useragent_ip", "useragent_host",
        "useragent_addr", "server_defn_name", "server_path", "server_admin", "server_hostname", "server_scheme", "server_timeout",
        "server_keep_alive_timeout", "server_keep_alive", "connection_handle", "connection_id", "connection_socket", "local_ip", "local_port", "client_ip", "client_port",
        "connection_remote_host", "connection_remote_logname", "connection_local_host", "connection_keepalive", "connection_keepalives", "connection_local_addr",
        "connection_local_ipaddr_ptr", "connection_client_addr", "connection_client_ipaddr_ptr", "User_Agent", "Accept", "Accept_Encoding", "Accept_Language", "Origin",
        "Referer", "X_Requested_With", "Cookie", "Connection", "Upgrade", "client_host_port", "server_host_port", "UniqueConnID"
    };
	
    // session尺寸
    constexpr unsigned c_KCSessionShareMemSize = 128 * 1024 * 1024;
    constexpr unsigned c_KCMaxParmBufSize = 32676;

    // // xint
    // constexpr wchar_t c_xlUser[] = L"Halil Kural";
    // constexpr wchar_t c_xlKey[] = L"windows-2723210a07c4e90162b26966a8jcdboe";

    // 分割线
    const char c_strLineShow[] = "\n----------------------------------------------------------------------------------------\n";

    // base62基础编码
    const char c_strCodeBase62x[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";

    // PlusAes加解密算法的IV：$MingYe@20151106
    const unsigned char c_plusAesIV[16] = {0x24, 0x4D, 0x69, 0x6E, 0x67, 0x59, 0x65, 0x40, 0x32, 0x30, 0x31, 0x35, 0x31, 0x31, 0x30, 0x36};

    // 默认aes加密算法的密钥（256位）
    const char c_strDefaultAesKey[] = "Zogyzen19750430my$KC@151106.Zzj`";

    // 默认需处理的Uri请求的扩展名
    const char c_DefaultWorkUriExtension[] = ".kc";

    // 常用web头
    const char c_WebHeader_ContentLength[] = "Content-Length";
    const char c_WebHeader_ContentType[] = "Content-Type";
    const char c_WebHeader_UserAgent[] = "User-Agent";

    // 默认Respond类型
    const char c_DefaultResponseContentType[] = "application/json";
    // 下载文本文件Respond类型
    const char c_DownTxtFileResponseContentType[] = "text/plain";
    // 下载二进制文件Respond类型
    const char c_OctetStreamResponseContentType[] = "application/octet-stream";
    // SSE的Respond类型
    const char c_SSEResponseContentType[] = "text/event-stream;charset=UTF-8";
    // WebSocket的Respond类型
    const char c_WebSocketResponseContentType[] = "application/vnd.kc.websocket";

    // cookie过期日期
    const char c_CookieExpiredDate[] = "Tue, 23 Apr %d75 08:08:08 GMT";

    // 请求应答头
    const char c_RespondHeaderResultJson[] = "Result-Json";
    const char c_RequestHeaderCookie[] = "Cookie";

    // websocket加密公钥
    const char c_WebSocketPublicKey[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    // 默认
    const char c_const_default[] = "default";

    // xml关键字
    const char c_RESTful_xmlattr[] = "<xmlattr>";           // 属性
    const char c_RESTful_xmlcomment[] = "<xmlcomment>";     // 注释

    // webapi名称
    const char c_RESTful_Controllers[] = "Controllers";     // .kc文件的根节点名称
    const char c_RESTful_controllers[] = "controllers";     // 配置文件中各模块的控制器节点名称
    const char c_RESTful_grpBranchCtrl[] = "grpBranchCtrl";
    const char c_RESTful_jsonCaseSensitive[] = "jsonCaseSensitive";
    const char c_RESTful_jsonLibrary[] = "jsonLibrary";
    const char c_RESTful_cJSON[] = "cJSON";
    const char c_RESTful_yyjson[] = "yyjson";

    const char c_RESTful_keepAliveTimeout[] = "keepAliveTimeout";
    const char c_RESTful_recursionLayerMax[] = "recursionLayerMax";

    const char c_RESTful_srvID[] = "srvID";
    const char c_RESTful_sysFlag[] = "sysFlag";
    const char c_RESTful_sign[] = "sign";
    const char c_RESTful_rootSign[] = "rootSign";
    const char c_RESTful_type[] = "type";
    const char c_RESTful_ctrl[] = "ctrl";
    const char c_RESTful_method[] = "method";
    const char c_RESTful_attach[] = "attach";
    const char c_RESTful_content[] = "content";
    const char c_RESTful_return[] = "return";
    const char c_RESTful_srv[] = "srv";
    const char c_RESTful_srvCtrl[] = "srvCtrl";
    const char c_RESTful_file[] = "file";
    const char c_RESTful_bundle[] = "bundle";
    const char c_RESTful_so[] = "so";
    const char c_RESTful_group[] = "group";
    const char c_RESTful_depend[] = "depend";
    const char c_RESTful_complete[] = "complete";

    const char c_RESTful_insert[] = "insert";
    const char c_RESTful_delete[] = "delete";
    const char c_RESTful_update[] = "update";
    const char c_RESTful_select[] = "select";
    const char c_RESTful_exec[] = "exec";
    const char c_RESTful_query[] = "query";
    const char c_RESTful_batch[] = "batch";
    const char c_RESTful_procedures[] = "procedures";
    const char c_RESTful_function[] = "function";
    const char c_RESTful_create[] = "create";
    const char c_RESTful_package[] = "package";

    const char c_RESTful_sql_DDL[] = "DDL";
    const char c_RESTful_sql_DQL[] = "DQL";
    const char c_RESTful_sql_DML[] = "DML";
    const char c_RESTful_sql_DCL[] = "DCL";

    const char c_RESTful_url[] = "url";
    const char c_RESTful_ignoreError[] = "ignoreError";
    const char c_RESTful_uri[] = "uri";
    const char c_RESTful_main[] = "main";
    const char c_RESTful_act[] = "act";
    const char c_RESTful_goTo[] = "goto";               // 控制器执行方式。（针对postgresql的临时函数，如果“goto=perform”则表示直接执行，否则仅为创建）
    const char c_RESTful_perform[] = "perform";
    const char c_RESTful_out[] = "out";
    const char c_RESTful_part[] = "part";

    const char c_RESTful_parallel[] = "parallel";
    const char c_RESTful_branch[] = "branch";
    const char c_RESTful_order[] = "order";
    const char c_RESTful_order_if[] = "if";
    const char c_RESTful_order_then[] = "then";
    const char c_RESTful_order_else[] = "else";
    const char c_RESTful_order_while[] = "while";
    const char c_RESTful_order_exec[] = "exec";

    const char c_RESTful_int[] = "int";
    const char c_RESTful_number[] = "number";
    const char c_RESTful_varchar[] = "varchar";
    const char c_RESTful_varchar2[] = "varchar2";
    const char c_RESTful_date[] = "date";
    const char c_RESTful_clob[] = "clob";
    const char c_RESTful_json[] = "json";

    const char c_RESTful_dbset[] = "dbset";
    const char c_RESTful_dbset_one[] = "dbset-one";
    const char c_RESTful_dbset_array[] = "dbset-array";
    const char c_RESTful_dbset_inner[] = "dbset-inner";
    const char c_RESTful_cursor[] = "cursor";
    const char c_RESTful_cursor_one[] = "cursor-one";
    const char c_RESTful_cursor_array[] = "cursor-array";
    const char c_RESTful_temp_table[] = "temp-table";
    const char c_RESTful_temp_table_one[] = "temp-table-one";
    const char c_RESTful_temp_table_array[] = "temp-table-array";
    const char c_RESTful_rset[] = "rset";

    const char c_RESTful_needToken[] = "needToken";
    const char c_RESTful_keepAlive[] = "keepAlive";

    const char c_RESTful_KCTmpTab[] = "KC__TmpTab__";
    const char c_RESTful_KCTmpObj[] = "KCTmp__";

    const char c_RESTful_batchRowID[] = "KC__Parm__BatchRowID";
    const char c_RESTful_batchRowCount[] = "KC__Parm__BatchRowCount";

    const char c_RESTful_outParm[] = "outParms";
    const char c_RESTful_parmType[] = "parmsType";
    const char c_RESTful_defParm[] = "defaultParms";
    const char c_RESTful_fixedParmName[] = "fixParmName";       // 固定字段的名称，可修改
    const char c_RESTful_inParm[] = "parms";
    const char c_RESTful_dbsetParms[] = "dbsetParms";

    const char c_RESTful_errCode[] = "errCode";                 // 默认错误码的字段名
    const char c_RESTful_errMsg[] = "errMsg";                   // 默认错误信息的字段名
    const char c_RESTful_logMsg[] = "logMsg";
    const char c_RESTful_RecCount[] = "recCount";
    const char c_RESTful_feilds[] = "feilds";
    const char c_RESTful_vals[] = "vals";
    const char c_RESTful_val[] = "val";

    const char c_RESTful_include[] = "include";

    const char c_RESTful_replace[] = "replace";
    const char c_RESTful_NoCheck[] = "NoCheck";                 // 不检查，完整替换
    const char c_RESTful_SQLInjection[] = "SQLInjection";       // 检查sql注入
    const char c_RESTful_OnlyName[] = "OnlyName";               // 只能替换名称

    const char c_RESTful_batchParm[] = "batchParm";
    const char c_RESTful_batchValsName[] = "batchValsName";
    const char c_RESTful_batchMethod[] = "batchMethod";         // 1：出错继续（默认）；2：出错停止；3：出错回滚（只针对带事务的SQL）

    const char c_RESTful_ExcelName[] = "ExcelName";
    const char c_RESTful_SheetName[] = "SheetName";

    const char c_RESTful_KCSession[] = "KC__SESSION_";
    const char c_RESTful_KCSessoinID[] = "KCSSID";
    const char c_RESTful_KCClientID[] = "KCCLNID";
    const char c_RESTful_KCGlobalKCSSID[] = "999";

    const char c_RESTful_KCAct[] = "KC__ACT__";
    const char c_RESTful_Charset[] = "charset";
    const char c_RESTful_UTF8[] = "UTF-8";
    const char c_RESTful_GBK[] = "GBK";
    const char c_RESTful_RequestID[] = "RequestID";
    const char c_RESTful_KCActionID[] = "ActionID";
    const char c_RESTful_KCConnectID[] = "ConnectID";
    const char c_RESTful_KCAliveID[] = "KCAliveID";
    const char c_RESTful_KCPID[] = "PID";
    const char c_RESTful_KCNowTimeFlag[] = "NowTimeFlag";
    const char c_RESTful_SrvDatatime[] = "SrvDatatime";
    const char c_RESTful_SrvTimestamp[] = "SrvTimestamp";

    const char c_RESTful_KCJSON[] = "KC__JSON__";

    const char c_RESTful_KCGlobalConst[] = "KC__GLOBAL_CONSTANT__";
    const char c_RESTful_KCConst[] = "KC__CONST__";
    const char c_RESTful_KCGetParm[] = "KC__GET_PARM__";
    const char c_RESTful_MultiFormDataCount[] = "MultiFormDataCount";
    const char c_RESTful_MultiFormDataPos[] = "MultiFormDataPos";
    const char c_RESTful_RootActUri[] = "RootActUri";

    const char c_RESTful_DB_mssql[] = "mssql";

    // 各插件错误码定义
    enum EErrCodeDefine
    {
        // 控制器
        ecd_ErrCode_Ctrl = 100,
        ecd_ErrCode_Ctrl_NotAssigned,               // 未指派，未设定 控制器
        ecd_ErrCode_Ctrl_NoExists,                  // 不存在控制器

        // Web主模块 插件
        ecd_ErrCode_KCWebMain = 600,
        // Webapi主控 插件
        ecd_ErrCode_KCWebApiWork = 1000,
        // Session 插件
        ecd_ErrCode_KCSessionCookie = 2000,
        // Session 插件
        ecd_ErrCode_KCSSEWS = 2200,
        // 系统文件处理 插件
        ecd_ErrCode_KCOSFile = 2400,
        // 系统命令 插件
        ecd_ErrCode_KCOSExec = 2600,
        // 转发 插件
        ecd_ErrCode_KCRequestRelay = 2800,

        // 脚本 插件
        ecd_ErrCode_KCChaiScript = 5000,

        // postgresql 插件
        ecd_ErrCode_KCSqlPostgresql = 6000,

        // oracle 插件
        ecd_ErrCode_KCSqlOracle = 6500,

        // 达梦 插件
        ecd_ErrCode_KCSqlDM = 7000,

        // sqlite 插件
        ecd_ErrCode_KCSqlSqlite = 7500,

        // odbc 插件
        ecd_ErrCode_KCSqlODBC = 9000,

        // 用户自定义错误码
        ecd_ErrCode_UserDefine = 10000
    };
}
