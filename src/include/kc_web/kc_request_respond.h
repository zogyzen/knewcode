#pragma once

#include "for_user/kc_controller_i.h"

namespace KC
{
    // 基础请求和应答接口
    class IBaseRequestRespond : public IKCObject
    {
    public:
        ////////////////////////////请求部分///////////////////////////
        // 得到服务器端全部信息
        virtual const char* CALL_TYPE GetAllInfo(const char* = "\n") = 0;
        // 得到服务器端各单个请求信息
        virtual const char* CALL_TYPE GetSingleInfo(const char* = "the_request", const char* = "") = 0;
        // 得到本地完整文件名
        virtual const char* CALL_TYPE GetLocalFilename(void) = 0;
        virtual const char* CALL_TYPE GetLocalFilename(const char*) = 0;
        // 得到网络文件名
        virtual const char* CALL_TYPE GetUriFilename(void) = 0;
        // 得到网络路径
        virtual const char* CALL_TYPE GetUrlPagePath(void) = 0;
        // 得到网络根路径
        virtual const char* CALL_TYPE GetUrlPageRootPath(void) = 0;
        virtual const char* CALL_TYPE GetUrlPageRootPath(const char*) = 0;
        // 端口
        virtual int CALL_TYPE GetPort(void) = 0;
        // 获取GET参数
        virtual const char* CALL_TYPE GetGetArgStr(void) = 0;
        // 获取Post参数
        virtual const char* CALL_TYPE GetPostArgType(void) = 0;
        virtual int CALL_TYPE GetPostArgLength(void) = 0;
        virtual int CALL_TYPE GetPostArgBuffer(char*&, int = -1) = 0;
        virtual const char* CALL_TYPE GetPostArgStr(void) = 0;
        // 得到网站本地完整根目录
        virtual const char* CALL_TYPE GetLocalRootPath(void) = 0;
        // 按名称得到请求头
        virtual const char* CALL_TYPE GetRequestHeader(const char*) = 0;
        // 虚拟目录
        virtual unsigned CALL_TYPE VirtualPathCount(void) = 0;
        virtual const char* CALL_TYPE GetVirtualPath(unsigned) = 0;
        virtual const char* CALL_TYPE GetVirtualPathUri(unsigned) = 0;

        ////////////////////////////应答部分///////////////////////////
        // 响应文本类型
        virtual bool CALL_TYPE SetResponseContentType(const char*) = 0;
        virtual const char* CALL_TYPE GetResponseContentType(void) = 0;
        // 响应状态
        virtual bool CALL_TYPE SetResponseStatus(int = 200) = 0;
        // 按名称添加、删除响应头
        virtual void CALL_TYPE AddResponseHeader(const char*, const char*) = 0;
        virtual void CALL_TYPE DelResponseHeader(const char*) = 0;
        // 输出网页内容：参数为输出内容
        virtual bool CALL_TYPE AddResponseBody(const char*, int = 0) = 0;
        // 提交响应（将这段时间，服务器端的输出，同时推给客户端）
        virtual void CALL_TYPE CommitResponse(void) = 0;

    protected:
        virtual ~IBaseRequestRespond() = default;
    };

    // Web长链接
    class IKCWebLongConn : public IKCObject
    {
    public:
        // 链接状态
        enum EWebConnStatus
        {
            ecsNormal = 0,      // 正常
            ecsDisconn,         // 断线
            ecsError            // 错误
        };

        // 接收回调
        class IRecvCB : public IKCObject
        {
        public:
            // 接收信息
            virtual void CALL_TYPE Recv(char*, unsigned) = 0;
            // 断开链接
            virtual void CALL_TYPE Discon(void) = 0;
            // 错误日志
            virtual bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const = 0;
            // 其他指令
            virtual void CALL_TYPE Cmd(const char*, const char*) = 0;

        protected:
            virtual ~IRecvCB(void) = default;
        };

