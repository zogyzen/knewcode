#pragma once

#include "std.h"
#include "json_pack.h"

class CActionData;
class CCtrApilData;
class CRootCtrlApiData;
class CSubCtrlApiData;
class CKCWebApiWork : public IKCWebApiWork
{
public:
    CKCWebApiWork(const IBundle& bundle);
    ~CKCWebApiWork() override;

    // 得到服务特征码
    const char* CALL_TYPE getGUID(void) const override;
    // 对应的模块
    const IBundle& CALL_TYPE getBundle(void) const override;

    // 初始化系统
    void CALL_TYPE InitSys(void) override;
    void CALL_TYPE FreeSys(void) override;

    // 获取uri的本地地址
    const char* CALL_TYPE GetApiUrlLocalDir(const char*) override;
    // 得到本地完整文件名
    const char* CALL_TYPE GetUrlLocalPath(const char*) override;
    // 得到网络根路径
    const char* CALL_TYPE GetUrlPageRootPath(const char*) override;

    // 执行webapi请求：参数是请求和应答接口
    int CALL_TYPE RenderREST(IKCRequestRespondData& re) override;

public:
    // 初始化/释放
    bool init(void);
    bool free(void);

    // 开始/停止
    bool start(void)
    {
        return true;
    }
    bool stop(void)
    {
        return true;
    }

    // 垃圾回收
    void GC(void);
    // session接口
    IKCSessionCookie& Session(void);
    // *** 执行控制器（通用入口） ***
    void ExecCtrlApi(CCtrApilData&);
    // 可供其他第三方模块调用
    CKCJsonPackRespond::TCoreParmJsonRespondPtr SubCallCtrlApi(CCtrApilData&, string, string, string, bool = true);
    CKCJsonPackRespond::TCoreParmJsonRespondPtr SubCallResult(CCtrApilData&, CCtrApilData&, string = "", bool upResult = true);
    // 持久对象
    IActionData::TAliveObj& GetAliveObj(string);
    // session控制器（一般需要全局控制器），用于存储内部数据
    string GetInnerSessionCtrl(void) { return m_innerSessionCtrl; }
    // 执行插件
    void ExecBundleImpl(CCtrApilData&);
    // 执行动态库
    void ExecSoImpl(CCtrApilData&);

    // 功能分支，类似全局参数里的“sys_flag”配置。
    string GrpBranchCtrl() const { return m_GrpBranchCtrl; }
    // 默认json名称区分大小写
    bool JsonCaseSensitive() const { return m_jsonCaseSensitive; }
    // 字符集
    string Charset() const { return m_charset; }
    // json库类型
    string JsonLibrary() const { return m_jsonLibrary; }
    // 登录账号的session。需要验证是否登录
    string NeedToken() const { return m_NeedToken; }

    // 默认的返回数据集的名称
    const char* DefaultValsName(void) const { return m_defaultValsName.c_str(); }

    // 服务控制器别名
    string SrvCtrlAliasDefault(void) { return m_srvCtrlAliasDefault; }
    struct TSrvCtrlAlias
    {
        string m_srv, m_ctrl;
        TSrvCtrlAlias(string srv, string ctrl) : m_srv(srv), m_ctrl(ctrl) {}
    };
    const map<string, TSrvCtrlAlias>& SrvCtrlAliasLs(void) { return m_srvCtrlAliasLs; }

    // 文件部分内容的起止符号
    const char* FilePartFlagB(void) const { return m_FilePartFlagB.c_str(); }
    const char* FilePartFlagE(void) const { return m_FilePartFlagE.c_str(); }

protected:
    // 执行请求
    CKCJsonPackRespond::TCoreParmJsonRespondPtr Work(CActionData&, std::function<void(CRootCtrlApiData&)> fcbNewCtrl);
    // 检查是否需要登录
    void CheckToken(CRootCtrlApiData&, const property_tree::ptree&, string, string);
    // json是否区分大小写
    bool JsonCaseSensitive(const property_tree::ptree&, string, bool = true);
    // 执行插件
    void ExecBundle(CCtrApilData&);
    // 执行动态库
    void ExecSo(CCtrApilData&);
    // 执行组操作
    void ExecGroup(CCtrApilData&, string, string = "");
    bool ExecGroupSub(CCtrApilData&, property_tree::ptree::value_type, string);
    // 守护线程（定时垃圾回收等）
    void GuardThrd(void);
    // 执行定时任务
    void ExecScheduledTask(void);

protected:
    // 动态库
    struct TSoCtrl
    {
        CKCWebApiWork& m_apiWk;
        dll::shared_library m_lib;
        IKCController *m_ctrl = nullptr;

