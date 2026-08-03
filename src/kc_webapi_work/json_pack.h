#pragma once

#include "std.h"
#include "json_cjson.h"
#include "request_respond_parm.h"

// json容器
class CKCJsonPack : public IKCJson
{
public:
    CKCJsonPack(const CKCJsonPack&) = delete;       // 删除默认的拷贝构造
    // CKCJsonPack(CKCJsonPack&&) = default;        // 默认的移动构造
    ~CKCJsonPack(void) override;
    CKCJsonPack& operator=(const CKCJsonPack&) = delete;

    // key名字
    const char* CALL_TYPE GetName(void) const override;
    // 数值类型
    IKCJson::EDataType CALL_TYPE GetType(void) const override;
    // 父项
    const IKCJson& Own(void) const override;
    bool IsSub(void) const override;
    /// 子项
    // 读
    bool CALL_TYPE HasItem(const char* name, bool caseSensitive) const override;
    bool CALL_TYPE HasItem(const char*) const override;
    IKCJson& CALL_TYPE GetItem(const char*, bool caseSensitive) override;
    const IKCJson& CALL_TYPE GetItem(const char*, bool caseSensitive) const override;
    IKCJson& CALL_TYPE GetItem(const char*) override;
    const IKCJson& CALL_TYPE GetItem(const char*) const override;
    IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit, bool caseSensitive) override;
    const IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit, bool caseSensitive) const override;
    IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit) override;
    const IKCJson& CALL_TYPE GetItem(const char*, const char* sNameSplit) const override;
    void CALL_TYPE InitItemNextStep(void) const override;
    const IKCJson& CALL_TYPE GetItemNext(void) const override;
    IKCJson& CALL_TYPE GetItemNext(void) override;
    // 读数组
    unsigned CALL_TYPE GetItemCount(void) const override;
    IKCJson& CALL_TYPE GetItem(unsigned) override;
    const IKCJson& CALL_TYPE GetItem(unsigned) const override;
    // 读值
    double CALL_TYPE GetVal(const char* name, double def, bool caseSensitive) const override;
    double CALL_TYPE GetVal(const char*, double) const override;
    const char* CALL_TYPE GetStr(const char* name, const char* def, bool caseSensitive, const char* charset) const override;
    const char* CALL_TYPE GetStr(const char*, const char*) const override;
    bool CALL_TYPE IsNull(const char* name, bool caseSensitive) const override;
    bool CALL_TYPE IsNull(const char*) const override;
    // 写
    IKCJson& CALL_TYPE AddItem(const char*, bool repeat, bool caseSensitive) override;
    IKCJson& CALL_TYPE AddItem(const char*, bool repeat = true) override;
    IKCJson& CALL_TYPE AddArray(const char*, bool repeat, bool caseSensitive) override;
    IKCJson& CALL_TYPE AddArray(const char*, bool repeat = true) override;
    IKCJson& CALL_TYPE AddJson(const char*, const char*, bool repeat, bool caseSensitive) override;
    IKCJson& CALL_TYPE AddJson(const char*, const char*, bool repeat = true) override;
    bool CALL_TYPE AddJson(const char* name, IKCJson& sub, bool repeat, bool caseSensitive) override;
    bool CALL_TYPE AddJson(const char*, IKCJson&, bool repeat = true) override;
    bool CALL_TYPE SetJson(const char* name, const char* json, bool caseSensitive) override;
    bool CALL_TYPE SetJson(const char*, const char*) override;
    bool CALL_TYPE SetJson(const char* name, IKCJson& sub, bool caseSensitive) override;
    bool CALL_TYPE SetJson(const char*, IKCJson&) override;
    bool CALL_TYPE DelItem(const char* name, bool caseSensitive) override;
    bool CALL_TYPE DelItem(const char*) override;
    // 写数组
    bool CALL_TYPE DelItem(unsigned id) override;
    IKCJson& CALL_TYPE AddItem(unsigned, bool bReplace = false) override;
    IKCJson& CALL_TYPE AddArray(unsigned, bool bReplace = false) override;
    IKCJson& CALL_TYPE AddJson(unsigned, const char*, bool bReplace = false) override;
    bool CALL_TYPE SetJson(unsigned, IKCJson&) override;
    bool CALL_TYPE SetNull(unsigned) override;
    bool CALL_TYPE SetVal(unsigned, double) override;
    bool CALL_TYPE SetStr(unsigned, const char*) override;
    // 写值
    bool CALL_TYPE AddNull(const char* name) override;
    bool CALL_TYPE AddVal(const char* name, double val) override;
    bool CALL_TYPE AddStr(const char* name, const char* str, const char* charset) override;
    bool CALL_TYPE AddStr(const char*, const char*) override;
    bool CALL_TYPE SetNull(const char* name, bool caseSensitive) override;
    bool CALL_TYPE SetNull(const char*) override;
    bool CALL_TYPE SetVal(const char* name, double val, bool caseSensitive) override;
    bool CALL_TYPE SetVal(const char*, double) override;
    bool CALL_TYPE SetStr(const char* name, const char* str, bool caseSensitive, const char* charset) override;
    bool CALL_TYPE SetStr(const char*, const char*) override;
    /// 自身
    // 读取
    double CALL_TYPE GetVal(double def = 0) const override;
    const char* CALL_TYPE GetStrSelf(const char* def = "", const char* charset = c_RESTful_UTF8) const override;
    const char* CALL_TYPE GetStr(const char* = "") const override;
    bool CALL_TYPE IsNull(void) const override;
    // 写入
    bool CALL_TYPE SetVal(double val) override;
    bool CALL_TYPE SetStrSelf(const char* str, const char* charset) override;
    bool CALL_TYPE SetStr(const char*) override;
    // 判断
    bool CALL_TYPE EqualTo(const IKCJson& rv, bool caseSensitive) const override;
    bool CALL_TYPE EqualTo(const IKCJson&) const override;
    // 用其他根项重置（只针对根）
    bool CALL_TYPE ResetByRoot(const IKCJson&) override;
    // 用字符串重置（只针对根）
    bool CALL_TYPE ResetByStr(const char* = "{}") override;
    // 内存回收
    bool CALL_TYPE GC(void) const override;
    void GC(const char*) const override;
    // 生成字符串（fmt：0有无格式，由日志等级决定；1强制无格式；2强制有格式）
    const char* CALL_TYPE ToStr(int fmt = 0) const override;
    // 源最终的错误码和错误信息
    int CALL_TYPE GetErrCode(void) const override;
    const char* CALL_TYPE GetErrMsg(void) const override;
    // 其他非json参数
    IKCCtrlParmInOut& ParmInOut(void) override { return Core().Parm(); }
    const IKCCtrlParmInOut& ParmInOut(void) const override { return Core().Parm(); }

