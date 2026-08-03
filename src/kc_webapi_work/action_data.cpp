#include "action_data.h"
#include "ctrlapi_data.h"
#include "kc_webapi_work.h"

// 最新的活动页编号
std::atomic_ullong CActionData::m_actMaxID(0);
// 随机数
std::default_random_engine CActionData::m_rand((unsigned)time(nullptr));
std::uniform_int_distribution<unsigned> CActionData::m_randRange(0x10, 0xff);

////////////////////////////////////////////////////////////////////////////////
// CActionData类
CActionData::CActionData(CKCWebApiWork& wapi, IKCRequestRespondData& re)
    : m_charset(wapi.Charset()), m_jsonCaseSensitive(wapi.JsonCaseSensitive()), m_needToken(!wapi.NeedToken().empty())
    , m_webapi(wapi), m_actID(++m_actMaxID), m_re(re), m_GrpBranchCtrl(wapi.GrpBranchCtrl())
    , m_NowTimeFlag(CUtilFuncEx::BaseXXEncode((format("%X") % CUtilFunc::GetCurrentStampMS()).str()))
    , m_invalidJson(*this)
    , m_jsonRequest(*this, [&](){
        string sJson;
        unsigned iLen = 0;
        if (!re.IsMultipartFormData()) sJson = re.GetPostArgStr();
        else if (re.GetMultiFormDataCount() > 0)
        {
            re.SetCurrentMultiFormData(0);
            IMultiFormData& mfd = re.GetMultiFormData();
            if (!mfd.isFile()) sJson = mfd.GetBody(iLen);
        }
        return algorithm::trim_copy(sJson);
    }())
    , m_jsonRespond(*this)
{
    m_re.SetActionData(this);
    cout << "*[Knewcode] Default Charset: " << m_charset << endl;
}

CActionData::~CActionData()
{
    m_re.SetActionData(nullptr);
    for (auto it : m_actObj)
    {
        try
        {
            it.second->Release();
            it.second = nullptr;
        }
        catch (...) {}
    }
    m_actObj.clear();
    m_actData.clear();
}

// Json库
std::string CActionData::JsonLibrary(void) const
{
    return m_webapi.JsonLibrary();
}
// 是否区分大小写
bool CActionData::JsonCaseSensitive(void) const
{
    return m_jsonCaseSensitive;
}
// 字符集
const char* CActionData::GetCharset(void) const
{
    return m_charset.c_str();
}
// 得到配置的日志等级
int CActionData::GetCfgLogLevel(void) const
{
    return m_webapi.getContext().GetCfgLogLevel();
}

// 获取固定字段的名称
std::string CActionData::GetFixParmName(std::string name) const
{
    return name;
}

// 写日志
bool CActionData::WriteLogTrace(const char* info, const char* place, const char* other) const
{
    return m_webapi.WriteLogTrace(info, place, other);
}
bool CActionData::WriteLogDebug(const char* info, const char* place, const char* other) const
{
    return m_webapi.WriteLogDebug(info, place, other);
}
bool CActionData::WriteLogInfo(const char* info, const char* place, const char* other) const
{
    return m_webapi.WriteLogInfo(info, place, other);
}
bool CActionData::WriteLogWarning(const char* info, const char* place, const char* other) const
{
    return m_webapi.WriteLogWarning(info, place, other);
}
bool CActionData::WriteLogError(const char* info, const char* place, const char* other) const
{
    return m_webapi.WriteLogError(info, place, other);
}
bool CActionData::WriteLogFatal(const char* info, const char* place, const char* other) const
{
    return m_webapi.WriteLogFatal(info, place, other);
}

// 垃圾回收
void CActionData::GC(void)
{
    m_webapi.GC();
}

// 获取本地化提示信息
const char* CActionData::GetHint(const char* key) const
{
    return m_webapi.getHint(key);
}

// 抛一个c++异常
void CActionData::Throw(const char* ex, const char* place) const
{
    throw TActionException(ecd_ErrCode_KCWebApiWork + 99, place, ex, typeid(*this).name());
}

