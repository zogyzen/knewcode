#pragma once

#include <stdint.h>

// 回调函数格式声明
//extern "C"
//{
    // 每个回调函数固定的前2个参数依次为：本回调函数的参数数量；标记名称。调用方的回调函数被触发时，需判断这2个参数是否正确。

    ////////////////////////////////////////////////// 重要 //////////////////////////////////////////////////
    // 获取服务器配置信息的回调函数指针类型。参数依次为：3；"FGetSrvInfoCB"；信息名称（详见文件下方的附表1）。返回信息内容。
    typedef const char* (*FGetSrvInfoCB)(int argCount, const char* flag, const char* name);

    // 获取链接信息的回调函数指针类型。参数依次为：4；"FGetConnInfoCB"；链接的句柄；信息名称（详见文件下方的附表2）。返回信息内容。
    typedef const char* (*FGetConnInfoCB)(int argCount, const char* flag, intptr_t hConn, const char* name);

    // 获取请求头信息的回调函数指针类型。参数依次为：4；"FGetRequestHeadCB"；请求的句柄；信息名称。返回信息内容。
    typedef const char* (*FGetRequestHeadCB)(int argCount, const char* flag, intptr_t hRequest, const char* name);
    // 获取请求内容（post参数、多表单数据、其他二进制流等）的回调函数指针类型。参数依次为：3；"FGetRequestBodyCB"；请求的句柄。返回内容。
    // 内容的类型可通过头名称“Content-Type”获取，内容的长度可通过“Content-Length”获取。
    typedef const char* (*FGetRequestBodyCB)(int argCount, const char* flag, intptr_t hRequest);
    // 获取其他请求信息（get参数、本地文件等）的回调函数指针类型。参数依次为：4；"FGetRequestOtherCB"；请求的句柄；信息名称（详见文件下方的附表3）。返回信息内容。
    typedef const char* (*FGetRequestOtherCB)(int argCount, const char* flag, intptr_t hRequest, const char* name);
    // 获取请求的链接句柄的回调函数指针类型。参数依次为：3；"FGetRequestConnCB"；请求的句柄。返回链接句柄。
    typedef intptr_t (*FGetRequestConnCB)(int argCount, const char* flag, intptr_t hRequest);

    // 设置应答头信息的回调函数指针类型。参数依次为：5；"FSetRespondHeadCB"；请求的句柄；头名称；内容（为空时，删除）。返回错误码：0无错误。
    typedef int (*FSetRespondHeadCB)(int argCount, const char* flag, intptr_t hRequest, const char* name, const char* val);
    // 设置应答内容的回调函数指针类型。参数依次为：5；"FSetRespondBodyCB"；请求的句柄；内容的类型（json数据、二进制流等）；内容；内容的字节数。返回错误码：0无错误。
    typedef int (*FSetRespondBodyCB)(int argCount, const char* flag, intptr_t hRequest, const char* content, int len);
    // 设置其他应答信息（200或500状态等）的回调函数指针类型。参数依次为：5；"FSetRespondOtherCB"；请求的句柄；信息名称（详见文件下方的附表4）；内容（为空时，删除）。返回错误码：0无错误。
    typedef int (*FSetRespondOtherCB)(int argCount, const char* flag, intptr_t hRequest, const char* name, const char* val);
    // 设置提交应答的回调函数指针类型。参数依次为：3；"FSetCommitResponseCB"；请求的句柄。返回错误码：0无错误。
    typedef int (*FSetCommitResponseCB)(int argCount, const char* flag, intptr_t hRequest);

    // 设置通过错误码判断是否掉线的回调函数指针类型。参数依次为：3；"FSetIsDisconnByErrCodeCB"；错误码。返回：true已掉线；false未掉线。
    typedef bool (*FSetIsDisconnByErrCodeCB)(int argCount, const char* flag, int errCode);
    // 设置通过错误码得到错误信息的回调函数指针类型。参数依次为：3；"FSetGetStatusByErrCodeCB"；错误码。返回错误信息。
    typedef const char* (*FSetGetStatusByErrCodeCB)(int argCount, const char* flag, int errCode);

    ////////////////////////// 仅用于Server-Sent Events、Websocket、以及长连接的数据缓存 //////////////////////////
    // 设置链接为长连接的回调函数指针类型。参数依次为：4；"FSetKeepaliveCB"；链接的句柄；启用或关闭长连接。返回错误码：0无错误。
    typedef int (*FSetKeepaliveCB)(int argCount, const char* flag, intptr_t hConn, bool enable);
    // 设置断开的回调函数的函数指针参数（供调用方回调）
    typedef bool (*FBreakConnCBFromOwn)(intptr_t hInstance);
    // 设置连接断开事件的回调函数。参数依次为：5；"FSetBreakConnCB"；链接的句柄；回调的实例句柄；回调函数。返回错误码：0无错误。
    typedef int (*FSetBreakConnCB)(int argCount, const char* flag, intptr_t hConn, intptr_t hInstance, FBreakConnCBFromOwn fcb);
    // 关闭连接的回调函数。参数依次为：3；"FCloseConnCB"；链接的句柄。返回错误码：0无错误。
    typedef int (*FCloseConnCB)(int argCount, const char* flag, intptr_t hConn);
    // 得到是否关闭连接的回调函数。参数依次为：3；"FIsCloseConnCB"；链接的句柄。返回：true连接已关闭；false未关闭。
    typedef bool (*FIsCloseConnCB)(int argCount, const char* flag, intptr_t hConn);
    // 重置链接时间的回调函数。参数依次为：4；"FResetConnTimeCB"；请求的句柄；超时的时间（毫秒）。返回错误码：0无错误。
    typedef int (*FResetConnTimeCB)(int argCount, const char* flag, intptr_t hRequest, unsigned ms);
    // 通过链接发送数据的回调函数。参数依次为：5；"FSendByConnCB"；请求的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
    typedef int (*FSendByConnCB)(int argCount, const char* flag, intptr_t hRequest, const char* buf, unsigned len);
    // 通过链接进行Socket发送数据的回调函数。参数依次为：5；"FSockSendByConnCB"；链接的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
    typedef int (*FSockSendByConnCB)(int argCount, const char* flag, intptr_t hConn, const char* buf, unsigned len);
    // 通过链接进行websocket接收数据的回调函数。参数依次为：5；"FWscRecvByConnCB"；请求的句柄；接收缓冲器；缓存区最大字节数。返回实际接收数据的字节数。
    typedef int (*FWscRecvByConnCB)(int argCount, const char* flag, intptr_t hRequest, char* buf, unsigned len);

    // 附表1：服务器配置信息名称列表
    /*
        // 重要信息
        Version                         —— Web应用服务器的版本信息，例如，Apache v2.4.62
        MainExeModRoot                  —— Web应用服务器的所在目录。例如，apache的目录“D:\Apache24x64”。（如果使用相对路径，以当前路径为基准）
        KnewcodeRoot                    —— Knewcode API所在的绝对目录。即kc_websrv_api.x.dll动态库所在目录。（如果使用相对路径，以MainExeModRoot为基准）
        DocumentRoot                    —— 前端主页目录。（如果使用相对路径，以MainExeModRoot为基准）
        PlatformRoot                    —— 整体应用的所在目录。即前后端的上一级目录。（如果使用相对路径，以MainExeModRoot为基准）
        KnewcodeCfgFile                 —— Knewcode API的配置文件。（如果使用相对路径，以MainExeModRoot为基准）
        VirtualPath                     —— Web应用服务器配置的所有虚拟目录。每行一个。每行的格式为：虚拟目录名 + 制表符（\t） + 本地绝对目录（如果使用相对路径，以MainExeModRoot为基准）。

        // 不重要信息
        server_defn_name                ——
        server_path                     ——
        server_admin                    —— Web应用服务器配置的管理员名称。
        server_hostname                 ——
        server_scheme                   ——
        server_timeout                  ——
        server_keep_alive_timeout       ——
        server_keep_alive               ——
    */

    // 附表2：链接信息名称列表
    /*
        // 重要信息
        ProtocolType                    —— 协议类型。https、http
        client_ip                       —— 客户端IP
        local_ip                        —— 服务器端IP
        client_port                     —— 客户端的端口
        connection_id                   —— 连接的唯一编号

        // 不重要信息
        connection_keepalive            —— 是否保持长连接
        connection_keepalives           —— 长连接的最大秒数
        connection_handle               —— 链接的句柄
        connection_remote_host          ——
        connection_remote_logname       ——
        connection_local_host           ——
        connection_local_addr           ——
        connection_socket               ——
        connection_local_ipaddr_ptr     ——
        connection_client_addr          ——
        connection_client_ipaddr_ptr    ——
    */

    // 附表3：其他请求信息名称列表
    /*
        // 重要信息
        method                          —— 请求类型。post、get、put等
        filename                        —— 请求连接对应的本地文件。例如，d:\website\backend\api\login.kc
        uri                             —— 请求连接的文件部分（不包含get参数）。例如，/api/login.kc
        args                            —— get参数字符串。例如，act=getuser
        unparsed_uri                    —— 完整的请求连接。例如，/api/login.kc?act=getuser
        the_request                     —— 完整的请求信息。例如，POST /api/login.kc?act=getuser HTTP/1.1
        handler                         —— 请求页面扩展名的别名。Web应用服务器，如果检测到请求的页面扩展名为“.kc”，则调用本平台处理，并且本信息固定设置为dlib_kc_extname
        hostname                        —— 请求连接上的服务器主机名、域名或IP
        parsed_uri_port                 —— 请求连接上的服务器端口

        // 不重要信息
        canonical_filename              —— 同filename
        protocol                        —— http协议版本。例如，HTTP/1.1
        proto_num                       —— http协议数字版本。例如，1001
        ap_auth_type                    ——
        user                            ——
        vlist_validator                 ——
        content_encoding                ——
        range                           ——
        content_type                    —— 内容类型。同头信息“Content-Type”
        content_length                  —— 内容字节数。同头信息“Content-Length”
        status_line                     ——
        finfo_fname                     —— 同filename
        finfo_name                      ——
        parsed_uri_scheme               ——
        parsed_uri_hostinfo             ——
        parsed_uri_user                 ——
        parsed_uri_password             ——
        parsed_uri_hostname             ——
        parsed_uri_port_str             —— 请求的服务器端口（字符串）
        parsed_uri_path                 —— 同uri
        parsed_uri_query                —— 同args
        parsed_uri_fragment             ——
        path_info                       ——
        useragent_host                  —— 同hostname
        useragent_ip                    —— 请求的客户端IP
        useragent_addr                  ——
    */

    // 附表4：其他应答信息名称列表
    /*
        // 重要信息
        respond_status                  —— 应答状态。例如，200成功；40x客户端请求错误（404未找到页面）；50x服务器端错误；等。

        // 不重要信息
    */
//}
