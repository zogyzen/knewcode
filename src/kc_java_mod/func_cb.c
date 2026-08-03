#include "std.h"
#include "load_kc.h"
#include "func_cb.h"

// java回调函数指针
static struct TJavaCBPointer
{
    // 获取服务器配置信息的回调函数指针类型。参数依次为：3；"FGetSrvInfoCB"；信息名称（详见文件下方的附表1）。返回信息内容。
    FGetSrvInfoCB m_FGetSrvInfoCB;

    // 获取链接信息的回调函数指针类型。参数依次为：4；"FGetConnInfoCB"；链接的句柄；信息名称（详见文件下方的附表2）。返回信息内容。
    FGetConnInfoCB m_FGetConnInfoCB;

    // 获取请求头信息的回调函数指针类型。参数依次为：4；"FGetRequestHeadCB"；请求的句柄；信息名称。返回信息内容。
    FGetRequestHeadCB m_FGetRequestHeadCB;
    // 获取请求内容（post参数、多表单数据、其他二进制流等）的回调函数指针类型。参数依次为：3；"FGetRequestBodyCB"；请求的句柄。返回内容。
    // 内容的类型可通过头名称“Content-Type”获取，内容的长度可通过“Content-Length”获取。
    FGetRequestBodyCB m_FGetRequestBodyCB;
    // 获取其他请求信息（get参数、本地文件等）的回调函数指针类型。参数依次为：4；"FGetRequestOtherCB"；请求的句柄；信息名称（详见文件下方的附表3）。返回信息内容。
    FGetRequestOtherCB m_FGetRequestOtherCB;
    // 获取请求的链接句柄的回调函数指针类型。参数依次为：3；"FGetRequestConnCB"；请求的句柄。返回链接句柄。
    FGetRequestConnCB m_FGetRequestConnCB;

    // 设置应答头信息的回调函数指针类型。参数依次为：5；"FSetRespondHeadCB"；请求的句柄；头名称；内容（为空时，删除）。返回错误码：0无错误。
    FSetRespondHeadCB m_FSetRespondHeadCB;
    // 设置应答内容的回调函数指针类型。参数依次为：5；"FSetRespondBodyCB"；请求的句柄；内容的类型（json数据、二进制流等）；内容；内容的字节数。返回错误码：0无错误。
    FSetRespondBodyCB m_FSetRespondBodyCB;
    // 设置其他应答信息（200或500状态等）的回调函数指针类型。参数依次为：5；"FSetRespondOtherCB"；请求的句柄；信息名称（详见文件下方的附表4）；内容（为空时，删除）。返回错误码：0无错误。
    FSetRespondOtherCB m_FSetRespondOtherCB;
    // 设置提交应答的回调函数指针类型。参数依次为：3；"FSetCommitResponseCB"；请求的句柄。返回错误码：0无错误。
    FSetCommitResponseCB m_FSetCommitResponseCB;

    // 设置通过错误码判断是否掉线的回调函数指针类型。参数依次为：3；"FSetIsDisconnByErrCodeCB"；错误码。返回：true已掉线；false未掉线。
    FSetIsDisconnByErrCodeCB m_FSetIsDisconnByErrCodeCB;
    // 设置通过错误码得到错误信息的回调函数指针类型。参数依次为：3；"FSetGetStatusByErrCode"；错误码。返回错误信息。
    FSetGetStatusByErrCodeCB m_FSetGetStatusByErrCodeCB;

