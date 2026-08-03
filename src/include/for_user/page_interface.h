#pragma once

#include "for_user/kc_context_i.h"

namespace KC
{
    // 请求和应答参数
    class IKCCtrlParmInOut : public IKCObject
    {
    public:
        /// 写入
        // 设置参数类型为json
        virtual void CALL_TYPE SetIsJson(void) = 0;
        // 正文类型
        virtual void CALL_TYPE SetContentType(const char* = "application/json") = 0;
        // 头部数据（针对web请求的应答）
        virtual void CALL_TYPE SetHeader(const char*, const char*) = 0;
        // 正文数据
        virtual void CALL_TYPE SetContent(const char*, const unsigned) = 0;
        /// 读取
        // 参数类型是否json
        virtual bool CALL_TYPE IsJson(void) const = 0;
        // 正文类型
        virtual const char* CALL_TYPE GetContentType(void) const = 0;
        // 头部数据（针对web请求的应答）
        virtual const char* CALL_TYPE GetHeader(const char*, const char*) const = 0;
        // 正文数据
        virtual const char* CALL_TYPE GetContent(void) const = 0;
        virtual unsigned CALL_TYPE GetContentLength(void) const = 0;
        // 是否区分大小写
        virtual bool CALL_TYPE JsonCaseSensitive(void) const = 0;
        // 字符集
        virtual const char* CALL_TYPE GetCharset(void) const = 0;
        // 源输出最终的错误码和错误信息
        virtual int CALL_TYPE GetErrCode(void) const = 0;
        virtual const char* CALL_TYPE GetErrMsg(void) const = 0;
    };

    // json
    class IKCJson : public IKCObject
    {
    public:
        enum EDataType { edtNull = -1, edtString = 0, edtNumber, edtArray, edtObject };