// 全局标识符（变量、常量）
bool CActionData::IsGlobalVal(const char* pName) const
{
    return GetGlobalValType(pName) != IActionData::egtIsnot;
}
const char* CActionData::GetGlobalVal(const char* pName, const char* pDef)
{
    static thread_local string sResult;
    sResult.clear();
    string sName = CUtilFunc::PCharSafeToStr(pName);
    IActionData::EGlobalValTpe eGloVTp = GetGlobalValType(pName);
    // 系统常量（不能修改）
    if (IActionData::egtGloConst == eGloVTp || IActionData::egtConst == eGloVTp)
    {
        string sConst = sName.substr((IActionData::egtConst == eGloVTp ? sizeof(c_RESTful_KCConst) : sizeof(c_RESTful_KCGlobalConst)) - 1);
        // 服务器编号
        if (c_RESTful_srvID == sConst)
        {
            sResult = std::to_string(m_webapi.getContext().GetSrvID());
            return sResult.c_str();
        }
        // 系统标志
        else if (c_RESTful_sysFlag == sConst)
            return m_webapi.getContext().GetSysFlag("customer", m_GrpBranchCtrl.c_str());
        // 持久对象编号
        else if (c_RESTful_KCAliveID == sConst)
            return GetAliveID();
        // 进程编号
        else if (c_RESTful_KCPID == sConst)
        {
            sResult = std::to_string(this_process::get_id());
            return sResult.c_str();
        }
        // 链接编号
        else if (c_RESTful_KCConnectID == sConst)
        {
            sResult = (format("%d_%s") % this_process::get_id() % m_re.GetSingleInfo("connection_id")).str();
            return sResult.c_str();
        }
        // 控制器是否需要登录
        else if (c_RESTful_needToken == sConst)
            return m_needToken ? "1" : "0";
        // 服务器当前时间戳（毫秒）
        else if (c_RESTful_SrvTimestamp == sConst)
        {
            sResult = std::to_string(CUtilFunc::GetCurrentStampMS());
            return sResult.c_str();
        }
        // 服务器当前时间
        else if (c_RESTful_SrvDatatime == sConst)
        {
            sResult = CUtilFunc::TimeToStr();
            return sResult.c_str();
        }
        // Web常量
        else
            return m_re.GetSingleInfo(sConst.c_str(), pDef);
    }
    // GET参数
    else if (IActionData::egtGetParm == eGloVTp)
    {
        string sGetParm = boost::trim_copy(sName.substr(sizeof(c_RESTful_KCGetParm) - 1));
        // 不存在时，返回默认值
        return sGetParm.empty() ? m_re.GetGetArgStr() : m_re.GetGetArg(sGetParm.c_str(), pDef);
    }
    // session（除了编号，其他都可修改）
    else if (IActionData::egtSession == eGloVTp)
    {
        string sSession = sName.substr(sizeof(c_RESTful_KCSession) - 1);
        // client编号（不能修改）
        if (string("_") + c_RESTful_KCClientID == sSession)
            return this->GetKCCLNID();
        // session编号（不能修改）
        else if (string("_") + c_RESTful_KCSessoinID == sSession)
            return this->GetKCSSID();
        // session数据（不存在时，返回nullptr）
        else
            return m_webapi.Session().GetSessionVal(sSession.c_str(), *this);
    }
    // 本次请求的全局数据
    else if (IActionData::egtAction == eGloVTp)
    {
        string sActName = sName.substr(sizeof(c_RESTful_KCAct) - 1);
        // 活动页编号（不能修改）
        if (c_RESTful_KCActionID == sActName)
        {
            sResult = (format("%d_%lld") % this_process::get_id() % m_actID).str();
            return sResult.c_str();
        }
        // 当前时间标识，base62编码（不能修改）
        else if (c_RESTful_KCNowTimeFlag == sActName) return m_NowTimeFlag.c_str();
        // 字符集
        else if (c_RESTful_Charset == sActName) return GetCharset();
        // 分支标志
        else if (c_RESTful_grpBranchCtrl == sActName) return m_GrpBranchCtrl.c_str();
        // 其它自定义（不存在时，返回默认值）
        else
        {
            auto it = m_actData.find(sActName);
            if (m_actData.end() == it) return pDef;
            else return it->second.c_str();
        }
    }
    // 直接读取json
    else if (IActionData::egtJson == eGloVTp)
    {
        string sActName = sName.substr(sizeof(c_RESTful_KCJSON) - 1);
        const IKCJson &json = JsonRequest().GetItem(sActName.c_str(), "__");
        // 不存在时，返回nullptr
        if (json.IsValid()) return json.GetStr();
    }
    return nullptr;
}
void CActionData::SetGlobalVal(const char* pName, const char* pVal, unsigned iLen)
{
    string sName = CUtilFunc::PCharSafeToStr(pName);
    IActionData::EGlobalValTpe eGloVTp = GetGlobalValType(pName);
    // session
    if (IActionData::egtSession == eGloVTp)
    {
        string sSession = sName.substr(sizeof(c_RESTful_KCSession) - 1);
        if (iLen > 0)
            m_webapi.Session().SetSessionVal(sSession.c_str(), pVal, iLen, *this);
        else
            m_webapi.Session().SetSessionVal(sSession.c_str(), pVal, *this);
    }
    // 本次请求的全局数据
    else if (IActionData::egtAction == eGloVTp)
    {
        string sActName = sName.substr(sizeof(c_RESTful_KCAct) - 1);
        // 字符集
        if (c_RESTful_Charset == sActName)
        {
            // 字符集
            m_jsonRespond.SetStr(c_RESTful_Charset, pVal);
            if (algorithm::to_upper_copy(string(pVal)) != GetCharset())
            {
                // todo:
                // m_re.SetCharset(algorithm::to_upper_copy(string(pVal)).c_str());
                // m_jsonRespond.ResetByCharSet();
            }
        }
        // 分支标志
        else if (c_RESTful_grpBranchCtrl == sActName)
            m_GrpBranchCtrl = CUtilFunc::PCharSafeToStr(pVal);
        // 其他全局数据
        else
        {
            string sVal = "";
            if (nullptr != pVal) sVal = iLen > 0 ? string(pVal, iLen) : pVal;
            auto it = m_actData.find(sActName);
            if (m_actData.end() != it) it->second = sVal;
            else m_actData.insert(make_pair(sActName, sVal));
        }
    }
    // 直接写入json
    else if (IActionData::egtJson == eGloVTp)
    {
        string sActName = sName.substr(sizeof(c_RESTful_KCJSON) - 1);
        IKCJson &json = JsonRespond().GetItem(sActName.c_str(), "__");
        if (json.IsValid()) json.SetStr(pVal);
    }
}
// 设置全局标识符有效期限
void CActionData::SetSessionExpire(const char* pName, const char* pExpire)
{
    string sName = CUtilFunc::PCharSafeToStr(pName);
    // session
    if (GetGlobalValType(pName) == IActionData::egtSession)
    {
        string sSession = sName.substr(sizeof(c_RESTful_KCSession) - 1);
        m_webapi.Session().SetSessionExpire(sSession.c_str(), pExpire, *this);
    }
}
// 全局标识符类型
IActionData::EGlobalValTpe CActionData::GetGlobalValType(const char* pName) const
{
    IActionData::EGlobalValTpe eResult = IActionData::egtIsnot;
    if (nullptr != pName)
    {
        string sName(pName);
        // 系统常量
        if (sName.substr(0, sizeof(c_RESTful_KCGlobalConst) - 1) == c_RESTful_KCGlobalConst)
            eResult = IActionData::egtGloConst;
        else if (sName.substr(0, sizeof(c_RESTful_KCConst) - 1) == c_RESTful_KCConst)
            eResult = IActionData::egtConst;
        // GET参数
        else if (sName.substr(0, sizeof(c_RESTful_KCGetParm) - 1) == c_RESTful_KCGetParm)
            eResult = IActionData::egtGetParm;
        // session
        else if (sName.substr(0, sizeof(c_RESTful_KCSession) - 1) == c_RESTful_KCSession)
        {
            string sSession = sName.substr(sizeof(c_RESTful_KCSession) - 1);
            if (string("_") + c_RESTful_KCClientID == sSession || string("_") + c_RESTful_KCSessoinID == sSession)
                eResult = IActionData::egtSession;
            else if (m_webapi.Session().IsSession(sSession.c_str()))
                eResult = IActionData::egtSession;
        }
        // 本次请求的全局数据
        else if (sName.substr(0, sizeof(c_RESTful_KCAct) - 1) == c_RESTful_KCAct)
        {
            //string sActName = sName.substr(sizeof(c_RESTful_KCAct) - 1);
            //if (c_RESTful_Charset == sActName) return true;
            //else return m_actData.find(sActName) != m_actData.end();
            eResult = IActionData::egtAction;
        }
        // 直接读取json
        else if (sName.substr(0, sizeof(c_RESTful_KCJSON) - 1) == c_RESTful_KCJSON)
        {
            eResult = IActionData::egtJson;
        }
    }
    return eResult;
}

