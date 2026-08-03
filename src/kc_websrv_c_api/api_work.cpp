#include "api_work.h"
#include "work_proxy_cb.h"

////////////////////////////////////////////////////////////////////////////////
// 链接回调类
class CWSProxyConnectCB : public IWSProxyConnectCB
{
public:
    CWSProxyConnectCB(CApiWork& own, intptr_t c, intptr_t r) : m_own(own), m_conr(c), m_r(r) {};

    // 获取连接ID
    long CALL_TYPE GetID(void) const override
    {
        return atol(m_own.GetConnInfo(m_conr, "connection_id"));
    }

    // 判断是否ssl
    bool CALL_TYPE IsSSL(void) const override
    {
        return m_own.IsSSL(m_conr);
    }

    // 设置长连接
    void CALL_TYPE SetKeepalive(bool b) override
    {
        m_own.SetKeepalive(m_conr, b);
    }
    // 设置断开的回调函数
    void CALL_TYPE SetBreakCB(void* cdb, FBreakCB fcb) override
    {
        m_own.SetBreakConnCB(m_conr, reinterpret_cast<intptr_t>(cdb), reinterpret_cast<FBreakConnCBFromOwn>(fcb));
    }
    // 关闭连接
    void CALL_TYPE CloseConn(void) override
    {
        m_own.CloseConn(m_conr);
    }
    // 连接是否关闭
    bool isCloseConn(void) const override
    {
        return m_own.IsCloseConn(m_conr);
    }
    // 请求应答上下文的回调
    IWSProxyRequestCB& CALL_TYPE MakeReCB(void) override
    {
        return *new CWSProxyRequestCB(m_own, m_own.m_proxy, m_r);
    }
    void CALL_TYPE ReleaseReCB(IWSProxyRequestCB& rcb) override
    {
        delete dynamic_cast<CWSProxyRequestCB*>(&rcb);
    }
    // 重置链接时间
    void CALL_TYPE ResetTime(unsigned ms) override
    {
        m_own.ResetConnTime(m_conr, ms);
    }
    // 发送
    int CALL_TYPE Send(const char* buf, unsigned len) override
    {
        return m_own.SendByConn(m_conr, buf, len);
    }
    // socket发送
    int CALL_TYPE SockSend(const char* buf, size_t& sz) override
    {
        return m_own.SockSendByConn(m_conr, buf, static_cast<unsigned>(sz));
    }
    // websocket接收
    int CALL_TYPE WscRecv(char* buf, size_t& sz) override
    {
        return m_own.WscRecvByConn(m_conr, buf, static_cast<unsigned>(sz));
    }

private:
    CApiWork& m_own;
    intptr_t m_conr = 0;
    intptr_t m_r = 0;
};

////////////////////////////////////////////////////////////////////////////////
// web服务回调类
class CWSProxyServerCB : public IWSProxyServerCB
{
public:
    CWSProxyServerCB(CApiWork& own) : m_own(own) {};

    // 宿主应用程序名称
    const char* CALL_TYPE OwnName(void) const override
    {
        return m_own.m_ownName.c_str();
    }

    // 宿主应用程序版本
    const char* CALL_TYPE OwnVersion(void) const override
    {
        return m_own.m_ownVersion.c_str();
    }

    // web服务程序的根目录
    const char* CALL_TYPE apPath(void) const override
    {
        return m_own.m_WebSrvPath.c_str();
    }
    // 主平台根目录
    const char* CALL_TYPE platformPath(void) const override
    {
        return m_own.m_PlatformPath.c_str();
    }
    // 网站或应用根路径
    const char* CALL_TYPE websitePath(void) const override
    {
        return m_own.m_WebsitePath.c_str();
    }
    // KC系统根目录
    const char* CALL_TYPE fxPath(void) const override
    {
        return m_own.m_FxPath.c_str();
    }
    // 主页根目录
    const char* CALL_TYPE pgPath(void) const override
    {
        return m_own.m_PgPath.c_str();
    }
    // 配置文件完整路径
    const char* CALL_TYPE cfgFile(void) const override
    {
        return m_own.m_CfgFile.c_str();
    }

    // web虚拟目录
    const char* CALL_TYPE VirtualPath(void) const override
    {
        return m_own.m_virtualPath.c_str();
    }

    // 链接
    IWSProxyConnectCB& CALL_TYPE MakeConnCB(IWSProxyRequestCB& recb) override
    {
        auto &r = dynamic_cast<CWSProxyRequestCB&>(recb).m_hRe;
        return *new CWSProxyConnectCB(m_own, m_own.GetRequestConn(r), r);
    }
    void CALL_TYPE ReleaseConnCB(IWSProxyConnectCB& cncb) override
    {
        delete dynamic_cast<CWSProxyRequestCB*>(&cncb);
    }