    ////////////////////////// 仅用于Server-Sent Events、Websocket、以及长连接的数据缓存 //////////////////////////
    // 设置链接为长连接的回调函数指针类型。参数依次为：4；"FSetKeepaliveCB"；链接的句柄；启用或关闭长连接。返回错误码：0无错误。
    FSetKeepaliveCB m_FSetKeepaliveCB;
    // 设置连接断开事件的回调函数。参数依次为：5；"FSetBreakConnCB"；链接的句柄；回调的实例句柄；回调函数。返回错误码：0无错误。
    FSetBreakConnCB m_FSetBreakConnCB;
    // 关闭连接的回调函数。参数依次为：3；"FCloseConnCB"；链接的句柄。返回错误码：0无错误。
    FCloseConnCB m_FCloseConnCB;
    // 得到是否关闭连接的回调函数。参数依次为：3；"FIsCloseConnCB"；链接的句柄。返回：true连接已关闭；false未关闭。
    FIsCloseConnCB m_FIsCloseConnCB;
    // 重置链接时间的回调函数。参数依次为：4；"FResetConnTimeCB"；请求的句柄；超时的时间（毫秒）。返回错误码：0无错误。
    FResetConnTimeCB m_FResetConnTimeCB;
    // 通过链接发送数据的回调函数。参数依次为：5；"FSendByConnCB"；请求的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
    FSendByConnCB m_FSendByConnCB;
    // 通过链接进行Socket发送数据的回调函数。参数依次为：5；"FSockSendByConnCB"；链接的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
    FSockSendByConnCB m_FSockSendByConnCB;
    // 通过链接进行websocket接收数据的回调函数。参数依次为：5；"FWscRecvByConnCB"；请求的句柄；接收缓冲器；缓存区最大字节数。返回实际接收数据的字节数。
    FWscRecvByConnCB m_FWscRecvByConnCB;
} g_javaCBPointer;

// 判断java返回的值，是否提示未知的关键字
static bool CheckKeyUnknown(const char* pVal)
{
    const char* pTip = "Unknown key:";
    char pTipResult[32] = { 0 };
    memcpy(pTipResult, pVal, strlen(pTip));
    return strcmp(pTip, pTipResult) == 0;
}

// 申请C++线程静态空间
static char* GetThreadStaticSpace(unsigned len)
{
    if (!KC_HasLoad()) return 0;
    typedef char* (*FGetThreadStaticSpace)(unsigned);
    return ((FGetThreadStaticSpace)KC_GetLibFunc("GetThreadStaticSpace"))(len + 1);
}
#define RETURNVAL(pVal) \
        if (0 != pVal) { \
            if (CheckKeyUnknown(pVal)) return 0; \
            unsigned iLen = (unsigned)strlen(pVal); \
            char* pBuf = GetThreadStaticSpace(iLen); \
            memcpy(pBuf, pVal, iLen); \
            return pBuf; \
        }

/////////////////////////////////////////////// 隔绝java与C++的内存 /////////////////////////////////////////////////
// 获取服务器配置信息的回调函数指针类型。参数依次为：3；"FGetSrvInfoCB"；信息名称（详见文件下方的附表1）。返回信息内容。
static const char* __FGetSrvInfoCB(int argCount, const char* flag, const char* name)
{
    printf("*[JavaMod] FGetSrvInfoCB: %s - %s\n", flag, name);
    if (strcmp(flag, "FGetSrvInfoCB") == 0 && 3 == argCount)
    {
        if (strcmp(name, "Name") == 0) return "jna";
        // else if (strcmp(name, "Version") == 0) return "java1.0";
        // else if (strcmp(name, "MainExeModRoot") == 0) return ".";
        // else if (strcmp(name, "KnewcodeRoot") == 0) return "..\\..\\..\\..\\website_test\\..\\..\\bin\\debug";
        // else if (strcmp(name, "DocumentRoot") == 0) return "..\\..\\..\\..\\website_test\\frontend\\dist";
        // else if (strcmp(name, "KnewcodeCfgFile") == 0) return "..\\..\\..\\..\\website_test\\\\my-prj.xml";
        // else if (strcmp(name, "KnewcodeRoot") == 0) return "..\\..\\knewcode_v12";
        // else if (strcmp(name, "DocumentRoot") == 0) return "..\\..\\website\\frontend\\dist";
        // else if (strcmp(name, "KnewcodeCfgFile") == 0) return "..\\..\\website\\\\my-prj.xml";
        else if (0 != g_javaCBPointer.m_FGetSrvInfoCB)
        {
            const char* pVal = g_javaCBPointer.m_FGetSrvInfoCB(argCount, "FGetSrvInfoCB", name);
            RETURNVAL(pVal);
        }
    }
    return 0;
}

