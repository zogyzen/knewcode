#pragma once

#include "for_user/common_define.h"

namespace KC
{
    // 基础请求和应答接口
    class IBaseRequestRespond
    {
    public:
        ////////////////////////////请求部分///////////////////////////
        // 得到服务器端全部信息
        virtual const char* CALL_TYPE GetAllInfo(const char* = "\n") = 0;
        // 得到本地完整文件名
        virtual const char* CALL_TYPE GetLocalFilename(void) = 0;
        // 得到网络文件名
        virtual const char* CALL_TYPE GetUriFilename(void) = 0;
        // 协议
        virtual const char* CALL_TYPE GetProtocol(void) = 0;
        // 主机名
        virtual const char* CALL_TYPE GetHostName(void) = 0;
        // 端口
        virtual int CALL_TYPE GetPort(void) = 0;
        // 客户端信息
        virtual const char* CALL_TYPE GetUserAgent(void) = 0;
        // 客户端IP
        virtual const char* CALL_TYPE GetClientIP(void) = 0;
        // 接收的信息
        virtual const char* CALL_TYPE GetAccept(const char* = "") = 0;
        // GET的参数
        virtual const char* CALL_TYPE GetGetArgStr(void) = 0;
        // Post的参数
        virtual const char* CALL_TYPE GetPostArgType(void) = 0;
        virtual int CALL_TYPE GetPostArgLength(void) = 0;
        virtual int CALL_TYPE GetPostArgBuffer(char*, int) = 0;
        // 得到网站本地完整根目录
        virtual const char* CALL_TYPE GetLocalRootPath(void) = 0;
        // 按名称得到请求头
        virtual const char* CALL_TYPE GetRequestHeader(const char*) = 0;
        // 得到Cookie
        virtual const char* CALL_TYPE GetCookieStr(void) = 0;

        ////////////////////////////应答部分///////////////////////////
        // 响应文本类型
        virtual bool CALL_TYPE SetResponseContentType(const char*) = 0;
        virtual const char* CALL_TYPE GetResponseContentType(void) = 0;
        // 输出网页内容：参数为输出内容
        virtual bool CALL_TYPE AddResponseBody(const char*, int = 0) = 0;
        // 按名称添加响应头
        virtual void CALL_TYPE AddResponseHeader(const char*, const char*) = 0;

    protected:
        virtual CALL_TYPE ~IBaseRequestRespond() = default;
    };

    // 通用请求和应答接口
    class IActionData;
    class IKCRequestRespond : public IBaseRequestRespond
    {
    public:
        // 请求过程中的数据
        struct IReStepData
        {
            virtual CALL_TYPE ~IReStepData() = default;
            // 得到名称
            virtual const char* CALL_TYPE GetName(void) = 0;
        };

    public:
        // 信号处理实例接口
        virtual ISignoCatch& CALL_TYPE SignoCatch(void) = 0;
        // 锁
        virtual IKcLockWork& CALL_TYPE LockWork(void) = 0;
        // 超时时间
        virtual int CALL_TYPE GetTimeOutSeconds(void) const = 0;
        // 执行中的页面数据接口
        virtual IActionData* CALL_TYPE GetActionData(void) = 0;
        virtual void CALL_TYPE SetActionData(IActionData* = nullptr) = 0;

        ////////////////////////////请求部分///////////////////////////
        // 得到当前页本地完整根目录
        virtual const char* CALL_TYPE GetLocalPagePath(void) = 0;

        ////////////////////////////应答部分///////////////////////////
        // 添加Cookie；参数依次为【名称、值、有效期、Url路径、子域】。有效期格式，如，2014-10-01 13:00:05
        virtual bool CALL_TYPE AddCookie(const char*, const char*, const char* = "", const char* = "/", const char* = "") = 0;
        // 输出网页内容：参数为输出内容
        virtual bool CALL_TYPE OutputData(const char*, const char*, int = 0) = 0;
        virtual bool CALL_TYPE OutputHTML(const char*) = 0;

        ////////////////////////请求过程中的数据///////////////////////
        // 判断某名称数据是否存在
        virtual bool CALL_TYPE ExistsReStepData(const char*) = 0;
        // 得到某名称数据
        virtual IReStepData& CALL_TYPE GetReStepData(const char*) = 0;
        // 添加某名称数据（添加过程数据接口指针，并由保存者负责删除）
        virtual bool CALL_TYPE AddReStepData(const char*, IReStepData*&) = 0;

    protected:
        virtual CALL_TYPE ~IKCRequestRespond() = default;
    };

    // 执行中的页面数据接口
    class IActionData
    {
    public:
        // 得到页面请求和应答
        virtual IKCRequestRespond& CALL_TYPE GetRequestRespond(void) = 0;
        // 得到归属
        virtual IActionData* CALL_TYPE GetActOwn(void) = 0;
        virtual IActionData& CALL_TYPE GetActRoot(void) = 0;
        virtual IActionData& CALL_TYPE GetActNow(void) = 0;
        // 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogDebug(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogInfo(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const = 0;
        // 解析并执行字符串脚本
        virtual bool CALL_TYPE ParseActionScript(const char*, const char*) = 0;
        // 创建、释放简单数据或接口
        virtual IKcWebDataVal& CALL_TYPE CreateDataVal(void) = 0;
        virtual bool CALL_TYPE ReleaseDataVal(IKcWebDataVal&) = 0;
        virtual IKcWebInfVal& CALL_TYPE CreateInfVal(void) = 0;
        virtual bool CALL_TYPE ReleaseInfVal(IKcWebInfVal&) = 0;
        // 标识符、内部变量是否存在
        virtual bool CALL_TYPE ExistsVar(const char*) const = 0;
        virtual bool CALL_TYPE ExistsInnerVar(const char*, const char*) const = 0;
        // 接口变量
        virtual IKcWebInfVal& CALL_TYPE GetInterfaceVar(const char*) = 0;
        virtual bool CALL_TYPE SetInterfaceVar(const char*, IKcWebInfVal&) = 0;
        // 简单类型变量值
        virtual IKcWebDataVal& CALL_TYPE GetVarValue(const char*) = 0;
        virtual bool CALL_TYPE SetVarValue(const char*, IKcWebDataVal&) = 0;
        // 内部变量值
        virtual IKcWebInfVal& CALL_TYPE GetInnerVarInf(const char*, const char*) = 0;
        virtual IKcWebDataVal& CALL_TYPE GetInnerVarVal(const char*, const char*) = 0;
        virtual bool CALL_TYPE SetInnerVar(const char*, const char*, IKcWebInfVal&) = 0;
        virtual bool CALL_TYPE SetInnerVar(const char*, const char*, IKcWebDataVal&) = 0;
        // 垃圾回收
        virtual void CALL_TYPE GC(void) = 0;
        // 获取本地化提示信息
        virtual const char* CALL_TYPE getHint(const char* key) const = 0;
        // 抛一个字符串异常
        virtual void CALL_TYPE ThrowString(const char* ex) const = 0;

    protected:
        virtual CALL_TYPE ~IActionData() = default;
    };
}