    public:
        // 链接编号
        virtual long CALL_TYPE GetID(void) = 0;
        virtual const char* CALL_TYPE GetUniqueID(void) = 0;
        // 发消息
        virtual EWebConnStatus CALL_TYPE Send(const char*, unsigned) = 0;
        // 设置接收消息回调接口
        virtual void CALL_TYPE SetRecvIF(IRecvCB* = nullptr) = 0;
        // 请求应答上下文
        virtual IBaseRequestRespond& CALL_TYPE MakeRe(void) = 0;
        virtual void CALL_TYPE ReleaseRe(IBaseRequestRespond&) = 0;
        // 启动
        virtual void CALL_TYPE Start(void) = 0;

    protected:
        virtual ~IKCWebLongConn(void) = default;
    };

    // 多表单数据里的单项数据，对应web的“multipart/form-data”
    struct IMultiFormData : public IKCObject
    {
        virtual ~IMultiFormData() = default;
        // 得到头
        virtual const char* CALL_TYPE GetHeader(const char*) = 0;
        // 得到体
        virtual const char* CALL_TYPE GetBody(unsigned&) = 0;
        // 是否文件
        virtual bool CALL_TYPE isFile(void) = 0;
    };

    // 通用请求和应答接口
    class IKCRequestRespond : public IBaseRequestRespond
    {
    public:
        // 请求过程中的数据
        struct IReStepData : public IKCObject
        {
            virtual ~IReStepData() = default;
            // 得到名称
            virtual const char* CALL_TYPE GetName(void) = 0;
        };

    public:
        // 锁
        virtual IKcLockWork& CALL_TYPE LockWork(void) = 0;
        // 超时时间
        virtual int CALL_TYPE GetTimeOutSeconds(void) const = 0;
        // 本次执行中的整体活动数据接口
        virtual IActionData* CALL_TYPE GetActionData(void) = 0;
        virtual void CALL_TYPE SetActionData(IActionData* = nullptr) = 0;
        virtual IActionData& CALL_TYPE ActionData(void) = 0;

        ////////////////////////////请求部分///////////////////////////
        // 规范化url路径（去除“..”和“.”）
        virtual const char* CALL_TYPE CanonicalUrl(const char*) = 0;
        // 得到当前页本地完整根目录
        virtual const char* CALL_TYPE GetLocalPagePath(void) = 0;
        virtual const char* CALL_TYPE GetLocalPath(const char*) = 0;
        // 协议
        virtual const char* CALL_TYPE GetProtocol(void) = 0;
        // 获取GET参数
        virtual const char* CALL_TYPE GetGetArg(const char*, const char* = "") = 0;
        // 多表单数据，multipart/form-data参数
        virtual bool CALL_TYPE IsMultipartFormData(void) = 0;
        virtual unsigned CALL_TYPE GetMultiFormDataCount(void) = 0;
        virtual IMultiFormData& CALL_TYPE GetMultiFormData(void) = 0;
        virtual void CALL_TYPE SetCurrentMultiFormData(unsigned) = 0;
        // 得到Cookie值
        virtual const char* CALL_TYPE GetCookieVal(const char*) = 0;

        ////////////////////////////应答部分///////////////////////////
        // 添加Cookie；参数依次为【名称、值、有效期、Url路径、子域】。有效期格式，如，2014-10-01 13:00:05
        virtual bool CALL_TYPE AddCookie(const char*, const char*, const char* = "", const char* = "/", const char* = "") = 0;

        ////////////////////////请求过程中的数据///////////////////////
        // 判断某名称数据是否存在
        virtual bool CALL_TYPE ExistsReStepData(const char*) = 0;
        // 得到某名称数据
        virtual IReStepData& CALL_TYPE GetReStepData(const char*) = 0;
        // 添加某名称数据（添加过程数据接口指针，并由保存者负责删除）
        virtual bool CALL_TYPE AddReStepData(const char*, IReStepData*&) = 0;

    protected:
        virtual ~IKCRequestRespond() = default;
    };

    // 执行中Api控制器的数据接口
    class ICtrlApiDataX : public ICtrlApiData
    {
    public:
        // 当前请求上下文信息（todo: 准备去掉）
        virtual IKCRequestRespond& CALL_TYPE GetRequestRespond(void) const = 0;
    };
}