// 新建浏览器编号
string CActionData::NewBrowserID(int iType)
{
    string sBrwID;
    // 类型
    string sType = 0 == iType ? c_RESTful_KCClientID : c_RESTful_KCSessoinID;
    // 使用文件session里的自增id
    auto fSessionInc = [&](void)
    {
        string sCfgNode = string("Config.Modules.") + m_webapi.getBundle().getName() + "." + sType;
        string sSessionCtrl = m_webapi.getContext().GetCfgInfo(sCfgNode.c_str(), "SessionCtrl");
        if (!sSessionCtrl.empty())
        {
            string sSession = sSessionCtrl + "__next_" + sType;
            string sNextVal = CUtilFunc::PCharSafeToStr(m_webapi.Session().GetSessionNextVal(sSession.c_str(), "10000", *this));
            if (!sNextVal.empty())
                sBrwID = (format("%d_%s") % m_webapi.getContext().GetSrvID() % sNextVal).str();
        }
    };
    // 使用客户端IP、端口、时间戳的base62编码
    string sHex;
    auto fTimeStamp = [&](void)
    {
        unsigned char arrIpPort[6] = { 0 };
        // 客户端ip
        string sClnIP = m_re.GetSingleInfo("client_ip");
        if ("::1" == sClnIP || sClnIP.empty()) sClnIP = "127.0.0.1";
        vector<string> vecIP;
        algorithm::split(vecIP, sClnIP, is_any_of("."));
        for (int i = 0, c = static_cast<int>(vecIP.size()); i < c; ++i)
            arrIpPort[i] = static_cast<unsigned char>(atoi(vecIP[i].c_str()));
        // 客户端端口
        unsigned short iClnPort = static_cast<unsigned short>(atoi(m_re.GetSingleInfo("client_port")));
        //CUtilFunc::RotateDB(&iClnPort, sizeof(iClnPort));   // 使编码后，尽可能短
        memcpy(&arrIpPort[4], &iClnPort, 2);
        // 随机数
        unsigned iRnd = CActionData::m_randRange(CActionData::m_rand);
        // 转变为16进制字符串
        sHex = (format("%X%s%X%X") % iRnd % CUtilFunc::TransHex(arrIpPort, sizeof(arrIpPort)) % CUtilFunc::GetCurrentStampS() % m_webapi.getContext().GetSrvID()).str();
        // base62编码
        sBrwID = CUtilFuncEx::BaseXXEncode(sHex);
    };
    if (sBrwID.empty() && 0 == iType) fSessionInc();
    if (sBrwID.empty()) fTimeStamp();
    // 日志
    m_webapi.WriteLogInfo("New Browser", __CURR_CODE_PLACE_C__, (format("[%s=%s (%s)] %s [%s:%s]\n") % sType % sBrwID % sHex % m_re.GetSingleInfo("User_Agent") % m_re.GetSingleInfo("client_ip") % m_re.GetSingleInfo("client_port")).str().c_str());
    return sBrwID;
}