    public:
        // 是否有效
        virtual bool CALL_TYPE IsValid(void) const = 0;
        // key名字
        virtual const char* CALL_TYPE GetName(void) const = 0;
        // 数值类型
        virtual EDataType CALL_TYPE GetType(void) const = 0;
        // 父项
        virtual const IKCJson& Own(void) const = 0;
        virtual bool IsSub(void) const = 0;
        /// 子项
        // 读
        virtual bool CALL_TYPE HasItem(const char*, bool caseSensitive) const = 0;
        virtual bool CALL_TYPE HasItem(const char*) const = 0;
        virtual const IKCJson& CALL_TYPE GetItem(const char*, bool caseSensitive) const = 0;
        virtual IKCJson& CALL_TYPE GetItem(const char*, bool caseSensitive) = 0;
        virtual const IKCJson& CALL_TYPE GetItem(const char*) const = 0;
        virtual IKCJson& CALL_TYPE GetItem(const char*) = 0;
        virtual const IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit, bool caseSensitive) const = 0;
        virtual IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit, bool caseSensitive) = 0;
        virtual const IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit) const = 0;
        virtual IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit) = 0;
        virtual void CALL_TYPE InitItemNextStep(void) const = 0;
        virtual const IKCJson& CALL_TYPE GetItemNext(void) const = 0;
        virtual IKCJson& CALL_TYPE GetItemNext(void) = 0;
        // 读数组
        virtual unsigned CALL_TYPE GetItemCount(void) const = 0;
        virtual const IKCJson& CALL_TYPE GetItem(unsigned) const = 0;
        virtual IKCJson& CALL_TYPE GetItem(unsigned) = 0;
        // 读值
        virtual double CALL_TYPE GetVal(const char*, double, bool caseSensitive) const = 0;
        virtual double CALL_TYPE GetVal(const char*, double) const = 0;
        virtual const char* CALL_TYPE GetStr(const char*, const char*, bool caseSensitive, const char* charset) const = 0;
        virtual const char* CALL_TYPE GetStr(const char*, const char*) const = 0;
        virtual bool CALL_TYPE IsNull(const char*, bool caseSensitive) const = 0;
        virtual bool CALL_TYPE IsNull(const char*) const = 0;
        // 写
        virtual IKCJson& CALL_TYPE AddItem(const char*, bool repeat, bool caseSensitive) = 0;
        virtual IKCJson& CALL_TYPE AddItem(const char*, bool repeat = true) = 0;
        virtual IKCJson& CALL_TYPE AddArray(const char*, bool repeat, bool caseSensitive) = 0;
        virtual IKCJson& CALL_TYPE AddArray(const char*, bool repeat = true) = 0;
        virtual IKCJson& CALL_TYPE AddJson(const char*, const char*, bool repeat, bool caseSensitive) = 0;
        virtual IKCJson& CALL_TYPE AddJson(const char*, const char*, bool repeat = true) = 0;
        virtual bool CALL_TYPE AddJson(const char*, IKCJson&, bool repeat, bool caseSensitive) = 0;
        virtual bool CALL_TYPE AddJson(const char*, IKCJson&, bool repeat = true) = 0;
        virtual bool CALL_TYPE SetJson(const char*, const char*, bool caseSensitive) = 0;
        virtual bool CALL_TYPE SetJson(const char*, const char*) = 0;
        virtual bool CALL_TYPE SetJson(const char*, IKCJson&, bool caseSensitive) = 0;
        virtual bool CALL_TYPE SetJson(const char*, IKCJson&) = 0;
        virtual bool CALL_TYPE DelItem(const char*, bool caseSensitive) = 0;
        virtual bool CALL_TYPE DelItem(const char*) = 0;
        // 写数组
        virtual bool CALL_TYPE DelItem(unsigned) = 0;
        virtual IKCJson& CALL_TYPE AddItem(unsigned, bool bReplace = false) = 0;
        virtual IKCJson& CALL_TYPE AddArray(unsigned, bool bReplace = false) = 0;
        virtual IKCJson& CALL_TYPE AddJson(unsigned, const char*, bool bReplace = false) = 0;
        virtual bool CALL_TYPE SetJson(unsigned, IKCJson&) = 0;
        virtual bool CALL_TYPE SetNull(unsigned) = 0;
        virtual bool CALL_TYPE SetVal(unsigned, double) = 0;
        virtual bool CALL_TYPE SetStr(unsigned, const char*) = 0;
        // 写值
        virtual bool CALL_TYPE AddNull(const char*) = 0;
        virtual bool CALL_TYPE AddVal(const char*, double) = 0;
        virtual bool CALL_TYPE AddStr(const char*, const char*, const char* charset) = 0;
        virtual bool CALL_TYPE AddStr(const char*, const char*) = 0;
        virtual bool CALL_TYPE SetNull(const char*, bool caseSensitive) = 0;
        virtual bool CALL_TYPE SetNull(const char*) = 0;
        virtual bool CALL_TYPE SetVal(const char*, double, bool caseSensitive) = 0;
        virtual bool CALL_TYPE SetVal(const char*, double) = 0;
        virtual bool CALL_TYPE SetStr(const char*, const char*, bool caseSensitive, const char* charset) = 0;
        virtual bool CALL_TYPE SetStr(const char*, const char*) = 0;
        /// 自身
        // 读
        virtual double CALL_TYPE GetVal(double = 0) const = 0;
        virtual const char* CALL_TYPE GetStrSelf(const char*, const char* charset) const = 0;
        virtual const char* CALL_TYPE GetStr(const char* = "") const = 0;
        virtual bool CALL_TYPE IsNull(void) const = 0;
        // 写
        virtual bool CALL_TYPE SetVal(double) = 0;
        virtual bool CALL_TYPE SetStrSelf(const char*, const char* charset) = 0;
        virtual bool CALL_TYPE SetStr(const char*) = 0;
        // 判断
        virtual bool CALL_TYPE EqualTo(const IKCJson&, bool caseSensitive) const = 0;
        virtual bool CALL_TYPE EqualTo(const IKCJson&) const = 0;
        // 用其他根项重置（只针对根）
        virtual bool CALL_TYPE ResetByRoot(const IKCJson&) = 0;
        // 用字符串重置（只针对根）
        virtual bool CALL_TYPE ResetByStr(const char* = "{}") = 0;
        // 内存回收
        virtual bool CALL_TYPE GC(void) const = 0;
        virtual void GC(const char*) const = 0;
        // 生成字符串（fmt：0有无格式，由日志等级决定[调试等级下有格式]；1强制无格式；2强制有格式）
        virtual const char* CALL_TYPE ToStr(int fmt = 1) const = 0;
        // 源最终的错误码和错误信息
        virtual int CALL_TYPE GetErrCode(void) const = 0;
        virtual const char* CALL_TYPE GetErrMsg(void) const = 0;
        // 其他非json参数
        virtual IKCCtrlParmInOut& ParmInOut(void) = 0;
        virtual const IKCCtrlParmInOut& ParmInOut(void) const = 0;
    };

    // 本次执行中的整体活动数据接口
    class IActionData : public IKCObject
    {
    public:
        // 本次请求的活动对象
        struct TActObj : public IKCObject
        {
            virtual void Release(void) = 0;
        };
        // 持久链接的活动对象
        struct TAliveObj : public IKCObject
        {
            virtual ~TAliveObj(void) = default;
            virtual const char* AliveID(void) = 0;
            // 活动对象
            virtual void AddActObj(const char* sName, TActObj*) = 0;
            virtual void DelActObj(const char* sName) = 0;
            virtual TActObj* GetActObj(const char* sName) = 0;
        };

    public:
        // 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogDebug(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogInfo(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogWarning(const char*, const char* = "", const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const = 0;
        virtual bool CALL_TYPE WriteLogFatal(const char*, const char*, const char* = "") const = 0;
        // 垃圾回收
        virtual void CALL_TYPE GC(void) = 0;
        // 获取本地化提示信息
        virtual const char* CALL_TYPE GetHint(const char* key) const = 0;
        // 抛一个c++异常
        virtual void CALL_TYPE Throw(const char* ex, const char* place) const = 0;
        // 全局标识符（变量、常量）
        virtual bool CALL_TYPE IsGlobalVal(const char*) const = 0;
        virtual const char* CALL_TYPE GetGlobalVal(const char*, const char* = "") = 0;
        virtual void CALL_TYPE SetGlobalVal(const char*, const char*, unsigned = 0) = 0;
        // 设置全局标识符有效期限
        virtual void CALL_TYPE SetSessionExpire(const char*, const char* = "1d") = 0;
        // 全局标识符类型
        enum EGlobalValTpe { egtIsnot = 0, egtGloConst, egtConst, egtGetParm, egtSession, egtAction, egtJson };
        virtual EGlobalValTpe CALL_TYPE GetGlobalValType(const char*) const = 0;
        // Client编号
        virtual const char* CALL_TYPE GetKCCLNID(void) = 0;
        // Session编号
        virtual const char* CALL_TYPE GetKCSSID(void) = 0;
        // 用户输入参数的json
        virtual const IKCJson& CALL_TYPE JsonRequest(void) const = 0;
        // 执行结果的json
        virtual IKCJson& CALL_TYPE JsonRespond(void) = 0;
        // 无效json
        virtual IKCJson& CALL_TYPE InvalidJson(void) = 0;
        // 自定义json
        virtual IKCJson& CALL_TYPE MakeJson(const char* = "{}") = 0;
        virtual void CALL_TYPE FreeJson(IKCJson&) = 0;
        // 框架环境
        virtual IKCContext& CALL_TYPE Context(void) = 0;
        // 本次请求的活动对象
        virtual void CALL_TYPE AddActObj(const char* sName, TActObj*) = 0;
        virtual void CALL_TYPE DelActObj(const char* sName) = 0;
        virtual TActObj* CALL_TYPE GetActObj(const char* sName) = 0;
        // 持久链接的活动对象
        virtual TAliveObj& CALL_TYPE GetAliveObj(void) = 0;
        // 默认的返回数据集的名称
        virtual const char* CALL_TYPE GetValsName(void) const = 0;
        // 得到本次执行的时间戳标识
        virtual const char* CALL_TYPE GetNowTimeFlag(void) = 0;
        // 各单个请求信息
        virtual const char* CALL_TYPE GetSingleInfo(const char*, const char* = "") const = 0;
        // 得到当前页本地完整根目录
        virtual const char* CALL_TYPE GetLocalFullPath(const char*) = 0;
        // 得到url对应的本地路径
        virtual const char* CALL_TYPE GetUrlLocalPath(const char*) = 0;
        // 得到网页根路径
        virtual const char* CALL_TYPE GetUrlPageRootPath(const char*) = 0;
        // 虚拟目录
        virtual unsigned CALL_TYPE VirtualPathCount(void) = 0;
        virtual const char* CALL_TYPE GetVirtualPath(unsigned) = 0;
        virtual const char* CALL_TYPE GetVirtualPathUri(unsigned) = 0;

    protected:
        virtual ~IActionData() = default;
    };

    // 执行中Api控制器的数据接口
    class ICtrlApiData : public IKCObject
    {
    public:
        // 控制器标识名
        virtual const char* CALL_TYPE SignName(void) const = 0;
        // url路径（KC文件）
        virtual const char* CALL_TYPE Url(void) const = 0;
        // 本地文件（KC文件）
        virtual const char* CALL_TYPE LocalFile(void) const = 0;
        // 子项路径转换到本地完整路径
        virtual const char* CALL_TYPE transItemUrlToFullPath(const char*) const = 0;
        // 活动控制器名称（控制器xml节点名）
        virtual const char* CALL_TYPE ActName(void) const = 0;
        // 控制器xml全节点名
        virtual const char* CALL_TYPE NodeName(void) const = 0;
        // 插件类型（服务、动态库、组等）
        virtual const char* CALL_TYPE Type(void) const = 0;
        // 插件对应服务名（服务类型）
        virtual const char* CALL_TYPE Srv(void) const = 0;
        // 插件对应动态库文件（动态库类型）
        virtual const char* CALL_TYPE SoFile(void) const = 0;
        // 对应服务子控制器名称
        virtual const char* CALL_TYPE CtrlName(void) const = 0;
        // 控制器方法类型
        virtual const char* CALL_TYPE Method(void) const = 0;
        // 控制器正文
        virtual const char* CALL_TYPE Content(void) const = 0;
        // 是否根调用
        virtual bool CALL_TYPE IsRootRun(void) const = 0;
        // 是否区分大小写
        virtual bool CALL_TYPE JsonCaseSensitive(void) const = 0;
        // 字符集
        virtual const char* CALL_TYPE GetCharset(void) const = 0;
        // 获取GET参数
        virtual const char* CALL_TYPE GetGetArg(const char*, const char* = "") const = 0;
        // 获取POST参数
        virtual const char* CALL_TYPE GetPostStr(void) const = 0;
        // 得到其他子节点信息
        virtual const char* CALL_TYPE GetSubInfo(const char*, const char* = "", const char* = "") const = 0;
        // 得到其他信息
        virtual const char* CALL_TYPE GetOtherInfo(const char*, const char* = "") const = 0;

    public:
        // 控制器返回结果json
        virtual IKCJson& CALL_TYPE JsonRespond(void) = 0;
        // 控制器返回结果json
        virtual void CALL_TYPE SetJsonRespond(int errNo, const char* errMsg) = 0;
        // 上一个控制器返回结果json
        virtual const IKCJson& CALL_TYPE JsonRespondLast(void) const = 0;
        // 控制器附加参数的json
        virtual const IKCJson& CALL_TYPE JsonAttach(void) const = 0;
        virtual const IKCJson& CALL_TYPE JsonAttachByKcFile(void) const = 0;
        // 输入参数的json
        virtual const IKCJson& CALL_TYPE JsonRequest(void) const = 0;
        // 获取参数默认值json
        virtual const IKCJson& CALL_TYPE GetDefParmJson(const char*) const = 0;
        // 获取用户输入参数值json
        virtual const IKCJson& CALL_TYPE GetUserParmJson(const char*) const = 0;
        // 获取参数值json（依次查找上一个控制器的输出、用户输入的参数、默认的参数）
        virtual const IKCJson& CALL_TYPE GetParmJson(const char* = "") const = 0;
        // 获取参数值（依次查找全局数据、上一个控制器的输出、用户输入的参数、默认的参数）
        virtual const char* CALL_TYPE GetParmVal(const char*) const = 0;
        virtual const char* CALL_TYPE GetParmVal(const char*, const char*) const = 0;
        // 获取批量操作的数组参数json
        virtual const IKCJson& CALL_TYPE GetBatchArrayJson(const char* = "") const = 0;

    public:
        // 本次执行中的整体活动数据接口
        virtual IActionData& CALL_TYPE ActionData(void) const = 0;

        // 执行控制器
        virtual int WorkAct(const char*, const char*) = 0;
        virtual void ExecBundle(const char*, const char*, const char*, const char* = "", const char* = "") = 0;
        virtual void ExecBundleSub(const char*, const char*, const char*, const char* = "", const char* = "") = 0;
        virtual void ExecSo(const char*, const char*, const char*, const char* = "", const char* = "") = 0;
        virtual void ExecSoSub(const char*, const char*, const char*, const char* = "", const char* = "") = 0;
    };
}