public:
    // 内存回收
    void GCImpl(void) const;

    // 功能实现
    typedef std::shared_ptr<CKCJsonPack> CKCJsonPackPtr;
    CKCJsonPackPtr ImplGetItem(unsigned);
    CKCJsonPackPtr ImplGetItem(const char*, bool caseSensitive);
    CKCJsonPackPtr ImplGetItemNext(void);
    CKCJsonPackPtr ImplAddItem(const char*, bool repeat, bool caseSensitive);
    CKCJsonPackPtr ImplAddJson(const char*, const char*, bool repeat, bool caseSensitive);
    CKCJsonPackPtr ImplAddArray(const char*, bool repeat, bool caseSensitive);

    // 核心的参数和json
    virtual ICoreParmJson& Core(void) = 0;
    virtual const ICoreParmJson& Core(void) const = 0;

protected:
    // 构造
    CKCJsonPack(IJsonCallBack&);

protected:
    IJsonCallBack &m_cb;

    // 临时子项
    mutable multimap<string, CKCJsonPackPtr> m_mapSubItm_obj;
    mutable map<int, CKCJsonPackPtr> m_mapSubItm_inArr;
    // 当前步进子项
    mutable CKCJsonPackPtr m_stepSub;

    friend class CKCJsonPackSub;
};