// 取客户端上传的浏览器编号
string CActionData::ClnBrowserID(string sType)
{
    // 优先从post的json数据里取
    string sBrwID = CUtilFunc::PCharSafeToStr(JsonRequest().GetStr(sType.c_str(), ""));
    // 从get参数里取
    if (sBrwID.empty()) sBrwID = CUtilFunc::PCharSafeToStr(m_re.GetGetArg(sType.c_str()));
    // 从cookie里取
    if (sBrwID.empty()) sBrwID = CUtilFunc::PCharSafeToStr(m_re.GetCookieVal(sType.c_str()));
    // 返回
    return sBrwID;
}

// 刷新客户端cookie
void CActionData::ResetCookie(int iType, string sType, string sBrwID)
{
    string sCookieID = CUtilFunc::PCharSafeToStr(m_re.GetCookieVal(sType.c_str()));
    if (sBrwID != sCookieID)
    {
        if (0 == iType)
        {
            // cookie期限
            string sExpires = (format(c_CookieExpiredDate) % (boost::gregorian::day_clock::local_day().year() / 100 + 1)).str();
            //m_re.AddCookie(sType.c_str(), sBrwID.c_str(), "Tue, 23 Apr 2475 12:34:56 GMT");
            m_re.AddCookie(sType.c_str(), sBrwID.c_str(), sExpires.c_str());
        }
        else m_re.AddCookie(sType.c_str(), sBrwID.c_str());
        m_jsonRespond.SetStr(sType.c_str(), sBrwID.c_str());
    }
}