    // 判断是否断线状态
    bool isDisconn(int stt) const override
    {
        return m_own.IsDisconnByErrCode(stt);
    }
    // 获取状态的错误信息
    virtual const char* GetStatus(int stt) const override
    {
        return m_own.GetStatusByErrCode(stt);
    }

private:
    CApiWork& m_own;
};

////////////////////////////////////////////////////////////////////////////////
// CApiWork 工作类
CApiWork::CApiWork(void)
    : m_ownName(CUtilFunc::PCharSafeToStr(GetSrvInfo("Name"))), m_ownVersion(CUtilFunc::PCharSafeToStr(GetSrvInfo("Version")))
    , m_WebSrvPath([&](){
        string sPth = CUtilFunc::PCharSafeToStr(GetSrvInfo("MainExeModRoot"));
        if (sPth.empty()) sPth = ".";
        sPth = boost::filesystem::canonical(sPth).string();
        return sPth;
    }())
    , m_PlatformPath(GetSrvDir("PlatformRoot"))
    , m_WebsitePath(GetSrvDir("WebsiteRoot"))
    , m_FxPath(GetSrvDir("KnewcodeRoot"))
    , m_PgPath(GetSrvDir("DocumentRoot"))
    , m_CfgFile(GetSrvDir("KnewcodeCfgFile", "my-prj.xml"))
    , m_virtualPath([&](){
        string sVPth = CUtilFunc::PCharSafeToStr(GetSrvInfo("VirtualPath"));
        vector<string> vctItm;
        boost::algorithm::split(vctItm, sVPth, boost::is_any_of("\n"));
        for (auto itm : vctItm)
        {
            vector<string> vctVP;
            boost::algorithm::split(vctVP, itm, boost::is_any_of("\t"));
            string sUri = boost::algorithm::trim_copy(vctVP[0]);
            if ("/" != sUri) sUri = boost::trim_right_copy_if(sUri, boost::is_any_of("/\\"));
            if (vctVP.size() == 2 && !sUri.empty())
            {
                string sLocal = boost::algorithm::trim_copy(vctVP[1]);
                if ("/" != sLocal) sLocal = boost::trim_right_copy_if(sLocal, boost::is_any_of("/\\"));
                string sLocalFull = boost::filesystem::path(sLocal).is_absolute() ? sLocal
                                    : CUtilFunc::ToAbsPath(sLocal, CUtilFunc::PCharSafeToStr(GetContext().GetMain().GetApPath()));
                auto it = m_vPath.find(sUri);
                if (m_vPath.end() == it && !sLocal.empty())
                    m_vPath.insert(make_pair(sUri, sLocalFull));
            }
        }
        return sVPth;
    }())
    , m_srvCB(*new CWSProxyServerCB(*this)), m_load(m_srvCB), m_proxy(m_load.Proxy())
{
    string sMsg = (boost::format("MainExeModRoot: \t%s \nPlatformRoot: \t%s \nWebsiteRoot: \t%s \nKnewcodeRoot: \t%s \nDocumentRoot: \t%s \nKnewcodeCfgFile: \t%s\n\nVirtualPath: \t%s\n")
                   % m_WebSrvPath % m_PlatformPath % m_WebsitePath % m_FxPath % m_PgPath % m_CfgFile % m_virtualPath
                   ).str();
    cout << sMsg << endl;
    m_proxy.Init();
    m_proxy.GetContext().WriteLogDebug(sMsg.c_str(), __CURR_CODE_PLACE_C__);
}
CApiWork::~CApiWork(void)
{
    m_proxy.Free();
    delete &m_srvCB;
}

// 处理请求
void CApiWork::Work(intptr_t r)
{
    CWSProxyRequestCB reqCB(*this, m_proxy, r);
    m_proxy.Work(reqCB);
}

// 判断是否ssl
bool CApiWork::IsSSL(intptr_t c)
{
    return GetConnInfo(c, "ProtocolType") == string("https");
}