// 无效项
class CKCJsonPackInvalid : public CKCJsonPack
{
    friend class CKCJsonPack;

public:
    CKCJsonPackInvalid(IJsonCallBack& cb)  : CKCJsonPack(cb) {}

    bool IsValid(void) const override { return false; }

    // 无效的参数和json
    ICoreParmJson& Core(void) override { throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name()); }
    const ICoreParmJson& Core(void) const override { throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name()); }
};

// 根项
class CKCJsonPackRoot : public CKCJsonPack
{
    friend class CKCJsonPack;

public:
    CKCJsonPackRoot(IJsonCallBack&);

    // 核心的参数和json类
    class TCoreParmJsonRoot : public ICoreParmJson
    {
        friend class CKCJsonPackAttach;
        friend class CKCJsonPackRequest;

    public:
        TCoreParmJsonRoot(string jsonLib, string json, string nameErrCode = c_RESTful_errCode, string nameErrMsg = c_RESTful_errMsg, string charset = c_RESTful_UTF8, bool jsonCaseSensitive = false);
        TCoreParmJsonRoot(IKCJsonCore::IJsonCorePtr, string nameErrCode = c_RESTful_errCode, string nameErrMsg = c_RESTful_errMsg, string charset = c_RESTful_UTF8, bool jsonCaseSensitive = false);
        ~TCoreParmJsonRoot(void);

        // 输入输出参数
        IKCCtrlParmInOut& Parm(void) override { return m_parm; }
        const IKCCtrlParmInOut& Parm(void) const override { return m_parm; }

        // Json
        IKCJsonCore& Json(void) override { return *m_json; }
        const IKCJsonCore& Json(void) const override { return *m_json; }

        // 按照配置选择的json库，通过json字符串生成json
        static IKCJsonCore* NewJsonCore(string jsonLib, string json);

    protected:
        // Json
        IKCJsonCore::IJsonCorePtr m_json;

    public:
        // 输入输出参数
        CKCCtrlParmInOut &m_parm;
    };
    typedef std::shared_ptr<TCoreParmJsonRoot> TCoreParmJsonRootPtr;

    // 生成核心的参数和json类（用于应答）
    static TCoreParmJsonRoot* NewCore(IJsonCallBack& cb, string json = "")
    {
        return new TCoreParmJsonRoot(cb.JsonLibrary(), json, cb.GetFixParmName(c_RESTful_errCode), cb.GetFixParmName(c_RESTful_errMsg), cb.GetCharset(), cb.JsonCaseSensitive());
    }
    static TCoreParmJsonRoot* NewCore(IJsonCallBack& cb, IKCJsonCore::IJsonCorePtr json)
    {
        return new TCoreParmJsonRoot(json, cb.GetFixParmName(c_RESTful_errCode), cb.GetFixParmName(c_RESTful_errMsg), cb.GetCharset(), cb.JsonCaseSensitive());
    }
};
// 附加参数
class CKCJsonPackAttach : public CKCJsonPackRoot
{
public:
    CKCJsonPackAttach(IJsonCallBack&, string json);

    // 是否有效
    bool CALL_TYPE IsValid(void) const override { return m_core.m_json.get() != nullptr && m_core.m_json->IsValid(); }

    ICoreParmJson& Core(void) override { return m_core; }
    const ICoreParmJson& Core(void) const override { return m_core; }

protected:
    TCoreParmJsonRoot m_core;
};

// 请求
class CKCJsonPackRequest : public CKCJsonPackRoot
{
public:
    CKCJsonPackRequest(IJsonCallBack&, string json);
    CKCJsonPackRequest(IJsonCallBack&, const CKCJsonPackRequest&);