// Client编号
const char* CActionData::GetKCCLNID(void)
{
    this->GetBrowserID<0>(m_clientID);
    return m_clientID.c_str();
}
// Session编号
const char* CActionData::GetKCSSID(void)
{
    this->GetBrowserID<1>(m_sessionID);
    return m_sessionID.c_str();
}

// 验证客户端和Session编号
void CActionData::CheckKCSSID(void)
{
    if (m_re.getRe().IsSubCall()) return;
    m_clientID = ClnBrowserID(c_RESTful_KCClientID);
    if (m_clientID.size() <= 10)
    {
        m_clientID.clear();
        this->GetBrowserID<0>(m_clientID, true);
        m_jsonRespond.SetStr(c_RESTful_KCClientID, m_clientID.c_str());
    }
    // 向浏览器设置cookie
    else ResetCookie(0, c_RESTful_KCClientID, m_clientID);
}
void CActionData::CheckSessinID(void)
{
    if (m_re.getRe().IsSubCall()) return;
    string sSsidIP;
    // 客户端ip
    string sClnIP = m_re.GetSingleInfo("client_ip");
    if (sClnIP.empty() || "::1" == sClnIP) sClnIP = "127.0.0.1";
    // 客户端session编号
    m_sessionID = ClnBrowserID(c_RESTful_KCSessoinID);
    // 获取session编号对应的客户端ip，从全局session
//    string sSessName;
//    string innerSessionCtrl = m_webapi.GetInnerSessionCtrl();
//    if (!innerSessionCtrl.empty())
//    {
//        sSessName = c_RESTful_KCSession + innerSessionCtrl + "__ipv4_" + m_sessionID;
//        sSsidIP = CUtilFunc::PCharSafeToStr(GetGlobalVal(sSessName.c_str()));
//    }
    // 从Session编号里解析
    if (sSsidIP.empty() && m_sessionID.size() > 10)
    {
        // session编号里的ip
        string sHex = CUtilFuncEx::BaseXXDecode(m_sessionID);
        for (int i = 2, c = std::min(static_cast<int>(sHex.size()), 10); i < c; i += 2)
        {
            string sIPnd = sHex.substr(i, 2);
            unsigned int iIPnd = std::stoul(sIPnd, nullptr, 16);
            sSsidIP += std::to_string(iIPnd) + ".";
        }
        algorithm::trim_if(sSsidIP, boost::is_any_of("."));
        // 将session编号对应的客户端ip，保存到全局session
//        if (!sSessName.empty()) SetGlobalVal(sSessName.c_str(), sClnIP.c_str());
    }
    // session编号里的ip，与客户端的ip必须一致
    if (sClnIP != sSsidIP)
    {
        m_sessionID.clear();
        this->GetBrowserID<1>(m_sessionID, true);
        m_jsonRespond.SetStr(c_RESTful_KCSessoinID, m_sessionID.c_str());
    }
    // 向浏览器设置cookie
    else ResetCookie(1, c_RESTful_KCSessoinID, m_sessionID);
}

// 输入参数的json
const IKCJson& CActionData::JsonRequest(void) const
{
    return m_jsonRequest;
}
// 执行结果的json
IKCJson& CActionData::JsonRespond(void)
{
    return m_jsonRespond;
}