////////////////////////////////////////////////// 回调函数指针 //////////////////////////////////////////////////
// 设置回调函数指针
int CApiWork::SetCBFuncPointer(int argCount, string flag, void* func)
{
    bool bSucc = true;
    if ("FGetSrvInfoCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fGetSrvInfoCB, 3, argCount, flag, func);

    else if ("FGetConnInfoCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fGetConnInfoCB, 4, argCount, flag, func);

    else if ("FGetRequestHeadCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fGetRequestHeadCB, 4, argCount, flag, func);
    else if ("FGetRequestBodyCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fGetRequestBodyCB, 3, argCount, flag, func);
    else if ("FGetRequestOtherCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fGetRequestOtherCB, 4, argCount, flag, func);
    else if ("FGetRequestConnCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fGetRequestConnCB, 3, argCount, flag, func);

    else if ("FSetRespondHeadCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetRespondHeadCB, 5, argCount, flag, func);
    else if ("FSetRespondBodyCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetRespondBodyCB, 5, argCount, flag, func);
    else if ("FSetRespondOtherCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetRespondOtherCB, 5, argCount, flag, func);
    else if ("FSetCommitResponseCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetCommitResponseCB, 3, argCount, flag, func);

    else if ("FSetIsDisconnByErrCodeCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetIsDisconnByErrCodeCB, 3, argCount, flag, func);
    else if ("FSetGetStatusByErrCodeCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetGetStatusByErrCodeCB, 3, argCount, flag, func);

    else if ("FSetKeepaliveCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetKeepaliveCB, 4, argCount, flag, func);
    else if ("FSetBreakConnCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSetBreakConnCB, 5, argCount, flag, func);
    else if ("FCloseConnCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fCloseConnCB, 3, argCount, flag, func);
    else if ("FIsCloseConnCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fIsCloseConnCB, 3, argCount, flag, func);
    else if ("FResetConnTimeCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fResetConnTimeCB, 4, argCount, flag, func);
    else if ("FSendByConnCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSendByConnCB, 5, argCount, flag, func);
    else if ("FSockSendByConnCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fSockSendByConnCB, 5, argCount, flag, func);
    else if ("FWscRecvByConnCB" == flag)
        bSucc = SetCBFuncPointer(CApiWork::s_fWscRecvByConnCB, 5, argCount, flag, func);

    else bSucc = false;
    return bSucc ? 0 : 1;
}
// 检查重要的回调函数是否设置
bool CApiWork::CheckVitalCBFuncPtr(void)
{
    return nullptr != s_fGetSrvInfoCB && nullptr != s_fGetConnInfoCB
        && nullptr != s_fGetRequestHeadCB && nullptr != s_fGetRequestBodyCB
        && nullptr != s_fGetRequestOtherCB && nullptr != s_fGetRequestConnCB
        && nullptr != s_fSetRespondHeadCB && nullptr != s_fSetRespondBodyCB
        && nullptr != s_fSetCommitResponseCB && nullptr != s_fSetRespondOtherCB
        && nullptr != s_fSetIsDisconnByErrCodeCB && nullptr != s_fSetGetStatusByErrCodeCB
    ;
}

// 获取服务器配置信息
FGetSrvInfoCB CApiWork::s_fGetSrvInfoCB = nullptr;
const char* CApiWork::GetSrvInfo(const char* name)
{
    try
    {
        if (nullptr == s_fGetSrvInfoCB) return "";
        const char* result = s_fGetSrvInfoCB(3, "FGetSrvInfoCB", name);
        cout << "*[knewcode] FGetSrvInfoCB: " << name << " = " << CUtilFunc::PCharSafeToPChar(result) << endl;
        return result;
    }
    catch (...) {}
    return nullptr;
}

// 获取链接信息
FGetConnInfoCB CApiWork::s_fGetConnInfoCB = nullptr;
const char* CApiWork::GetConnInfo(intptr_t hConn, const char* name)
{
    try
    {
        if (nullptr == s_fGetConnInfoCB) return "";
        const char* result =  s_fGetConnInfoCB(4, "FGetConnInfoCB", hConn, name);
        cout << "*[knewcode] FGetConnInfoCB: " << name << " = " << CUtilFunc::PCharSafeToPChar(result) << endl;
        return result;
    }
    catch (...) {}
    return nullptr;
}

// 获取请求头信息
FGetRequestHeadCB CApiWork::s_fGetRequestHeadCB = nullptr;
const char* CApiWork::GetRequestHead(intptr_t hRequest, const char* name)
{
    try
    {
        if (nullptr == s_fGetRequestHeadCB) return "";
        const char* result = s_fGetRequestHeadCB(4, "FGetRequestHeadCB", hRequest, name);
        cout << "*[knewcode] FGetRequestHeadCB: " << name << " = " << CUtilFunc::PCharSafeToPChar(result) << endl;
        return result;
    }
    catch (...) {}
    return nullptr;
}
// 获取请求内容
FGetRequestBodyCB CApiWork::s_fGetRequestBodyCB = nullptr;
std::tuple<unsigned, const char*> CApiWork::GetRequestBody(intptr_t re)
{
    try
    {
        if (nullptr == s_fGetRequestBodyCB) return std::make_tuple(0, "");
        // boost::this_thread::sleep(boost::posix_time::milliseconds(6));
        const int iLen = atoi(GetRequestHead(re, c_WebHeader_ContentLength));
        string sContentType = CUtilFunc::PCharSafeToStr(GetRequestHead(re, c_WebHeader_ContentType));
        const char* pBody = CUtilFunc::PCharSafeToPChar(s_fGetRequestBodyCB(3, "FGetRequestBodyCB", re));
        // 判断是否字符串类型
        auto fCheckIfStr = [&]()
        {
            return sContentType.find("application/x-www-form-urlencoded‌") != string::npos
                || sContentType.find(c_DefaultResponseContentType) != string::npos
                || sContentType.find("text/") != string::npos
                ;
        };
        // 是json类型，并且获取到的数据为空，则再执行几次
        for (int i = 1; i <= 3 && fCheckIfStr() && strlen(pBody) == 0; ++i)
        {
            cout << "\t" << i << ". GetRequestBody - ." << pBody << ". \t - " << __CURR_CODE_PLACE_C__ << endl;
            boost::this_thread::sleep(boost::posix_time::milliseconds((int)std::pow(8, i)));
            pBody = CUtilFunc::PCharSafeToPChar(s_fGetRequestBodyCB(3, "FGetRequestBodyCB", re));
        }
        cout << "*[knewcode] FGetRequestBodyCB: " << sContentType << " - " << strlen(pBody) << " - ." << pBody << "." << endl;
        return std::make_tuple(iLen, pBody);
    }
    catch (...) {}
    return std::make_tuple(0, "");
}
// 获取请其他求信息（get参数等）
FGetRequestOtherCB CApiWork::s_fGetRequestOtherCB = nullptr;
const char* CApiWork::GetRequestOther(intptr_t hRequest, const char* name)
{
    try
    {
        if (nullptr == s_fGetRequestOtherCB) return "";
        const char* result = s_fGetRequestOtherCB(4, "FGetRequestOtherCB", hRequest, name);
        cout << "*[knewcode] FGetRequestOtherCB: " << name << " = " << CUtilFunc::PCharSafeToPChar(result) << endl;
        return result;
    }
    catch (...) {}
    return nullptr;
}
// 获取请求的链接句柄
FGetRequestConnCB CApiWork::s_fGetRequestConnCB = nullptr;
intptr_t CApiWork::GetRequestConn(intptr_t hRequest)
{
    try
    {
        if (nullptr == s_fGetRequestConnCB) return 0;
        return s_fGetRequestConnCB(3, "FGetRequestConnCB", hRequest);
    }
    catch (...) {}
    return 0;
}

// 设置应答头信息
FSetRespondHeadCB CApiWork::s_fSetRespondHeadCB = nullptr;
int CApiWork::SetRespondHead(intptr_t hRequest, const char* name, string val)
{
    try
    {
        if (nullptr == s_fSetRespondHeadCB) return 1;
        cout << "*[knewcode] FSetRespondHeadCB: " << name << " = " << val << endl;
        return s_fSetRespondHeadCB(5, "FSetRespondHeadCB", hRequest, name, val.c_str());
    }
    catch (...) {}
    return 1;
}
// 设置应答内容
FSetRespondBodyCB CApiWork::s_fSetRespondBodyCB = nullptr;
int CApiWork::SetRespondBody(intptr_t hRequest, string content)
{
    try
    {
        if (nullptr == s_fSetRespondBodyCB) return 1;
        return s_fSetRespondBodyCB(5, "FSetRespondBodyCB", hRequest, content.c_str(), static_cast<int>(content.size()));
    }
    catch (...) {}
    return 1;
}
// 设置其他应答信息
FSetRespondOtherCB CApiWork::s_fSetRespondOtherCB = nullptr;
int CApiWork::SetRespondOther(intptr_t hRequest, const char* name, string val)
{
    try
    {
        if (nullptr == s_fSetRespondOtherCB) return 1;
        cout << "*[knewcode] FSetRespondOtherCB: " << name << " = " << val << endl;
        return s_fSetRespondOtherCB(5, "FSetRespondOtherCB", hRequest, name, val.c_str());
    }
    catch (...) {}
    return 1;
}
// 提交应答
FSetCommitResponseCB CApiWork::s_fSetCommitResponseCB = nullptr;
int CApiWork::CommitResponse(intptr_t hRequest)
{
    try
    {
        if (nullptr == s_fSetCommitResponseCB) return 1;
        return s_fSetCommitResponseCB(3, "FSetCommitResponseCB", hRequest);
    }
    catch (...) {}
    return 1;
}

// 通过错误码判断是否掉线
FSetIsDisconnByErrCodeCB CApiWork::s_fSetIsDisconnByErrCodeCB = nullptr;
bool CApiWork::IsDisconnByErrCode(int errCode)
{
    try
    {
        if (nullptr == s_fSetIsDisconnByErrCodeCB) return false;
        return s_fSetIsDisconnByErrCodeCB(3, "FSetIsDisconnByErrCodeCB", errCode);
    }
    catch (...) {}
    return false;
}
// 通过错误码得到错误信息
FSetGetStatusByErrCodeCB CApiWork::s_fSetGetStatusByErrCodeCB = nullptr;
const char* CApiWork::GetStatusByErrCode(int errCode)
{
    try
    {
        if (nullptr == s_fSetGetStatusByErrCodeCB) return "";
        return CUtilFunc::PCharSafeToPChar(s_fSetGetStatusByErrCodeCB(3, "FSetGetStatusByErrCodeCB", errCode));
    }
    catch (...) {}
    return "";
}

// 设置链接为长连接
FSetKeepaliveCB CApiWork::s_fSetKeepaliveCB = nullptr;
int CApiWork::SetKeepalive(intptr_t hConn, bool enable)
{
    try
    {
        if (nullptr == s_fSetKeepaliveCB) return 1;
        return s_fSetKeepaliveCB(4, "FSetKeepaliveCB", hConn, enable);
    }
    catch (...) {}
    return 1;
}
// 设置断开的回调函数
FSetBreakConnCB CApiWork::s_fSetBreakConnCB = nullptr;
int CApiWork::SetBreakConnCB(intptr_t hConn, intptr_t hInstance, FBreakConnCBFromOwn fcb)
{
    try
    {
        if (nullptr == s_fSetBreakConnCB) return 1;
        return s_fSetBreakConnCB(5, "FSetBreakConnCB", hConn, hInstance, fcb);
    }
    catch (...) {}
    return 1;
}
// 关闭连接
FCloseConnCB CApiWork::s_fCloseConnCB = nullptr;
int CApiWork::CloseConn(intptr_t hConn)
{
    try
    {
        if (nullptr == s_fCloseConnCB) return 1;
        return s_fCloseConnCB(3, "FCloseConnCB", hConn);
    }
    catch (...) {}
    return 1;
}
// 得到是否连接
FIsCloseConnCB CApiWork::s_fIsCloseConnCB = nullptr;
bool CApiWork::IsCloseConn(intptr_t hConn)
{
    try
    {
        if (nullptr == s_fIsCloseConnCB) return 1;
        return s_fIsCloseConnCB(3, "FIsCloseConnCB", hConn);
    }
    catch (...) {}
    return 1;
}
// 重置链接时间
FResetConnTimeCB CApiWork::s_fResetConnTimeCB = nullptr;
int CApiWork::ResetConnTime(intptr_t hRequest, unsigned ms)
{
    try
    {
        if (nullptr == s_fResetConnTimeCB) return 1;
        return s_fResetConnTimeCB(4, "FResetConnTimeCB", hRequest, ms);
    }
    catch (...) {}
    return 1;
}
// 通过链接发送数据
FSendByConnCB CApiWork::s_fSendByConnCB = nullptr;
int CApiWork::SendByConn(intptr_t hRequest, const char* buf, unsigned len)
{
    try
    {
        if (nullptr == s_fSendByConnCB) return 1;
        return s_fSendByConnCB(5, "FSendByConnCB", hRequest, buf, len);
    }
    catch (...) {}
    return 1;
}
// 通过链接进行Socket发送数据
FSockSendByConnCB CApiWork::s_fSockSendByConnCB = nullptr;
int CApiWork::SockSendByConn(intptr_t hConn, const char* buf, unsigned len)
{
    try
    {
        if (nullptr == s_fSockSendByConnCB) return 1;
        return s_fSockSendByConnCB(5, "FSockSendByConnCB", hConn, buf, len);
    }
    catch (...) {}
    return 1;
}
// 通过链接进行websocket接收数据
FWscRecvByConnCB CApiWork::s_fWscRecvByConnCB = nullptr;
int CApiWork::WscRecvByConn(intptr_t hRequest, char* buf, unsigned len)
{
    try
    {
        if (nullptr == s_fWscRecvByConnCB) return 1;
        return s_fWscRecvByConnCB(5, "FWscRecvByConnCB", hRequest, buf, len);
    }
    catch (...) {}
    return 1;
}