// 获取链接信息的回调函数指针类型。参数依次为：4；"FGetConnInfoCB"；链接的句柄；信息名称（详见文件下方的附表2）。返回信息内容。
static const char* __FGetConnInfoCB(int argCount, const char* flag, intptr_t hConn, const char* name)
{
    printf("*[JavaMod] FGetConnInfoCB: %s - %s\n", flag, name);
    if (strcmp(flag, "FGetConnInfoCB") == 0 && 4 == argCount && 0 != g_javaCBPointer.m_FGetConnInfoCB)
    {
        const char* pVal = g_javaCBPointer.m_FGetConnInfoCB(argCount, "FGetConnInfoCB", hConn, name);
        RETURNVAL(pVal);
    }
    return 0;
}

// 获取请求头信息的回调函数指针类型。参数依次为：4；"FGetRequestHeadCB"；请求的句柄；信息名称。返回信息内容。
static const char* __FGetRequestHeadCB(int argCount, const char* flag, intptr_t hRequest, const char* name)
{
    printf("*[JavaMod] FGetRequestHeadCB: %s - %s\n", flag, name);
    if (strcmp(flag, "FGetRequestHeadCB") == 0 && 4 == argCount && 0 != g_javaCBPointer.m_FGetRequestHeadCB)
    {
        const char* pVal = g_javaCBPointer.m_FGetRequestHeadCB(argCount, "FGetRequestHeadCB", hRequest, name);
        RETURNVAL(pVal);
    }
    return 0;
}
// 获取请求内容（post参数、多表单数据、其他二进制流等）的回调函数指针类型。参数依次为：3；"FGetRequestBodyCB"；请求的句柄。返回内容。
// 内容的类型可通过头名称“Content-Type”获取，内容的长度可通过“Content-Length”获取。
static const char* __FGetRequestBodyCB(int argCount, const char* flag, intptr_t hRequest)
{
    const unsigned iLen = atoi(__FGetRequestHeadCB(4, "FGetRequestHeadCB", hRequest, "Content-Length"));
    printf("*[JavaMod] FGetRequestBodyCB: %s - %d\n", flag, iLen);
    if (iLen > 0 && strcmp(flag, "FGetRequestBodyCB") == 0 && 3 == argCount && 0 != g_javaCBPointer.m_FGetRequestBodyCB)
    {
        const char* pVal = g_javaCBPointer.m_FGetRequestBodyCB(argCount, "FGetRequestBodyCB", hRequest);
        if (0 != pVal)
        {
            char* pBuf = GetThreadStaticSpace(iLen);
            memcpy(pBuf, pVal, iLen);
            return pBuf;
        }
    }
    return "";
}
// 获取其他请求信息（get参数、本地文件等）的回调函数指针类型。参数依次为：4；"FGetRequestOtherCB"；请求的句柄；信息名称（详见文件下方的附表3）。返回信息内容。
static const char* __FGetRequestOtherCB(int argCount, const char* flag, intptr_t hRequest, const char* name)
{
    printf("*[JavaMod] FGetRequestOtherCB: %s - %s\n", flag, name);
    if (strcmp(flag, "FGetRequestOtherCB") == 0 && 4 == argCount && 0 != g_javaCBPointer.m_FGetRequestOtherCB)
    {
        const char* pVal = g_javaCBPointer.m_FGetRequestOtherCB(argCount, "FGetRequestOtherCB", hRequest, name);
        RETURNVAL(pVal);
    }
    return 0;
}
// 获取请求的链接句柄的回调函数指针类型。参数依次为：3；"FGetRequestConnCB"；请求的句柄。返回链接句柄。
static intptr_t __FGetRequestConnCB(int argCount, const char* flag, intptr_t hRequest)
{
    printf("*[JavaMod] FGetRequestConnCB: %s\n", flag);
    if (strcmp(flag, "FGetRequestConnCB") == 0 && 3 == argCount && 0 != g_javaCBPointer.m_FGetRequestConnCB)
        return g_javaCBPointer.m_FGetRequestConnCB(argCount, "FGetRequestConnCB", hRequest);
    return 0;
}

