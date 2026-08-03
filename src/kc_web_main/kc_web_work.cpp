#include "kc_web_work.h"
#include "request_respond.h"

////////////////////////////////////////////////////////////////////////////////
// CKCWebWork类
CKCWebWork::CKCWebWork(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle), m_WebApiRef(*m_context.takeServiceReference(c_KCWebApiWorkSrvGUID))
{
}

CKCWebWork::~CKCWebWork()
{
}

// 初始化/释放
bool CKCWebWork::init(void)
{
    string sCfgMod = string("Config.Modules.") + m_bundle.getName();
    // 添加响应头
    auto fAddHeader = [&](string key, string val)
    {
        if (!key.empty() /*&& !val.empty()*/)
        {
            auto iter = m_CfgHeader.find(key);
            if (m_CfgHeader.end() == iter)
                m_CfgHeader.insert(make_pair(key, val));
            else iter->second = val;
        }
    };
    // 版本信息
    fAddHeader("Knewcode-Api-Ver", m_context.VersionInfo());
    fAddHeader("Server-Api-Ext", c_DefaultWorkUriExtension);
    // 配置中的响应头信息
    string sHeaderNode = sCfgMod + ".Header";
    for (int i = 0, c = m_context.GetCfgSubCount(sHeaderNode.c_str()); i < c; ++i)
        if (m_context.IsCfgSubValid(sHeaderNode.c_str(), i))
            fAddHeader(m_context.GetCfgSubInfo(sHeaderNode.c_str(), i, "key", ""), m_context.GetCfgSubInfo(sHeaderNode.c_str(), i, "value", ""));
    return true;
}
bool CKCWebWork::free(void)
{
    IServiceReference *srvRef = &m_WebApiRef;
    m_context.freeServiceReference(srvRef);
    return true;
}

// 得到服务特征码
const char* CKCWebWork::getGUID(void) const
{
    return c_KCWebWorkSrvGUID;
}

// 对应的模块
const IBundle& CKCWebWork::getBundle(void) const
{
    return m_bundle;
}

// 处理请求
int CKCWebWork::RenderKC(ISrcRequestRespond& re)
{
    int iErrCode = -1;
    // 创建页面数据
    CKCRequestRespond reKc(re, *this);
    // 错误应答
    auto fRespondErr = [&](string sMsg, string sType)
    {
        string sPTID = std::to_string(boost::this_process::get_id()) + ":" + boost::lexical_cast<string>(boost::this_thread::get_id());
        string sErrMsg = "Internal server error. (" + sPTID + " - " + sType + ") " + sMsg;
        reKc.AddResponseBody(CCtrlCommon::GetRespondJson(33, sErrMsg));
    };
    // 提交应答
    CAutoRelease _auto(boost::bind(&CKCRequestRespond::CommitResponse, &reKc));
    try
    {
        // 随机数因子
        time_t t;
        srand((unsigned)time(&t));
        // 设置返回类型
        // re.SetResponseContentType("text/html");
        // 时间
        //posix_time::ptime pt = posix_time::microsec_clock::local_time();
        //re.AddResponseHeader("Date", posix_time::to_iso_string(pt).c_str());
        // 响应头信息
        //if (re.GetResponseContentType() != string(c_WebSocketResponseContentType))
        {
            // 跨域请求的头
            // re.AddResponseHeader("Access-Control-Allow-Origin", re.GetRequestHeader("Origin"));
            // 其他信息
            // re.AddResponseHeader("Clent-IP-Port", re.GetSingleInfo("ClientIpPort"));
            // re.AddResponseHeader("Unique-ConnID", re.GetSingleInfo("UniqueConnID"));

            // 固定的响应头信息
            for (auto &h: m_CfgHeader)
                re.AddResponseHeader(h.first.c_str(), h.second.c_str());
        }
        // 首次运行的日志
        static bool bFirstCall = true;
        if (!re.IsSubCall() && bFirstCall)
        {
            bFirstCall = false;
            this->WriteLogDebug(reKc.GetAllInfo(), __CURR_CODE_PLACE_C__);
        }
        // 处理webapi请求
        IKCWebApiWork& wbApi = dynamic_cast<IServiceReferenceEx*>(&m_WebApiRef)->getServiceSafe<IKCWebApiWork>();
        iErrCode = wbApi.RenderREST(reKc);
        // 输入参数写入日志
        // if (m_context.GetCfgLogLevel() <= 0)
        //     this->WriteLogTrace((boost::format("👀 %s ~=> %s\n%s\n") % re.GetSingleInfo("client_host_port") % re.GetSingleInfo("unparsed_uri") % re.GetPostArgStr()).str().c_str(), __CURR_CODE_PLACE_C__);
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        ex.OtherInfo() = re.GetLocalFilename();
        TLogInfo log(ex);
        log.m_place = __CURR_CODE_PLACE_C__;
        this->WriteLog(log);
        fRespondErr(ex.CurrPosInfo() + ex.what(), typeid(ex).name());
    }
    catch(std::exception& e)
    {
        TException ex(iErrCode, __CURR_CODE_PLACE_C__, e.what());
        ex.LineCode() = __LINE__;
        ex.OtherInfo() = re.GetLocalFilename();
        ex.ExceptType() = typeid(e).name();
        this->WriteLog(TLogInfo(ex));
        fRespondErr(ex.what(), typeid(ex).name());
    }
	catch(const char* e)
	{
        TException ex(iErrCode, __CURR_CODE_PLACE_C__, e);
        ex.LineCode() = __LINE__;
        ex.OtherInfo() = re.GetLocalFilename();
        ex.ExceptType() = "const char*";
        this->WriteLog(TLogInfo(ex));
        fRespondErr(ex.what(), "char");
    }
	catch(int e)
	{
        TException ex(iErrCode, __CURR_CODE_PLACE_C__, std::to_string(e).c_str());
        ex.LineCode() = __LINE__;
        ex.OtherInfo() = re.GetLocalFilename();
        ex.ExceptType() = "int";
        this->WriteLog(TLogInfo(ex));
        fRespondErr(ex.what(), "int");
    }
    catch(...)
    {
        this->WriteLogError(this->getHint("Unknown_exception"), __CURR_CODE_PLACE_C__, re.GetLocalFilename());
        fRespondErr(re.GetLocalFilename(), "Unknown");
    }
    return iErrCode;
}

// 静态页面处理
void CKCWebWork::StaticPage(ISrcRequestRespond& re)
{
    try
    {
        // 固定的响应头信息
        for (auto &h: m_CfgHeader)
            re.AddResponseHeader(h.first.c_str(), h.second.c_str());
    }
    catch (...) {}
}

// 固定响应类型字符串
const char* CKCWebWork::FixContentTypeString(const char* ct)
{
    for (auto it = m_ResponseContentTypes.begin(); it != m_ResponseContentTypes.end(); ++it)
        if (*it == ct) return it->c_str();
    m_ResponseContentTypes.push_back(string(ct));
    return m_ResponseContentTypes.rbegin()->c_str();
}
