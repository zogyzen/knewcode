#pragma once

#include "std.h"
#include "json_pack.h"
#include "kc_webapi_work.h"

class CActionData;
// 主控制器API执行的数据
class CCtrApilDataBase : public CSqlFunc::ICtrlApiDataWithDbSet, public IJsonCallBack
{
    friend class CRootCtrlApiData;
    friend class CSubCtrlApiData;

public:
    virtual ~CCtrApilDataBase() = default;

    // 当前请求上下文信息
    IKCRequestRespond& CALL_TYPE GetRequestRespond(void) const override;
    // 本次执行中的整体活动数据接口
    IActionData& CALL_TYPE ActionData(void) const override;

    // 控制器标识名
    const char* CALL_TYPE SignName(void) const override;
    // 控制器的链接
    const char* Url(void) const override;
    // 本地文件（KC文件）
    const char* CALL_TYPE LocalFile(void) const override;
    // 子项路径转换到本地完整路径
    const char* CALL_TYPE transItemUrlToFullPath(const char*) const override;
    // 活动控制器名称（控制器xml节点名）
    const char* CALL_TYPE ActName(void) const override;
    // 控制器xml全节点名
    const char* CALL_TYPE NodeName(void) const override;
    // 插件类型（服务、动态库、组等）
    const char* CALL_TYPE Type(void) const override;
    // 插件对应服务名（服务类型）
    const char* CALL_TYPE Srv(void) const override;
    // 插件对应动态库文件（动态库类型）
    const char* CALL_TYPE SoFile(void) const override;
    // 对应服务控制器名称
    const char* CALL_TYPE CtrlName(void) const override;
    // 控制器方法类型
    const char* CALL_TYPE Method(void) const override;
    // 控制器正文
    const char* CALL_TYPE Content(void) const override;
    // 是否根调用
    bool CALL_TYPE IsRootRun(void) const override;
    // 获取GET参数
    const char* CALL_TYPE GetGetArg(const char*, const char* = "") const override;
    // 获取POST参数
    const char* CALL_TYPE GetPostStr(void) const override;
    // 得到其他子节点信息
    const char* CALL_TYPE GetSubInfo(const char* sNode, const char* sAttr = "", const char* sDefault = "") const override;
    // 得到其他信息
    const char* CALL_TYPE GetOtherInfo(const char*, const char* = "") const override;

    // 控制器返回结果json
    IKCJson& CALL_TYPE JsonRespond(void) override;
    // 控制器返回结果json
    void CALL_TYPE SetJsonRespond(int errNo, const char* errMsg) override;
    // 控制器附加参数的json
    const IKCJson& CALL_TYPE JsonAttach(void) const override;
    const IKCJson& CALL_TYPE JsonAttachByKcFile(void) const override;
    // 输入参数的json
    const IKCJson& CALL_TYPE JsonRequest(void) const override;

    // 保存内部数据集
    void CALL_TYPE DataSetSaveToInner(CSqlFunc::TRecordSetPtr) override;

public:
    // 抛一个c++异常
    void CALL_TYPE Throw(const char* ex, const char* place) const override;
    // 获取本地化提示信息
    const char* CALL_TYPE GetHint(const char* key) const override;
    // 各单个请求信息
    const char* CALL_TYPE GetSingleInfo(const char*, const char* = "") const override;
    // Json库
    std::string CALL_TYPE JsonLibrary(void) const override;
    // 是否区分大小写
    bool CALL_TYPE JsonCaseSensitive(void) const override;
    // 字符集
    const char* CALL_TYPE GetCharset(void) const override;
    // 无效的json
    IKCJson& CALL_TYPE InvalidJson(void) override;
    // 得到配置的日志等级
    int CALL_TYPE GetCfgLogLevel(void) const override;
    // 获取固定字段的名称
    std::string CALL_TYPE GetFixParmName(std::string) const override;

public:
    // 活动数据类实例
    CActionData &Act(void) { return m_act; }
    // kc文件xml解析类实例
    const property_tree::ptree &Pt(void) const { return m_pt; }
    const property_tree::ptree &PtCtrlApi(void) const { return m_ptCtrlApi; }
    // 控制器返回结果json
    CKCJsonPackRespond& JsonRes(void) { return m_jsonRespond; }