    // 是否有效
    bool CALL_TYPE IsValid(void) const override { return m_core.get() != nullptr && m_core->m_json.get() != nullptr && m_core->m_json->IsValid(); }

    // 核心的参数和json
    ICoreParmJson& Core(void) override { return *m_core; }
    const ICoreParmJson& Core(void) const override { return *m_core; }

    // 拷贝出来（用于拷贝到其他的控制器实例里）
    TCoreParmJsonRootPtr CloneOut(void) const { return m_core; }

protected:
    TCoreParmJsonRootPtr m_core;
};

// 应答
class CKCJsonPackRespond : public CKCJsonPackRoot
{
public:
    // 是否有效
    bool CALL_TYPE IsValid(void) const override { return m_core.get() != nullptr && m_core->m_json.get() != nullptr && m_core->m_json->IsValid(); }

    // 用其他根项重置（只针对根）
    bool CALL_TYPE ResetByRoot(const IKCJson&) override;
    // 用字符串重置
    bool CALL_TYPE ResetByStr(const char* = "{}") override;

    // 源最终的错误码和错误信息
    // int CALL_TYPE GetErrCode(void) const override { return this->IsValid() ? static_cast<int>(GetVal(m_core->m_parm.m_nameErrCode.c_str(), 0)) : -70; }
    int CALL_TYPE GetErrCode(void) const override { return this->IsValid() ? static_cast<int>(GetVal(m_core->m_parm.m_nameErrCode.c_str(), 0)) : 0; }
    const char* CALL_TYPE GetErrMsg(void) const override { return this->IsValid() ? GetStr(m_core->m_parm.m_nameErrMsg.c_str(), "") : "NaN"; }

    // 核心的参数和json类（用于应答）
    class TCoreParmJsonRespond : public TCoreParmJsonRoot
    {
        friend class CKCJsonPackRespond;

    public:
        using TCoreParmJsonRoot::TCoreParmJsonRoot;

        // 用字符串重置
        bool ResetByStr(string jsonLib, string json);

        // 保存执行的最终结果
        void SaveErrInfo(void)
        {
            m_errCode = m_parm.GetErrCode();
            m_errMsg = CUtilFunc::PCharSafeToStr(m_parm.GetErrMsg());
        }
        int GetErrCode(void) const
        {
            return m_errCode;
        }
        const char* GetErrMsg(void) const
        {
            return m_errMsg.c_str();
        }

    protected:
        // int m_errCode = -70;
        int m_errCode = 0;
        string m_errMsg = "NaN";
    };
    typedef std::shared_ptr<TCoreParmJsonRespond> TCoreParmJsonRespondPtr;
    typedef std::weak_ptr<TCoreParmJsonRespond> TCoreParmJsonRespondWeakPtr;

    // 生成核心的参数和json类（用于应答）
    static TCoreParmJsonRespond* NewCore(IJsonCallBack& cb, string json = "")
    {
        return new TCoreParmJsonRespond(cb.JsonLibrary(), json, cb.GetFixParmName(c_RESTful_errCode), cb.GetFixParmName(c_RESTful_errMsg), cb.GetCharset(), cb.JsonCaseSensitive());
    }
    static TCoreParmJsonRespond* NewCore(IJsonCallBack& cb, IKCJsonCore::IJsonCorePtr json)
    {
        return new TCoreParmJsonRespond(json, cb.GetFixParmName(c_RESTful_errCode), cb.GetFixParmName(c_RESTful_errMsg), cb.GetCharset(), cb.JsonCaseSensitive());
    }

    ICoreParmJson& Core(void) override { return *m_core; }
    const ICoreParmJson& Core(void) const override { return *m_core; }
    TCoreParmJsonRespond& Kernel(void) { return *m_core; }
    const TCoreParmJsonRespond& Kernel(void) const { return *m_core; }

