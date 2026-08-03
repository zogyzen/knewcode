#include "request_respond.h"
#include "kc_web_work.h"

////////////////////////////////////////////////////////////////////////////////
// TMultiFormData
CKCRequestRespond::TMultiFormData::TMultiFormData(string sHeader, string sBody) : m_body(sBody)
{
    string sLine;
    stringstream ssHeader(sHeader);
    while (std::getline(ssHeader, sLine))
    {
        vector<string> varsVec;
        algorithm::split(varsVec, sLine, is_any_of(";"));
        for (auto var : varsVec)
        {
            /*
            vector<string> varVec;
            algorithm::split(varVec, trim_copy(var), is_any_of(":="), token_compress_on);
            if (varVec.size() > 1)
                m_header.insert(make_pair(erase_all_copy(trim_copy(varVec[0]), "-"),
                                trim_copy(trim_copy_if(trim_copy(varVec[1]), is_any_of("\"")))));
            */
            string sName = trim_copy(var);
            string sVal = "";
            auto iPos = sName.find(":");
            if (string::npos != iPos || string::npos != (iPos = sName.find("=")))
            {
                sVal = sName.substr(iPos + 1);
                sName = sName.substr(0, iPos);
                if ("name" == sName) sName = "FormDataName";
                if ("filename" == sName) m_isFile = true;
            }
            //cout << sName << "\t" << sVal << endl;
            m_header.insert(make_pair(erase_all_copy(trim_copy(sName), "-"),
                            trim_copy(trim_copy_if(trim_copy(sVal), is_any_of("\"")))));
        }
    }
}

CKCRequestRespond::TMultiFormData::~TMultiFormData()
{
}

// 得到头
const char* CKCRequestRespond::TMultiFormData::GetHeader(const char* sHeaderName)
{
    auto it = m_header.find(sHeaderName);
    if (m_header.end() != it) return it->second.c_str();
    else return "";
}

// 得到体
const char* CKCRequestRespond::TMultiFormData::GetBody(unsigned& iLen)
{
    iLen = static_cast<unsigned>(m_body.size());
    return m_body.c_str();
}

// 是否文件
bool CKCRequestRespond::TMultiFormData::isFile(void)
{
    return m_isFile;
}

