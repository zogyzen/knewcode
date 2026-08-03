#pragma once

#include "std.h"
#include "json_pack.h"

class CKCWebApiWork;
// 本次执行的活动数据
class CActionData : public IActionData, public IJsonCallBack
{
public:
    CActionData(CKCWebApiWork&, IKCRequestRespondData&);
    ~CActionData() override;

    // 写日志
    bool CALL_TYPE WriteLogTrace(const char*, const char*, const char* = "") const override;
    bool CALL_TYPE WriteLogDebug(const char*, const char*, const char* = "") const override;
    bool CALL_TYPE WriteLogInfo(const char*, const char*, const char* = "") const override;
    bool CALL_TYPE WriteLogWarning(const char*, const char* = "", const char* = "") const override;
    bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const override;
    bool CALL_TYPE WriteLogFatal(const char*, const char*, const char* = "") const override;
    // 垃圾回收
    void CALL_TYPE GC(void) override;
    // 获取本地化提示信息
    const char* CALL_TYPE GetHint(const char* key) const override;
    // 抛一个c++异常
    void CALL_TYPE Throw(const char* ex, const char* place) const override;
    // 全局标识符（变量、常量）
    bool CALL_TYPE IsGlobalVal(const char*) const override;
    const char* CALL_TYPE GetGlobalVal(const char*, const char* = "") override;
    void CALL_TYPE SetGlobalVal(const char*, const char*, unsigned = 0) override;
    // 设置全局标识符有效期限
    void CALL_TYPE SetSessionExpire(const char*, const char* = "1d") override;
    // 全局标识符类型
    IActionData::EGlobalValTpe CALL_TYPE GetGlobalValType(const char*) const override;
    // Client编号
    const char* CALL_TYPE GetKCCLNID(void) override;
    // Session编号
    const char* CALL_TYPE GetKCSSID(void) override;
    // 输入参数的json
    const IKCJson& CALL_TYPE JsonRequest(void) const override;
    // 执行结果的json
    IKCJson& CALL_TYPE JsonRespond(void) override;
    // 无效json
    IKCJson& CALL_TYPE InvalidJson(void) override { return m_invalidJson; }
    // 自定义json
    IKCJson& CALL_TYPE MakeJson(const char* = "{}") override;
    void CALL_TYPE FreeJson(IKCJson&) override;
    // 框架环境
    IKCContext& CALL_TYPE Context(void) override;
    // 本次请求的活动对象
    void CALL_TYPE AddActObj(const char* sName, IActionData::TActObj*) override;
    void CALL_TYPE DelActObj(const char* sName) override;
    IActionData::TActObj* CALL_TYPE GetActObj(const char* sName) override;
    // 持久链接的活动对象
    TAliveObj& CALL_TYPE GetAliveObj(void) override;
    // 默认的返回数据集的名称
    const char* CALL_TYPE GetValsName(void) const override;
    // 得到本次执行的时间戳标识
    const char* CALL_TYPE GetNowTimeFlag(void) override;
    // 各单个请求信息
    const char* CALL_TYPE GetSingleInfo(const char*, const char* = "") const override;
    // 得到本地完整文件名
    const char* CALL_TYPE GetLocalFullPath(const char*) override;
    // 得到url对应的本地路径
    const char* CALL_TYPE GetUrlLocalPath(const char*) override;
    // 得到网页根路径
    const char* CALL_TYPE GetUrlPageRootPath(const char*) override;
    // 虚拟目录
    unsigned CALL_TYPE VirtualPathCount(void) override;
    const char* CALL_TYPE GetVirtualPath(unsigned) override;
    const char* CALL_TYPE GetVirtualPathUri(unsigned) override;

public:
    // Json库
    std::string CALL_TYPE JsonLibrary(void) const override;
    // 是否区分大小写
    bool CALL_TYPE JsonCaseSensitive(void) const override;
    // 字符集
    const char* CALL_TYPE GetCharset(void) const override;
    // 得到配置的日志等级
    int CALL_TYPE GetCfgLogLevel(void) const override;
    // 获取固定字段的名称
    std::string CALL_TYPE GetFixParmName(std::string) const override;

public:
    // 持久对象编号
    const char* GetAliveID(void);
    // 验证客户端和Session编号
    void CheckKCSSID(void);
    void CheckSessinID(void);

public:
    // 默认字符集
    const string m_charset = c_RESTful_UTF8;
    // 设置json是否区分大小写
    const bool m_jsonCaseSensitive = false;
    // 根控制器是否需要验证登录
    const bool m_needToken = true;

protected:
    CKCWebApiWork& m_webapi;
    // 活动页编号
    const unsigned long long m_actID = 0;
    static std::atomic_ullong m_actMaxID;
    // 请求应答接口
    IKCRequestRespondData& m_re;
    // client编号
    string m_clientID = "";
    // session编号
    string m_sessionID = "";
    // 执行组分支名称。可修改。
    string m_GrpBranchCtrl = "";
    // 当前时间标识
    string m_NowTimeFlag = "";
    // 本次请求的全局数据和对象
    map<string, string> m_actData;
    map<string, IActionData::TActObj*> m_actObj;
    // 随机数
    static std::default_random_engine m_rand;
    static std::uniform_int_distribution<unsigned> m_randRange;

protected:
    // 得到浏览器编号：iType=0客户端固定编号；=1本次会话编号
    template<int iType>
    void GetBrowserID(string &sBrwID, bool renew = false)
    {
        if (sBrwID.empty())
        {
            // 类型
            string sType = 0 == iType ? c_RESTful_KCClientID : c_RESTful_KCSessoinID;
            // 不是新建，先取客户端上传
            sBrwID = renew ? "" : ClnBrowserID(sType);
            // 针对ClientID，没取到，则取SessionID。
            if (sBrwID.empty() && 0 == iType) sBrwID = m_sessionID;
            // 没取到，则新建
            if (sBrwID.empty()) sBrwID = NewBrowserID(iType);
            // 向浏览器设置cookie
            ResetCookie(iType, sType, sBrwID);
        }
    }
    // 新建浏览器编号
    string NewBrowserID(int iType);
    // 取客户端上传的浏览器编号
    string ClnBrowserID(string sType);
    // 刷新客户端cookie
    void ResetCookie(int iType, string sType, string sBrwID);

public:
    CKCWebApiWork& webapi(void) { return m_webapi; }
    IKCRequestRespondData& re(void) { return m_re; }
    const CKCJsonPackRequest& JsonReq(void) { return m_jsonRequest; }
    CKCJsonPackRespond& JsonRes(void) { return m_jsonRespond; }
    CKCJsonPackInvalid& JsonInvalid(void) { return m_invalidJson; }

protected:
    // 无效json
    CKCJsonPackInvalid m_invalidJson;
    // 请求的参数
    const CKCJsonPackRequest m_jsonRequest;
    // 应答结果
    CKCJsonPackRespond m_jsonRespond;
};