    // 移入移出（用于在控制器实例里转移核心参数和Json）
    TCoreParmJsonRespondPtr MoveOut(void);
    bool MoveIn(TCoreParmJsonRespondPtr);
    TCoreParmJsonRespondPtr Copy(void) const { return m_core; }
    TCoreParmJsonRespondWeakPtr Ref(void) const { return TCoreParmJsonRespondWeakPtr(m_core); }

    // 只输出一个参数
    bool OutParmSubItem(string sPath, string sSplit = "\\.");

    // 构造函数
    CKCJsonPackRespond(IJsonCallBack&, string json = "");
    CKCJsonPackRespond(IJsonCallBack&, TCoreParmJsonRespondPtr);
    CKCJsonPackRespond(IJsonCallBack&, IKCJsonCore::IJsonCorePtr);

protected:
    // 核心的参数和json类（用于应答）
    TCoreParmJsonRespondPtr m_core;

    friend class CKCJsonPackRespondRef;
};
typedef std::shared_ptr<CKCJsonPackRespond> CKCJsonPackRespondPtr;
// 应答的引用
class CKCJsonPackRespondRef
{
public:
    CKCJsonPackRespondRef(void) = default;
    CKCJsonPackRespondRef(CKCJsonPackRespond& ref) : m_ref(&ref), m_core(ref.m_core) {}

    // 是否有效
    bool CALL_TYPE IsValid(void) const
    {
        return nullptr != m_ref && !m_core.expired() && m_ref->IsValid();
    }

    // 应答
    const CKCJsonPackRespond& JsonPackRespond(void) const
    {
        if (nullptr == m_ref || m_core.expired()) throw std::runtime_error("JsonPackRespond Expired");
        return *m_ref;
    }

protected:
    CKCJsonPackRespond *m_ref = nullptr;
    // 核心的参数和json类的引用
    CKCJsonPackRespond::TCoreParmJsonRespondWeakPtr m_core;
};

// 子项
class CKCJsonPackSub : public CKCJsonPack
{
    friend class CKCJsonPack;

public:
    // 是否有效
    bool CALL_TYPE IsValid(void) const override { return m_core.m_json.get() != nullptr && m_core.m_json->IsValid(); }

    // 父项
    const IKCJson& Own(void) const override { return m_own; }
    bool IsSub(void) const override { return true; }

public:
    // 核心的参数和json
    class TCoreParmJsonSub : public ICoreParmJson
    {
        friend class CKCJsonPackSub;

    public:
        TCoreParmJsonSub(IKCJsonCore::IJsonCorePtr json, const IKCCtrlParmInOut& parm);
        ~TCoreParmJsonSub(void);

        // 输入输出参数
        IKCCtrlParmInOut& Parm(void) override { throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFuncAccessDenied, __CURR_CODE_PLACE_C__, "Access Denied", typeid(*this).name()); }
        const IKCCtrlParmInOut& Parm(void) const override { return m_parm; }

        // Json
        IKCJsonCore& Json(void) override { return *m_json; }
        const IKCJsonCore& Json(void) const override { return *m_json; }

    protected:
        // Json
        IKCJsonCore::IJsonCorePtr m_json;
        // 根参数
        const IKCCtrlParmInOut &m_parm;
    };
    TCoreParmJsonSub m_core;
    ICoreParmJson& Core(void) override { return m_core; }
    const ICoreParmJson& Core(void) const override { return m_core; }

    // 子项提升为根项
    CKCJsonPackRespondPtr PromoteToRoot(bool delFromOwn = false) const;
    CKCJsonPackRespond* PromoteToRootNeedDel(bool delFromOwn = false) const;

protected:
    // 构造
    CKCJsonPackSub(IJsonCallBack&, IKCJsonCore::IJsonCorePtr, const CKCJsonPack&);

protected:
    const CKCJsonPack &m_own;
};
