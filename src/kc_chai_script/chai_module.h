#pragma once

#include "std.h"

class CCtrlChai;

// 脚本中根环境背景
class KcContext : public IChaiContext
{
public:
    KcContext(CCtrlChai& own);

    // 得到环境上下文
    IKCContext& getContext(void) const override;

    // 加密数据库连接
    string EncryptDBConnStr(string str);
    string DecryptDBConnStr(string str);

private:
    // 宿主
    CCtrlChai &m_own;
};

// json类
class KcJson
{
public:
    KcJson(IKCJson& kcJsn);
    KcJson(const IKCJson& kcJsn);

    // 获取值
    int GetInt(string sName) const;
    long long GetInt64(string sName) const;
    double GetNum(string sName) const;
    string GetStr(string sName) const;
    bool IsNull(string sName) const;
    bool HasItem(string sName) const;

    // 设置值
    void SetVal(string sName, int val);
    void SetVal(string sName, long long val);
    void SetVal(string sName, double val);
    void SetVal(string sName, string val);
    void AddArray(string sName);

private:
    IKCJson& m_kcJson;
};

// 全局值
class KcGlobalVal
{
public:
    KcGlobalVal(IActionData& act);

    // 获取值
    int GetInt(string sName) const;
    long long GetInt64(string sName) const;
    double GetNum(string sName) const;
    string GetStr(string sName) const;

    // 设置值
    void SetVal(string sName, int val);
    void SetVal(string sName, long long val);
    void SetVal(string sName, double val);
    void SetVal(string sName, string val);

private:
    IActionData& m_act;
};

// 当前活动控制器类
class KcActCtrl : public IChaiKcActCtrl
{
public:
    KcActCtrl(ICtrlApiData& ctrlD, KcContext& cont);

    // 得到脚本环境上下文
    KcContext& Context(void) const;
    IChaiContext& getContext(void) const override;

    // 得到当前控制器Api数据接口
    ICtrlApiData& getCtrlApiData(void) const override;

    // 操作json
    const KcJson& Request(void);
    KcJson& Json(void);
    const KcJson& JsonPrev(void);
    const KcJson& Attach(void);

    // 操作全局值
    KcGlobalVal& GlobalVal(void);

    // 执行其他api
    int ExecUrlApi(string sUrl);

private:
    ICtrlApiData& m_objCtrlD;
    KcContext& m_context;
    // json
    const KcJson m_jsonRequest;
    KcJson m_jsonRespond;
    const KcJson m_jsonRespondLast;
    const KcJson m_jsonAttach;
    // 全局值
    KcGlobalVal m_globalVal;
};

// 基本模块
class ChaiBaseModule
{
public:
    ChaiBaseModule(CCtrlChai&, property_tree::ptree&);
    ~ChaiBaseModule(void);

    // 初始化基础模块
    void InitBaseMod(void);
    // 初始化基础C++类
    void InitBaseCPPClass(void);
    // 初始化附加库
    static void InitAdditions(ChaiBaseModule&, const property_tree::ptree&, chaiscript::ModulePtr, ICtrlApiData *ctrlD = nullptr);
    static void InitAdditionOne(ChaiBaseModule&, string sName, const property_tree::ptree&, chaiscript::ModulePtr, ICtrlApiData *ctrlD = nullptr);

private:
    // 宿主
    CCtrlChai &m_own;
    // 脚本模块的配置
    property_tree::ptree& m_pt;
    // 基础模块
    chaiscript::ModulePtr m_baseMod;
    // 背景环境上下文
    KcContext m_context;
    // 附加模块库
    typedef TSubModule<IChaiModAddition> TAdditionModLib;
    typedef std::shared_ptr<TAdditionModLib> TAdditionModLibPtr;
    map<string, TAdditionModLibPtr> m_exModList;

    friend class CCtrlChai;
};