////////////////////////////////////////////////////////////////////////////////
// CKCRequestRespond类
CKCRequestRespond::CKCRequestRespond(ISrcRequestRespond& re, IKCWebWork& ww)
    : m_runIndex(++m_runCount), m_re(re), m_webWork(ww)
{
    // 开始日志
    string sConnHandle =  CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("connection_handle")),
           sConnID = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("connection_id")),
           sUserAddr = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("useragent_addr")),
           sCookie = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("Cookie")),
           sGet = CUtilFunc::PCharSafeToStr(m_re.GetGetArgStr()),
           sContentType = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo(c_WebHeader_ContentType)),
           sClnHost = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("client_host_port")),
           sSrvHost = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("server_host_port")),
           sSrvHost2 = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("Host")),
           sKeepalive = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("connection_keepalive")),
           sKeepalives = CUtilFunc::PCharSafeToStr(m_re.GetSingleInfo("connection_keepalives"))
        ;
    string sBody = CUtilFunc::PCharSafeToStr(re.GetPostArgStr());
    if (!re.IsSubCall())
    {
        m_webWork.WriteLogDebug((std::to_string(m_runIndex) + ". " + m_webWork.getHint("Reander_page_") + m_re.GetSingleInfo()).c_str(), __CURR_CODE_PLACE_C__,
                           (format("👀 0x%X - [%s - %s - %s - %s - %s] - %s - [%s -> %s(%s)] - %s\nGET: %s\nCookie: %s\n%s\n")
                                % this % sConnID % sConnHandle % sUserAddr % sKeepalive % sKeepalives
                                % boost::lexical_cast<string>(boost::this_thread::get_id()) % sClnHost % sSrvHost % sSrvHost2 % sContentType
                                % sGet % sCookie
                                % sBody.substr(0, 1024)
                            ).str().c_str());
        cout << ">>>>>>>>>>>>>>>> " << m_runIndex << endl;
    }
    // GET参数
    CUtilFunc::splitUrlGetParm(m_getParms, sGet);
    // Cookie
    if (!sCookie.empty())
    {
        vector<string> cookiesVec;
        split(cookiesVec, sCookie, is_any_of(";"));
        for (string ck : cookiesVec)
        {
            // algorithm::trim(ck);
            // vector<string> cookieVec;
            // algorithm::split(cookieVec, ck, is_any_of("="));
            // if (cookieVec.size() > 1)
            //     m_cookies.insert(make_pair(cookieVec[0], cookieVec[1]));
            auto prCk = CUtilFunc::splitKeyValue(boost::trim_left_copy(ck));
            if (m_cookies.find(prCk.first) == m_cookies.end())
                m_cookies.insert(prCk);
        }
    }
    // multipart/form-data
    if (!sContentType.empty())
    {
        vector<string> frmDaVec;
        split(frmDaVec, sContentType, is_any_of(";"));
        // 请求类型是多表单类型
        if (frmDaVec.size() > 1 && (m_IsMultipartFormData = "multipart/form-data" == to_lower_copy(trim_copy(frmDaVec[0]))))
        {
            // 取多表单的分隔字符串
            vector<string> boundaryVec;
            split(boundaryVec, trim_copy(frmDaVec[1]), is_any_of("="));
            if (boundaryVec.size() > 1 && "boundary" == to_lower_copy(trim_copy(boundaryVec[0])))
            {
                // 多表单的分隔字符串
                m_multiFormDataBoundary = boundaryVec[1];
                trim_if(m_multiFormDataBoundary, is_any_of("\""));
                // post缓冲区
                int iPostLen = GetPostArgLength();
                char *pPostBuf = nullptr;
                GetPostArgBuffer(pPostBuf, iPostLen);
                string sPostBuf(pPostBuf, iPostLen);
                // 头、尾、中间分隔的字符串
                string strStart = "--" + m_multiFormDataBoundary + "\r\n";
                string strEnd = "\r\n--" + m_multiFormDataBoundary + "--\r\n";
                string strSplit = "\r\n--" + m_multiFormDataBoundary + "\r\n";
                // 去掉头尾
                if (algorithm::starts_with(sPostBuf, strStart)) replace_first(sPostBuf, strStart, "");
                if (algorithm::ends_with(sPostBuf, strEnd)) replace_last(sPostBuf, strEnd, "");
                // 分割
                vector<string> multiVec;
                //algorithm::split_regex(multiVec, sPostBuf, boost::regex("(\r\n)?--" + m_multiFormDataBoundary + "(--|\r\n)"));
                algorithm::split_regex(multiVec, sPostBuf, boost::regex(strSplit));
                for (auto da: multiVec)
                {
                    unsigned iPos = static_cast<int>(da.find("\r\n\r\n"));
                    if (string::npos != iPos)
                        m_multiFormData.push_back(boost::shared_ptr<TMultiFormData>(new TMultiFormData(da.substr(0, iPos), da.substr(iPos + 4))));
                }
            }
            else m_webWork.WriteLogError("multipart/form-data Format Error!!", __CURR_CODE_PLACE_C__, frmDaVec[1].c_str());
        }
        if (m_IsMultipartFormData)
            m_webWork.WriteLogInfo("multipart/form-data", __CURR_CODE_PLACE_C__, (format("[%s][%d]") % m_multiFormDataBoundary % m_multiFormData.size()).str().c_str());
    }
}

CKCRequestRespond::~CKCRequestRespond()
{
    // 结束日志
    if (!m_re.IsSubCall())
    {
        m_webWork.WriteLogDebug((std::to_string(m_runIndex) + ". " + m_webWork.getHint("Reander_page_end_") + m_re.GetSingleInfo()).c_str(), __CURR_CODE_PLACE_C__,
                           (format("0x%X - [%s] - %s") % this % m_re.GetSingleInfo("connection_id") % m_re.GetSingleInfo("connection_client_ip")).str().c_str());
        cout << "<<<<<<<<<<<<<<<< " << m_runIndex << endl;
    }
    // 资源释放
    this->Release();
}

// 服务接口
IKCWebWork& CKCRequestRespond::GetWork(void)
{
    return m_webWork;
}

// 释放
void CKCRequestRespond::Release(void)
{
    try
    {
        // 释放缓冲数据
        TReStepDataPtrList sdFreeList;
        sdFreeList.swap(m_ReStepDataPtrList);
        m_ReStepDataPtrList.clear();
        m_multiFormData.clear();
        sdFreeList.clear();
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        ex.OtherInfo() = this->GetLocalFilename();
        TLogInfo log(ex);
        log.m_place = __CURR_CODE_PLACE_C__;
        m_webWork.WriteLog(log);
    }
    catch(...)
    {
        m_webWork.WriteLogError(m_webWork.getHint("Unknown_exception"), __CURR_CODE_PLACE_C__, this->GetLocalFilename());
    }
}

///////////////////////////////////////////////////////////////
ISrcRequestRespond& CKCRequestRespond::getRe(void) const
{
    return m_re;
}