        TSoCtrl(CKCWebApiWork& wk) : m_apiWk(wk) {}
        ~TSoCtrl(void) { m_apiWk.SoUninitActor(m_lib, m_ctrl); }
    };
    map<string, std::shared_ptr<TSoCtrl>> m_dlls;
    friend struct TSoCtrl;

    // 动态库的控制器
    IKCController* SoInitActor(dll::shared_library&, string, string);
    void SoUninitActor(dll::shared_library&, IKCController*&);

protected:
    // 本次持久链接的全局对象
    struct TKCAliveObj final : public IActionData::TAliveObj
    {
        CKCWebApiWork& m_own;
        // 持久链接编号
        string m_aliveID;
        // 持久对象
        map<string, IActionData::TActObj*> m_actObj;
        boost::shared_mutex m_mtx;
        // 终止判断
        posix_time::ptime m_ptime = posix_time::microsec_clock::local_time();

        TKCAliveObj(CKCWebApiWork& own, string id);
        ~TKCAliveObj(void) override;
        const char* AliveID(void) override;
        // 活动对象
        void AddActObj(const char* sName, IActionData::TActObj*) override;
        void DelActObj(const char* sName) override;
        IActionData::TActObj* GetActObj(const char* sName) override;
    };
    friend struct TKCAliveObj;
    list<boost::shared_ptr<TKCAliveObj>> m_aliveObjList;
    map<string, boost::weak_ptr<TKCAliveObj>> m_aliveObjMap;
    boost::shared_mutex m_mtxList, m_mtxMap;
    TKCAliveObj* GetAliveObjNotCreate(string);

protected:
    // 插件及上下文
    IBundleContext& m_context;
    const IBundle& m_bundle;
    // 引用
    map<string, IServiceReference*> m_refs;
    // sessoin引用
    IServiceReference &m_sessionRef;
    // 本模块的配置项
    const string m_cfgMod;
    // 本模块控制器部分配置项
    const string m_cfgModCtrl;
    // 登录账号的session。需要验证是否登录
    const string m_NeedToken;
    // session控制器（一般需要全局控制器），用于存储内部数据
    const string m_innerSessionCtrl;
    // 功能分支，类似全局参数里的“sys_flag”配置。
    const string m_GrpBranchCtrl = "KC";
    // 默认json名称区分大小写
    const bool m_jsonCaseSensitive = false;
    // 回传SessionID和ClientID
    const bool m_backSessionID = false;
    // 默认字符集
    const string m_charset = c_RESTful_UTF8;
    // json解析库
    const string m_jsonLibrary = c_RESTful_cJSON;
    // 持久连接的超时设置
    const unsigned m_keepAliveTimeout = 600;
    // 递归最大层数
    const unsigned m_recursionLayerMax = 128;
    // 预备执行和结束处理控制器
    const string m_initAct, m_checkTokenAct, m_prepAct, m_postpAct;
    // 授权执行webapi的uri路径
    struct TApiUri
    {
        string m_uri, m_localDir;
        bool m_grant = true;
        TApiUri(string uri, string dir, bool grant) : m_uri(uri), m_localDir(dir), m_grant(grant) {}

        operator string(void) const
        {
            return m_localDir;
        }
    };
    const map<string, TApiUri> m_apiUriLs;
    // 默认的返回数据集的名称
    const string m_defaultValsName = c_RESTful_vals;
    // 服务控制器别名
    const string m_srvCtrlAliasDefault;
    const map<string, TSrvCtrlAlias> m_srvCtrlAliasLs;
    // 文件部分的配置项和起止标记
    const string m_filePartFlag,
        m_FilePartFlagB = "------part::%s{{",
        m_FilePartFlagE = "------}}part::~%s";
    // 定时任务的处理控制器
    const string m_taskTimeExec, m_TaskExecManaFlag = "KC__SESSION_global__taskManaFlag", m_TaskExecGet, m_TaskExecFinish;
    const std::atomic_uint m_TaskExecInterval;
    // 守护线程
    std::atomic_bool m_guardThrdEnd;
    boost::thread m_GuardThrd;
    // 配置中的响应头信息
    std::map<string, string> m_CfgHeader;
};