// 自定义json
IKCJson& CActionData::MakeJson(const char* json)
{
    return *new CKCJsonPackRespond(*this, json);
}
void CActionData::FreeJson(IKCJson& json)
{
    delete &json;
}

// 框架上下文
IKCContext& CActionData::Context(void)
{
    return m_webapi.getContext();
}

// 本次请求的活动对象
void CActionData::AddActObj(const char* sName, IActionData::TActObj* pObj)
{
    if (nullptr == pObj)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 2, __CURR_CODE_PLACE_C__, string(this->GetHint("Null_Point_Data_")) + sName + "\n" + m_re.GetSingleInfo("the_request"), m_webapi);
    auto it = m_actObj.find(sName);
    if (m_actObj.end() != it)
        throw TKCWebApiWorkException(ecd_ErrCode_KCWebApiWork + 3, __CURR_CODE_PLACE_C__, string(this->GetHint("Repeat_Register_Data_")) + sName + "\n" + m_re.GetSingleInfo("the_request"), m_webapi);
    m_actObj.insert(make_pair(string(sName), pObj));
}
void CActionData::DelActObj(const char* sName)
{
    auto it = m_actObj.find(sName);
    if (m_actObj.end() != it)
    {
        if (nullptr != it->second) it->second->Release();
        m_actObj.erase(it);
    }
}
IActionData::TActObj* CActionData::GetActObj(const char* sName)
{
    auto it = m_actObj.find(sName);
    if (m_actObj.end() == it) return nullptr;
    else return it->second;
}

// 持久链接的活动对象
IActionData::TAliveObj& CActionData::GetAliveObj(void)
{
    return m_webapi.GetAliveObj(GetAliveID());
}
const char* CActionData::GetAliveID(void)
{
    static thread_local string sAliveID;
    //sAliveID = (format("%s-%s") % GetKCSSID() % m_re.GetSingleInfo("connection_id")).str();
    sAliveID = (format("%d-%d-%s") % m_webapi.getContext().GetSrvID() % this_process::get_id() % GetKCSSID()).str();
    return sAliveID.c_str();
}

// 默认的返回数据集的名称
const char* CActionData::GetValsName(void) const
{
    return m_webapi.DefaultValsName();
}

// 得到本次执行的时间戳标识
const char* CActionData::GetNowTimeFlag(void)
{
    return m_NowTimeFlag.c_str();
}

// 各单个请求信息
const char* CActionData::GetSingleInfo(const char* pName, const char* pDef) const
{
    return m_re.GetSingleInfo(pName, pDef);
}

// 得到本地完整根目录
const char* CActionData::GetLocalFullPath(const char* pth)
{
    // return m_re.GetLocalPath(pth);
    static thread_local string sLocalPth;
    sLocalPth = CUtilFunc::PCharSafeToStr(pth);
    // 完整路径
    if (!boost::filesystem::path(sLocalPth).is_absolute())
    {
        bool bInRoot = sLocalPth[0] == '>';
        trim_left_if(sLocalPth, is_any_of(">"));
        sLocalPth = (bInRoot ? Context().getPath() + string("/") + sLocalPth : re().GetLocalFilename(sLocalPth.c_str()));
    }
    return sLocalPth.c_str();
}
// 得到url对应的本地路径
const char* CActionData::GetUrlLocalPath(const char* url)
{
    return GetLocalFullPath(re().GetLocalFilename(url));
}

// 得到网页根路径
const char* CActionData::GetUrlPageRootPath(const char* uri)
{
    // 通过请求网站的虚拟目录配置
    const char* sReVPth = re().GetUrlPageRootPath(uri);
    if (nullptr != sReVPth && strlen(sReVPth) > 1) return sReVPth;
    // 后端api目录配置
    const char* sKcApi = m_webapi.GetUrlPageRootPath(uri);
    if (nullptr != sKcApi && strlen(sKcApi) > 1) return sKcApi;
    // 返回默认
    return "/";
}

// 虚拟目录
unsigned CActionData::VirtualPathCount(void)
{
    return re().VirtualPathCount();
}
const char* CActionData::GetVirtualPath(unsigned pos)
{
    return re().GetVirtualPath(pos);
}
const char* CActionData::GetVirtualPathUri(unsigned pos)
{
    return re().GetVirtualPathUri(pos);
}