// 锁
IKcLockWork& CKCRequestRespond::LockWork(void)
{
    return m_webWork.getContext().LockWork();
}

// 超时时间
int CKCRequestRespond::GetTimeOutSeconds(void) const
{
    return m_webWork.getContext().GetTimeOutSeconds();
}

// 执行中的页面数据接口
IActionData* CKCRequestRespond::GetActionData(void)
{
    return m_ActionData;
}

void CKCRequestRespond::SetActionData(IActionData* act)
{
    m_ActionData = act;
}

IActionData& CKCRequestRespond::ActionData(void)
{
    if (nullptr == m_ActionData)
        throw TKCWebWorkException(ecd_ErrCode_KCWebMain_NoAct, __CURR_CODE_PLACE_C__, string(m_webWork.getHint("Action_page_")) + m_re.GetLocalFilename(), m_webWork);
    return *m_ActionData;
}

////////////////////////////请求部分///////////////////////////
// 得到服务器端全部信息
const char* CKCRequestRespond::GetAllInfo(const char* endTag)
{
    static thread_local string sAllInfo;
    sAllInfo = m_re.GetAllInfo(endTag);
    sAllInfo += c_RESTful_MultiFormDataCount + string(": ") + CUtilFunc::PCharSafeToStr(GetSingleInfo(c_RESTful_MultiFormDataCount)) + endTag;
    sAllInfo += c_RESTful_MultiFormDataPos + string(": ") + CUtilFunc::PCharSafeToStr(GetSingleInfo(c_RESTful_MultiFormDataPos)) + endTag;
    for (auto fm : m_multiFormData)
        for (auto h : fm->m_header)
            sAllInfo += h.first + ": " + h.second + endTag;
    sAllInfo += c_RESTful_RootActUri + string(": ") + CUtilFunc::PCharSafeToStr(GetSingleInfo(c_RESTful_RootActUri)) + endTag;
    return sAllInfo.c_str();
}

// 得到服务器端各单个请求信息
const char* CKCRequestRespond::GetSingleInfo(const char* pName, const char* pDef)
{
    try
    {
        static thread_local string sResult;
        sResult.clear();
        string sName = CUtilFunc::PCharSafeToStr(pName);
        if (c_RESTful_MultiFormDataCount == sName)
        {
            sResult = std::to_string(m_multiFormData.size());
            return sResult.c_str();
        }
        else if (c_RESTful_MultiFormDataPos == sName)
        {
            sResult = std::to_string(m_posMultiFormData);
            return sResult.c_str();
        }
        else if (m_multiFormData.size() > m_posMultiFormData)
        {
            auto &frmHeader = m_multiFormData[m_posMultiFormData]->m_header;
            {
                auto it = frmHeader.find(sName);
                if (frmHeader.end() != it)
                {
                    cout << it->second << endl;
                    return it->second.c_str();
                }
            }
        }
        else if (c_RESTful_RootActUri == sName)
        {
            string sUri = m_re.GetSingleInfo("parsed_uri_path");
            const char* pAct = GetGetArg(c_RESTful_act, nullptr);
            if (nullptr == pAct || strlen(pAct) == 0)
                pAct = c_RESTful_main;
            sResult = sUri + "?act=" + pAct;
            return sResult.c_str();
        }
        return m_re.GetSingleInfo(pName, pDef);
    }
    catch (...) {}
    return pDef;
}

// 得到本地完整文件名
const char* CKCRequestRespond::GetLocalFilename(void)
{
    const char* sLocal = CUtilFunc::PCharSafeToPChar(m_re.GetLocalFilename());
    string sUri = CUtilFunc::PCharSafeToStr(m_re.GetUriFilename());
    m_webWork.WriteLogTrace((string(m_webWork.getHint("Get_full_name_of_the_local_file_")) + sUri).c_str(), __CURR_CODE_PLACE_C__, sLocal);
    if (!boost::filesystem::exists(sLocal))
    {
        string sErr = string(m_webWork.getHint("Don_t_exists_file_")) + sUri;
        cout << "Don't Exists File - " << sLocal << endl;
        m_webWork.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, (sLocal + string("\n")).c_str());
        throw TKCWebWorkException(ecd_ErrCode_KCWebMain_NoKCFile, __CURR_CODE_PLACE_C__, sErr, m_webWork);
    }
    return sLocal;
}
const char* CKCRequestRespond::GetLocalFilename(const char* uri)
{
    const char* sLocal = CUtilFunc::PCharSafeToPChar(GetLocalPath(uri));
    if (!boost::filesystem::exists(sLocal) && !boost::filesystem::exists(sLocal + string(c_so_ext_name)))
    {
        string sErr = string(m_webWork.getHint("Don_t_exists_file_")) + uri;
        m_webWork.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, sLocal);
        throw TKCWebWorkException(ecd_ErrCode_KCWebMain_NoKCFile, __CURR_CODE_PLACE_C__, sErr, m_webWork);
    }
    return sLocal;
}