    // 执行结果
    void SetPerformResult(void);

    // 控制器正文（用于替换）
    string& ContentRef(void) { return const_cast<string&>(m_content); }
    // 读取控制器正文
    string ReadContentFromXML(void);

    // 获取属性值
    std::tuple<bool, string> GetXmlattrStrBase(string sAttr);
    string GetXmlattrStr(string sAttr, string sDef = "");
    // 获取属性值（包括文件全局配置）
    bool GetXmlattrValFul(string sAttr, bool bDef = false);
    string GetXmlattrStrFul(string sAttr, string sDef = "");

    // 交换内部数据集
    void SwapInnerDbSet(CCtrApilDataBase& ctrlD)
    {
        this->m_innerDbSet.swap(ctrlD.m_innerDbSet);
    }

public:
    // 批量组操作的内部临时参数
    typedef std::weak_ptr<const CKCJsonPack> CKCJsonPackWeakPtr;
    CKCJsonPackWeakPtr m_batchGrpInTmpPrm;

    // 批量组操作的内部数据集行数据的临时json
    struct TBatchGrpInDbSetTmpJson
    {
        CKCJsonPackRespondPtr m_json;
        TBatchGrpInDbSetTmpJson(CActionData& act);
    };
    typedef std::shared_ptr<TBatchGrpInDbSetTmpJson> TBatchGrpInDbSetTmpJsonPtr;
    typedef std::weak_ptr<TBatchGrpInDbSetTmpJson> TBatchGrpInDbSetTmpJsonWeakPtr;
    TBatchGrpInDbSetTmpJsonWeakPtr m_batchGrpInDbSetTmpJson;

protected:
    // 构造
    CCtrApilDataBase(CActionData& act, string sUrl, string sGetArg, string sKCFilePath, string sSign, bool isRootRun, CKCJsonPackWeakPtr = CKCJsonPackWeakPtr(), TBatchGrpInDbSetTmpJsonWeakPtr = TBatchGrpInDbSetTmpJsonWeakPtr());
    // 临时调用时的构造函数
    CCtrApilDataBase(std::weak_ptr<CCtrApilDataBase> own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut);

protected:
    // 请求应答的上下文
    IKCRequestRespondData& m_re;
    // 本次执行的活动数据
    CActionData &m_act;
    // 是否根执行
    const bool m_isRootRun = true;
    // GET参数
    map<string, string> m_getParms;
    // 控制器的xml
    property_tree::ptree m_pt;
    // 插件对应服务名 取自文件根节点
    bool m_bundleSrvFromRoot = false;
    // 控制器参数：请求的链接、控制器文件、完整节点、控制器标识名、插件类型、插件对应服务名、对应服务控制器名称、插件对应动态库文件、控制器方法类型、控制器内容
    const string m_url, m_kcFilePath, m_node, m_ctrlSign, m_bundleType, m_bundleSrv, m_bundleCtrlName, m_bundleSoFile, m_method, m_content;
    // 控制器的xml
    property_tree::ptree &m_ptCtrlApi;

    // 默认字符集
    const string m_charset = c_RESTful_UTF8;
    // 设置json是否区分大小写
    const bool m_jsonCaseSensitive = true;

    // 控制器附加参数（xml节点Attach属性的json值）
    const CKCJsonPackAttach m_attachJson;
    // KC文件附加参数（xml节点Attach属性的json值）
    const CKCJsonPackAttach m_attachJsonKcFile;
    // 请求的参数
    const CKCJsonPackRequest m_jsonRequest;
    // 本控制器api调用的返回结果
    CKCJsonPackRespond m_jsonRespond;
    // 本控制器保存的内部数据集
    map<string, CSqlFunc::TRecordSetPtr> m_innerDbSet;

public:
    // 固定的参数字段名称：输入参数
    const string m_nameInParms = c_RESTful_inParm;
};
class CCtrApilData : public CCtrApilDataBase
{
    friend class CRootCtrlApiData;
    friend class CSubCtrlApiData;

public:
    typedef std::weak_ptr<CCtrApilData> CCtrApilDataWeakPtr;