// 设置应答头信息的回调函数指针类型。参数依次为：5；"FSetRespondHeadCB"；请求的句柄；头名称；内容（为空时，删除）。返回错误码：0无错误。
static int __FSetRespondHeadCB(int argCount, const char* flag, intptr_t hRequest, const char* name, const char* val)
{
    printf("*[JavaMod] FSetRespondHeadCB: %s\n", flag);
    if (strcmp(flag, "FSetRespondHeadCB") == 0 && 5 == argCount && 0 != g_javaCBPointer.m_FSetRespondHeadCB)
        return g_javaCBPointer.m_FSetRespondHeadCB(argCount, "FSetRespondHeadCB", hRequest, name, val);
    return 0;
}
// 设置应答内容的回调函数指针类型。参数依次为：5；"FSetRespondBodyCB"；请求的句柄；内容的类型（json数据、二进制流等）；内容；内容的字节数。返回错误码：0无错误。
static int __FSetRespondBodyCB(int argCount, const char* flag, intptr_t hRequest, const char* content, int len)
{
    printf("*[JavaMod] FSetRespondBodyCB: %s\n", flag);
    if (strcmp(flag, "FSetRespondBodyCB") == 0 && 5 == argCount && 0 != g_javaCBPointer.m_FSetRespondBodyCB)
        return g_javaCBPointer.m_FSetRespondBodyCB(argCount, "FSetRespondBodyCB", hRequest, content, len);
    return 0;
}
// 设置其他应答信息（200或500状态等）的回调函数指针类型。参数依次为：5；"FSetRespondOtherCB"；请求的句柄；信息名称（详见文件下方的附表4）；内容（为空时，删除）。返回错误码：0无错误。
static int __FSetRespondOtherCB(int argCount, const char* flag, intptr_t hRequest, const char* name, const char* val)
{
    printf("*[JavaMod] FSetRespondOtherCB: %s\n", flag);
    if (strcmp(flag, "FSetRespondOtherCB") == 0 && 5 == argCount && 0 != g_javaCBPointer.m_FSetRespondOtherCB)
        return g_javaCBPointer.m_FSetRespondOtherCB(argCount, "FSetRespondOtherCB", hRequest, name, val);
    return 0;
}
// 设置提交应答的回调函数指针类型。参数依次为：3；"FSetCommitResponseCB"；请求的句柄。返回错误码：0无错误。
static int __FSetCommitResponseCB(int argCount, const char* flag, intptr_t hRequest)
{
    printf("*[JavaMod] FSetCommitResponseCB: %s\n", flag);
    if (strcmp(flag, "FSetCommitResponseCB") == 0 && 3 == argCount && 0 != g_javaCBPointer.m_FSetCommitResponseCB)
        return g_javaCBPointer.m_FSetCommitResponseCB(argCount, "FSetCommitResponseCB", hRequest);
    return 0;
}

// 设置通过错误码判断是否掉线的回调函数指针类型。参数依次为：3；"FSetIsDisconnByErrCodeCB"；错误码。返回：true已掉线；false未掉线。
static bool __FSetIsDisconnByErrCodeCB(int argCount, const char* flag, int errCode)
{
    printf("*[JavaMod] FSetIsDisconnByErrCodeCB: %s\n", flag);
    if (strcmp(flag, "FSetIsDisconnByErrCodeCB") == 0 && 3 == argCount && 0 != g_javaCBPointer.m_FSetIsDisconnByErrCodeCB)
        return g_javaCBPointer.m_FSetIsDisconnByErrCodeCB(argCount, "FSetIsDisconnByErrCodeCB", errCode);
    return false;
}
// 设置通过错误码得到错误信息的回调函数指针类型。参数依次为：3；"FSetGetStatusByErrCodeCB"；错误码。返回错误信息。
static const char* __FSetGetStatusByErrCodeCB(int argCount, const char* flag, int errCode)
{
    printf("*[JavaMod] FSetGetStatusByErrCode: %s\n", flag);
    if (strcmp(flag, "FSetGetStatusByErrCode") == 0 && 3 == argCount && 0 != g_javaCBPointer.m_FSetGetStatusByErrCodeCB)
    {
        const char* pVal = g_javaCBPointer.m_FSetGetStatusByErrCodeCB(argCount, "FSetGetStatusByErrCode", errCode);
        RETURNVAL(pVal);
    }
    return "";
}