// 规范化url路径（去除“..”和“.”）
const char* CKCRequestRespond::CanonicalUrl(const char* uri)
{
    //sSubFileUrl = boost::filesystem::canonical(sSubFileUrl).string();
    return uri;
}

// 得到网络文件名
const char* CKCRequestRespond::GetUriFilename(void)
{
    return m_re.GetUriFilename();
}
// 得到网络路径
const char* CKCRequestRespond::GetUrlPagePath(void)
{
    return m_re.GetUrlPagePath();
}
// 得到网络根路径
const char* CKCRequestRespond::GetUrlPageRootPath(void)
{
    return m_re.GetUrlPageRootPath();
}
const char* CKCRequestRespond::GetUrlPageRootPath(const char* uri)
{
    return m_re.GetUrlPageRootPath(uri);
}

// 协议
const char* CKCRequestRespond::GetProtocol(void)
{
    m_Protocol = m_re.GetSingleInfo("protocol");
    vector<string> vecSegTag;
    boost::split(vecSegTag, m_Protocol, boost::is_any_of("/"));
    if (!vecSegTag.empty()) m_Protocol = vecSegTag[0];
    return m_Protocol.c_str();
}

// 端口
int CKCRequestRespond::GetPort(void)
{
    return m_re.GetPort();
}

// GET的参数
const char* CKCRequestRespond::GetGetArgStr(void)
{
    return m_re.GetGetArgStr();
}
const char* CKCRequestRespond::GetGetArg(const char* sName, const char* sDef)
{
    auto it = m_getParms.find(sName);
    if (m_getParms.end() != it) return it->second.c_str();
    else return sDef;
}

// Post的参数
const char* CKCRequestRespond::GetPostArgType(void)
{
    return m_re.GetPostArgType();
}
int CKCRequestRespond::GetPostArgLength(void)
{
    return m_re.GetPostArgLength();
}
int CKCRequestRespond::GetPostArgBuffer(char*& buf, int len)
{
    return m_re.GetPostArgBuffer(buf, len);
}
const char* CKCRequestRespond::GetPostArgStr(void)
{
    return m_re.GetPostArgStr();
}

// multipart/form-data参数
bool CKCRequestRespond::IsMultipartFormData(void)
{
    return m_IsMultipartFormData;
}
unsigned CKCRequestRespond::GetMultiFormDataCount(void)
{
    return static_cast<unsigned>(m_multiFormData.size());
}
IMultiFormData& CKCRequestRespond::GetMultiFormData(void)
{
    if (m_multiFormData.size() <= m_posMultiFormData)
        throw TKCWebWorkException(ecd_ErrCode_KCWebMain_MFD_SizeWrong, __CURR_CODE_PLACE_C__, string(m_webWork.getHint("Array_index_out_of_bounds")) + (format(" - %d/%d") % m_posMultiFormData % m_multiFormData.size()).str(), m_webWork);
    return *m_multiFormData[m_posMultiFormData];
}
void CKCRequestRespond::SetCurrentMultiFormData(unsigned i)
{
    m_posMultiFormData = static_cast<unsigned>(max(min(static_cast<int>(i), static_cast<int>(m_multiFormData.size()) - 1), 0));
}

// 得到网站本地完整根目录
const char* CKCRequestRespond::GetLocalRootPath(void)
{
    return m_re.GetLocalRootPath();
}
const char* CKCRequestRespond::GetLocalPath(const char* uri)
{
    const char* sLocal = m_re.GetLocalFilename(uri);
    m_webWork.WriteLogTrace((string(m_webWork.getHint("Get_full_name_of_the_local_file_")) + uri).c_str(), __CURR_CODE_PLACE_C__, sLocal);
    return sLocal;
}

// 得到当前页本地完整根目录
const char* CKCRequestRespond::GetLocalPagePath(void)
{
    boost::filesystem::path _path(m_re.GetLocalFilename());
    m_LocalPagePath = _path.parent_path().string();
    m_LocalPagePath = CUtilFunc::FormatPath(m_LocalPagePath);
    return m_LocalPagePath.c_str();
}

// 按名称得到请求头
const char* CKCRequestRespond::GetRequestHeader(const char* name)
{
    return m_re.GetRequestHeader(name);
}