    // 执行控制器
    int WorkAct(const char*, const char*) override;
    void ExecBundle(const char*, const char*, const char*, const char* = "", const char* = "") override;
    void ExecBundleSub(const char*, const char*, const char*, const char* = "", const char* = "") override;
    void ExecSo(const char*, const char*, const char*, const char* = "", const char* = "") override;
    void ExecSoSub(const char*, const char*, const char*, const char* = "", const char* = "") override;

    // 上一个控制器返回结果json
    const IKCJson& CALL_TYPE JsonRespondLast(void) const override;

    // 获取上一个控制器的内部数据集
    CSqlFunc::TRecordSetPtr CALL_TYPE DataSetInnerLast(string) override;

    // 获取批量操作的数组参数json
    const IKCJson& CALL_TYPE GetBatchArrayJson(const char* = "") const override;
    // 获取参数默认值json
    const IKCJson& CALL_TYPE GetDefParmJson(const char*) const override;
    // 获取用户输入参数值json
    const IKCJson& CALL_TYPE GetUserParmJson(const char*) const override;
    // 获取参数值json（依次查找上一个控制器的输出、用户输入的参数、默认的参数）
    const IKCJson& CALL_TYPE GetParmJson(const char* = "") const override;
    // 获取参数值（依次查找全局数据、上一个控制器的输出、用户输入的参数、默认的参数）
    const char* CALL_TYPE GetParmVal(const char*) const override;
    const char* CALL_TYPE GetParmVal(const char*, const char*) const override;

protected:
    // 根控制器构造
    CCtrApilData(CActionData& act);
    // 子控制器构造
    CCtrApilData(CCtrApilDataWeakPtr own, string sSubUrl, string sGetArg, string sKCFilePath, string sSign);
    // 临时调用时的构造函数
    CCtrApilData(CCtrApilDataWeakPtr own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut);

public:
    // 自己的弱指针引用（用于向子控制器api传递）
    CCtrApilDataWeakPtr selfRef(void) { return m_selfRef; }
    // 上一个控制器返回结果json
    const CKCJsonPackRespondRef& JsonResLast(void) const { return m_jsonRespondLast; }
    // 获取批量操作的数组参数json
    const CKCJsonPack& GetBatchArrayJsonImpl(string = "") const;

    // 移入移出结果
    CKCJsonPackRespond::TCoreParmJsonRespondPtr MoveOutResUp(void);
    bool MoveInRes(CKCJsonPackRespond::TCoreParmJsonRespondPtr);

protected:
    // 自己的弱指针引用（用于向子控制器api传递）
    CCtrApilDataWeakPtr m_selfRef;
    // 上一个控制器api调用的返回结果（用做本次控制器api调用的参数）
    const CKCJsonPackRespondRef m_jsonRespondLast;
    // 上一个控制器保存的内部数据集
    const map<string, CSqlFunc::TRecordSetPtr>* m_innerDbSetLast = nullptr;
};
typedef std::shared_ptr<CCtrApilData> CCtrApilDataPtr;
typedef CCtrApilData::CCtrApilDataWeakPtr CCtrApilDataWeakPtr;

// 根控制器API执行的数据
class CRootCtrlApiData : public CCtrApilData
{
public:
    // 创建
    static std::shared_ptr<CRootCtrlApiData> CreatePtr(CActionData& act);

    // 是否需要登录
    bool NeedToken(void) const { return m_needToken; }

protected:
    CRootCtrlApiData(CActionData& act);

protected:
    // 根控制器是否需要验证登录
    const bool m_needToken = false;
};

// 子控制器API执行的数据
class CSubCtrlApiData : public CCtrApilData
{
public:
    // 创建
    static CCtrApilDataPtr CreatePtr(CActionData& act, CCtrApilDataWeakPtr own, string sSubNode, string sSubUrl, string sGetArg, string sKCFilePath);
    static CCtrApilDataPtr CreatePtr(CActionData& act, CCtrApilDataWeakPtr own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut);

protected:
    CSubCtrlApiData(CCtrApilDataWeakPtr own, string sSubUrl, string sGetArg, string sKCFilePath, string sSign);
    // 临时调用时的构造函数
    CSubCtrlApiData(CCtrApilDataWeakPtr own, string btype, string sSrv, string sSo, string sCtrlName, string sMethod, string sContent, string sGoto, string sOut);

protected:
    // 父控制器Api数据接口
    CCtrApilDataWeakPtr m_own;
};