// 设置链接为长连接的回调函数指针类型。参数依次为：4；"FSetKeepaliveCB"；链接的句柄；启用或关闭长连接。返回错误码：0无错误。
static int __FSetKeepaliveCB(int argCount, const char* flag, intptr_t hConn, bool enable)
{
    printf("*[JavaMod] FSetKeepaliveCB: %s\n", flag);
    if (strcmp(flag, "FSetKeepaliveCB") == 0 && 4 == argCount && 0 != g_javaCBPointer.m_FSetKeepaliveCB)
        return g_javaCBPointer.m_FSetKeepaliveCB(argCount, "FSetKeepaliveCB", hConn, enable);
    return 0;
}
// 设置断开的回调函数的函数指针参数（供调用方回调）
typedef bool (*FBreakConnCBFromOwn)(intptr_t hInstance);
// 设置连接断开事件的回调函数。参数依次为：5；"FSetBreakConnCB"；链接的句柄；回调的实例句柄；回调函数。返回错误码：0无错误。
static int __FSetBreakConnCB(int argCount, const char* flag, intptr_t hConn, intptr_t hInstance, FBreakConnCBFromOwn fcb)
{
    printf("*[JavaMod] FSetBreakConnCB: %s\n", flag);
    if (strcmp(flag, "FSetBreakConnCB") == 0 && 5 == argCount && 0 != g_javaCBPointer.m_FSetBreakConnCB)
        return g_javaCBPointer.m_FSetBreakConnCB(argCount, "FSetBreakConnCB", hConn, hInstance, fcb);
    return 0;
}
// 关闭连接的回调函数。参数依次为：3；"FCloseConnCB"；链接的句柄。返回错误码：0无错误。
static int __FCloseConnCB(int argCount, const char* flag, intptr_t hConn)
{
    printf("*[JavaMod] FCloseConnCB: %s\n", flag);
    if (strcmp(flag, "FCloseConnCB") == 0 && 3 == argCount && 0 != g_javaCBPointer.m_FCloseConnCB)
        return g_javaCBPointer.m_FCloseConnCB(argCount, "FCloseConnCB", hConn);
    return 0;
}
// 得到是否关闭连接的回调函数。参数依次为：3；"FIsCloseConnCB"；链接的句柄。返回：true连接已关闭；false未关闭。
static bool __FIsCloseConnCB(int argCount, const char* flag, intptr_t hConn)
{
    printf("*[JavaMod] FIsCloseConnCB: %s\n", flag);
    if (strcmp(flag, "FIsCloseConnCB") == 0 && 3 == argCount && 0 != g_javaCBPointer.m_FIsCloseConnCB)
        return g_javaCBPointer.m_FIsCloseConnCB(argCount, "FIsCloseConnCB", hConn);
    return false;
}
// 重置链接时间的回调函数。参数依次为：4；"FResetConnTimeCB"；请求的句柄；超时的时间（毫秒）。返回错误码：0无错误。
static int __FResetConnTimeCB(int argCount, const char* flag, intptr_t hRequest, unsigned ms)
{
    printf("*[JavaMod] FResetConnTimeCB: %s\n", flag);
    if (strcmp(flag, "FResetConnTimeCB") == 0 && 4 == argCount && 0 != g_javaCBPointer.m_FResetConnTimeCB)
        return g_javaCBPointer.m_FResetConnTimeCB(argCount, "FResetConnTimeCB", hRequest, ms);
    return 0;
}
// 通过链接发送数据的回调函数。参数依次为：5；"FSendByConnCB"；请求的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
static int __FSendByConnCB(int argCount, const char* flag, intptr_t hRequest, const char* buf, unsigned len)
{
    printf("*[JavaMod] FSendByConnCB: %s\n", flag);
    if (strcmp(flag, "FSendByConnCB") == 0 && 5 == argCount && 0 != g_javaCBPointer.m_FSendByConnCB)
        return g_javaCBPointer.m_FSendByConnCB(argCount, "FSendByConnCB", hRequest, buf, len);
    return 0;
}
// 通过链接进行Socket发送数据的回调函数。参数依次为：5；"FSockSendByConnCB"；链接的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
static int __FSockSendByConnCB(int argCount, const char* flag, intptr_t hConn, const char* buf, unsigned len)
{
    printf("*[JavaMod] FSockSendByConnCB: %s\n", flag);
    if (strcmp(flag, "FSockSendByConnCB") == 0 && 5 == argCount && 0 != g_javaCBPointer.m_FSockSendByConnCB)
        return g_javaCBPointer.m_FSockSendByConnCB(argCount, "FSockSendByConnCB", hConn, buf, len);
    return 0;
}
// 通过链接进行websocket接收数据的回调函数。参数依次为：5；"FWscRecvByConnCB"；请求的句柄；接收缓冲器；缓存区最大字节数。返回实际接收数据的字节数。
static int __FWscRecvByConnCB(int argCount, const char* flag, intptr_t hRequest, char* buf, unsigned len)
{
    printf("*[JavaMod] FWscRecvByConnCB: %s\n", flag);
    if (strcmp(flag, "FWscRecvByConnCB") == 0 && 5 == argCount && 0 != g_javaCBPointer.m_FWscRecvByConnCB)
        return g_javaCBPointer.m_FWscRecvByConnCB(argCount, "FWscRecvByConnCB", hRequest, buf, len);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 设置回调函数指针
#define SETCBFUNCPOINTER(argCount1, argCount2, flag, func, funcName) \
        if (strcmp(flag, #funcName) == 0 && argCount1 == argCount2) { \
            g_javaCBPointer.m_##funcName = func;  \
            if (!KC_HasLoad()) return -1; \
            typedef int (*FSetCBFuncPointer)(int, const char*, void*); \
            FSetCBFuncPointer fSetCBFuncPointer = (FSetCBFuncPointer)KC_GetLibFunc("SetCBFuncPointer"); \
            return fSetCBFuncPointer(argCount1, #funcName, &__##funcName); \
        }
int __SetCBFuncPointer(int argCount, const char* flag, void* func)
{
    printf("*[JavaMod] SetCBFuncPointer: %s %d %#0x\n", flag, argCount, (unsigned)(intptr_t)func);
    SETCBFUNCPOINTER(3, argCount, flag, func, FGetSrvInfoCB);
    SETCBFUNCPOINTER(4, argCount, flag, func, FGetConnInfoCB);
    SETCBFUNCPOINTER(4, argCount, flag, func, FGetRequestHeadCB);
    SETCBFUNCPOINTER(3, argCount, flag, func, FGetRequestBodyCB);
    SETCBFUNCPOINTER(4, argCount, flag, func, FGetRequestOtherCB);
    SETCBFUNCPOINTER(3, argCount, flag, func, FGetRequestConnCB);
    SETCBFUNCPOINTER(5, argCount, flag, func, FSetRespondHeadCB);
    SETCBFUNCPOINTER(5, argCount, flag, func, FSetRespondBodyCB);
    SETCBFUNCPOINTER(5, argCount, flag, func, FSetRespondOtherCB);
    SETCBFUNCPOINTER(3, argCount, flag, func, FSetCommitResponseCB);
    SETCBFUNCPOINTER(3, argCount, flag, func, FSetIsDisconnByErrCodeCB);
    SETCBFUNCPOINTER(3, argCount, flag, func, FSetGetStatusByErrCodeCB);
    SETCBFUNCPOINTER(4, argCount, flag, func, FSetKeepaliveCB);
    SETCBFUNCPOINTER(5, argCount, flag, func, FSetBreakConnCB);
    SETCBFUNCPOINTER(3, argCount, flag, func, FCloseConnCB);
    SETCBFUNCPOINTER(3, argCount, flag, func, FIsCloseConnCB);
    SETCBFUNCPOINTER(4, argCount, flag, func, FResetConnTimeCB);
    SETCBFUNCPOINTER(5, argCount, flag, func, FSendByConnCB);
    SETCBFUNCPOINTER(5, argCount, flag, func, FSockSendByConnCB);
    SETCBFUNCPOINTER(5, argCount, flag, func, FWscRecvByConnCB);
    return -1;
}