// 得到Cookie
const char* CKCRequestRespond::GetCookieVal(const char* pName)
{
    string sName(nullptr != pName ? pName : "");
    auto it = m_cookies.find(sName);
    if (m_cookies.end() == it) return "";
    else return it->second.c_str();
}

// 虚拟目录
unsigned CKCRequestRespond::VirtualPathCount(void)
{
    return m_re.VirtualPathCount();
}
const char* CKCRequestRespond::GetVirtualPath(unsigned pos)
{
    return m_re.GetVirtualPath(pos);
}
const char* CKCRequestRespond::GetVirtualPathUri(unsigned pos)
{
    return m_re.GetVirtualPath(pos);
}

////////////////////////////应答部分///////////////////////////
// 响应文本类型
bool CKCRequestRespond::SetResponseContentType(const char* ct)
{
    return m_re.SetResponseContentType(dynamic_cast<CKCWebWork&>(m_webWork).FixContentTypeString(ct));
}
const char* CKCRequestRespond::GetResponseContentType(void)
{
    return m_re.GetResponseContentType();
}

// 响应状态
bool CKCRequestRespond::SetResponseStatus(int iStt)
{
    return m_re.SetResponseStatus(iStt);
}

// 按名称添加响应头
void CKCRequestRespond::AddResponseHeader(const char* name, const char* val)
{
    m_re.AddResponseHeader(name, val);
}
void CKCRequestRespond::DelResponseHeader(const char* name)
{
    m_re.DelResponseHeader(name);
}

// 输出网页内容
bool CKCRequestRespond::AddResponseBody(const char* buf, int nbyte)
{
    return m_re.AddResponseBody(buf, nbyte);
}
// 提交响应（将这段时间，服务器端的输出，同时推给客户端）
void CKCRequestRespond::CommitResponse(void)
{
    // 提交到客户端
    m_re.CommitResponse();
}

// 添加Cookie
bool CKCRequestRespond::AddCookie(const char* name, const char* val, const char* expires, const char* path, const char* domain)
{
    string  sName = nullptr != name ? name : "",
            sVal = nullptr != val ? val : "",
            sExpires = nullptr != expires ? expires : "",
            sPath = nullptr != path ? path : "",
            sDomain = nullptr != domain ? domain : "";
    bool bResult = !sName.empty();
    if (bResult)
    {
        string sCookie = sName + "=" + sVal;
        if (!sExpires.empty())
            //sCookie += "; expires=" + CUtilFunc::ISO2GMT2(sExpires);
            sCookie += "; expires=" + sExpires;
        if (!sPath.empty())
            sCookie += "; path=" + sPath;
        if (!sDomain.empty())
            sCookie += "; domain=" + sDomain;
        m_re.AddResponseHeader("Set-Cookie", sCookie.c_str());
    }
    return bResult;
}

////////////////////////请求过程中的数据///////////////////////
// 判断某名称数据是否存在
bool CKCRequestRespond::ExistsReStepData(const char* name)
{
    return m_ReStepDataPtrList.end() != m_ReStepDataPtrList.find(name);
}

// 得到某名称数据
IKCRequestRespond::IReStepData& CKCRequestRespond::GetReStepData(const char* name)
{
    auto iter = m_ReStepDataPtrList.find(name);
    if (m_ReStepDataPtrList.end() == iter)
        throw TKCWebWorkException(ecd_ErrCode_KCWebMain_NoData, __CURR_CODE_PLACE_C__, string(m_webWork.getHint("Don_t_exists_data_")) + name, m_webWork);
    return *iter->second;
}

// 添加某名称数据
bool CKCRequestRespond::AddReStepData(const char* name, IKCRequestRespond::IReStepData*& data)
{
    if (nullptr == data)
        throw TKCWebWorkException(ecd_ErrCode_KCWebMain_NullPointData, __CURR_CODE_PLACE_C__, string(m_webWork.getHint("Null_Point_Data_")) + name, m_webWork);
    if (!ExistsReStepData(name))
    {
        m_ReStepDataPtrList.insert(make_pair(string(name), TReStepDataPtr(data)));
        if (m_webWork.getContext().GetCfgLogLevel() <= (int)log::trivial::debug)
            m_webWork.WriteLogDebug((string(m_webWork.getHint("Register_Data_")) + name).c_str(), __CURR_CODE_PLACE_C__, typeid(*data).name());
    }
    else
    {
        delete data;
        data = nullptr;
        throw TKCWebWorkException(ecd_ErrCode_KCWebMain_RepeatData, __CURR_CODE_PLACE_C__, string(m_webWork.getHint("Repeat_Register_Data_")) + name, m_webWork);
    }
    return ExistsReStepData(name);
}
