#include "ctrl_sse.h"

// 最新的活动页编号
std::atomic_ullong CCtrlSSE::m_msgMaxID(0);

////////////////////////////////////////////////////////////////////////////////
// CCtrlSSE::TSSEconn 类
CCtrlSSE::TSSEconn::TSSEconn(CCtrlSSE& own, IKCWebLongConn &wc, int kl)
    : m_own(own), m_wc(&wc), m_keepalive_secs(kl)
    , m_wcid(wc.GetID()), m_sid(wc.GetUniqueID()), m_status(ewsProcess)
{
    WC().SetRecvIF(this);
    m_own.m_own.WriteLogDebug("Connect SSE", __CURR_CODE_PLACE_C__, m_sid.c_str());
}
CCtrlSSE::TSSEconn::~TSSEconn(void)
{
    SetDisable();
}

// 接收信息
void CCtrlSSE::TSSEconn::Recv(char* /*buf*/, unsigned /*len*/)
{
    throw TKCSSEWSException(ecd_ErrCode_KCSSEWS + 3, __CURR_CODE_PLACE_C__, "Nonsupport", m_own.m_own);
}

// 断开链接
void CCtrlSSE::TSSEconn::Discon(void)
{
    SetDisable();
    m_own.m_own.WriteLogDebug((string("Discon ") + typeid(*this).name()).c_str(), __CURR_CODE_PLACE_C__, m_sid.c_str());
}

// 错误日志
bool CCtrlSSE::TSSEconn::WriteLogError(const char* msg, const char* pos, const char* oth) const
{
    return m_own.m_own.WriteLogError(msg, pos, oth);
}

// 其他指令
void CCtrlSSE::TSSEconn::Cmd(const char* c, const char* msg)
{
    string sCmd = CUtilFunc::PCharSafeToStr(c);
    if ("Warning" == sCmd)
    {
        string sMsg = (format("event: %s\ndata: %s\n\n") % sCmd % msg).str();
        m_own.SendMxMessage(this_process::get_id(), m_wcid, 91, sMsg);
        m_own.m_own.WriteLogWarning(c, __CURR_CODE_PLACE_C__, msg);
    }
    else
    {
        string sMsg = (format("event: %s\ndata: %s\n\n") % sCmd % msg).str();
        m_own.SendMxMessage(this_process::get_id(), m_wcid, 91, sMsg);
    }
    cout << __CURR_CODE_PLACE_C__ << ": " << sCmd << " - " << msg << endl;
}

// 链接
IKCWebLongConn& CCtrlSSE::TSSEconn::WC(void)
{
    if (nullptr == m_wc)
    {
        m_status = ewsInvalid;
        throw TKCSSEWSException(ecd_ErrCode_KCSSEWS + 4, __CURR_CODE_PLACE_C__, string("nullptr ") + typeid(m_wc).name(), m_own.m_own);
    }
    return *m_wc;
}

// 置为无效
string CCtrlSSE::TSSEconn::SetDisable(void)
{
    try
    {
        m_status = ewsInvalid;
        if (dynamic_cast<IKCWebLongConn*>(m_wc) != nullptr)
        {
            cout << "SetDisable[" << m_sid << "]: " << typeid(*m_wc).name() << endl;
            CallSubCtrl("$cancel");
            m_wc->SetRecvIF();
            boost::this_thread::sleep(boost::posix_time::milliseconds(666));
            m_own.m_own.getContext().GetMain().ReleaseWebConn(*m_wc);
            m_wc = nullptr;
        }
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = m_sid;
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.m_own.WriteLog(ex);
    }
    catch (std::exception& ex)
    {
        string sErr = "[" + m_sid + "] " + ex.what();
        m_own.m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        string sErr = m_own.m_own.getHint("Unknown_exception");
        m_own.m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, m_sid.c_str());
    }
    return m_sid;
}

// 调用子控制器（取消或激活链接）
int CCtrlSSE::TSSEconn::CallSubCtrl(string sName)
{
    if (nullptr != m_wc)
    try
    {
        // 请求响应上下文
        IBaseRequestRespond &reBas = m_wc->MakeRe();
        CAutoRelease _auto2([&](){ m_wc->ReleaseRe(reBas); });
        CRequestRespondPack rePack(dynamic_cast<ISrcRequestRespond&>(reBas));
        // 附加控制器信息
        rePack.AddExSingleInfo("$__KCCtrl_Main_Header__", sName);
        // 调用控制器
        return CCltrFunc::CallCtrl(m_own.m_own.getContext(), rePack);
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = m_sid;
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.m_own.WriteLog(ex);
    }
    catch (std::exception& ex)
    {
        string sErr = "[" + m_sid + "] " + ex.what();
        m_own.m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        string sErr = m_own.m_own.getHint("Unknown_exception");
        m_own.m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, m_sid.c_str());
    }
    return -200;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlSSE类
CCtrlSSE::CCtrlSSE(IKCSSEWS& own, string sName, property_tree::ptree& pt)
    : m_own(own), m_name(sName)
    , m_type(pt.get_child_optional("<xmlattr>.type") ? pt.get<string>("<xmlattr>.type") : "sse")
    , m_flag(pt.get_child_optional("<xmlattr>.Flag") ? pt.get<string>("<xmlattr>.Flag") : "")
    , m_sendMQname(MQname(this_process::get_id()))
    , m_running(Init(pt))
    , m_GuardThrd(&CCtrlSSE::GuardThrd, this)
    , m_MQThrd(&CCtrlSSE::MQThrd, this)
    , m_PingThrd(&CCtrlSSE::PingThrd, this)
{
}

CCtrlSSE::~CCtrlSSE()
{
    try
    {
        m_running = false;
        // 终止ping线程
        if (m_PingThrd.joinable()) m_PingThrd.interrupt();
        if (m_PingThrd.joinable()) m_PingThrd.timed_join(boost::posix_time::milliseconds(666));
        // 终止消息队列线程
        if (m_MQThrd.joinable()) m_MQThrd.interrupt();
        if (m_MQThrd.joinable()) m_MQThrd.timed_join(boost::posix_time::milliseconds(666));
        // 终止守护线程
        if (m_GuardThrd.joinable()) m_GuardThrd.interrupt();
        if (m_GuardThrd.joinable()) m_GuardThrd.timed_join(boost::posix_time::milliseconds(666));
        // 删除发送消息队列
        interprocess::message_queue::remove(m_sendMQname.c_str());
        // 清除链接对象
        m_wcs.clear();
    }
    catch (...) {}
}

// 宿主
IKCSSEWS& CCtrlSSE::own(void)
{
    return m_own;
}

// 消息队列名称
string CCtrlSSE::MQname(int pid)
{
    return (format("_%s_$MQ$_%d_%s_%s_%s") % m_type % pid % m_name % m_own.getContext().BuildDatetime() % m_flag).str();
}

// 初始化
bool CCtrlSSE::Init(property_tree::ptree& pt)
{
    // 中转的session控制器
    if (pt.get_child_optional("<xmlattr>.SessionCtrl"))
        m_sessionCtrl = pt.get<string>("<xmlattr>.SessionCtrl");
    // 是否定时发送ping指令
    if (pt.get_child_optional("<xmlattr>.RunPing"))
        m_runPing = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.RunPing")) != "false";
    // 发送消息队列尺寸
    if (pt.get_child_optional("<xmlattr>.SendMQSize"))
    try
    {
        m_sendMQsize = pt.get<int>("<xmlattr>.SendMQSize");
    }
    catch (...) {}
    if (m_sendMQsize < 8) m_sendMQsize = 8;
    return true;
}

// session接口
IKCSessionCookie& CCtrlSSE::Session(IServiceReference*& pRef)
{
    // session插件服务接口
    pRef = m_own.getContext().takeServiceReference(c_KCSessionCookieSrvGUID);
    IServiceReferenceEx *pSessionJK = dynamic_cast<IServiceReferenceEx*>(pRef);
    if (nullptr == pSessionJK)
        throw TKCSSEWSException(ecd_ErrCode_KCSSEWS + 5, __CURR_CODE_PLACE_C__, "nullptr IKCSessionCookie", m_own);
    return pSessionJK->getServiceSafe<IKCSessionCookie>();
}

// 守护线程
void CCtrlSSE::GuardThrd(void)
{
    boost::this_thread::interruption_point();
    boost::this_thread::sleep(boost::posix_time::milliseconds(2222));
    posix_time::ptime ptLastAliveList = posix_time::microsec_clock::local_time();
    posix_time::ptime ptLastAliveMap = posix_time::microsec_clock::local_time();
    while (m_running)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(999));
        posix_time::ptime ptNow = posix_time::microsec_clock::local_time();
        // 超时置为无效链接
        if (m_runPing && (ptNow - ptLastAliveList).total_seconds() >= 888)
            try
            {
                ptLastAliveList = ptNow;
                string sInfo = "";
                {
                    boost::shared_lock<boost::shared_mutex> lck(m_mtx);
                    // 循环每个链接
                    for (auto &wc : m_wcs)
                        if (TSSEconn::ewsInvalid != wc.second->m_status && (ptNow - wc.second->m_pt).total_seconds() >= 666)
                            sInfo += wc.second->SetDisable() + ",";
                }
                // 日志
                if (!sInfo.empty())
                    m_own.WriteLogDebug((string("Timeout ") + typeid(*this).name()).c_str(), __CURR_CODE_PLACE_C__, sInfo.c_str());
            }
            catch(TException& ex)
            {
                ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
                ex.LineCode() = __LINE__;
                m_own.WriteLog(ex);
            }
            catch (std::exception& ex)
            {
                m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch (...)
            {
                m_own.WriteLogFatal(m_own.getHint("Unknown_exception"), __CURR_CODE_PLACE_C__);
            }
        // 每天清除无效链接
        if (ptNow.date().day() != ptLastAliveMap.date().day())
            try
            {
                ptLastAliveMap = ptNow;
                boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtx);
                // 循环每个链接
                for(auto it = m_wcs.begin(); m_wcs.end() != it;)
                    if (TSSEconn::ewsInvalid == it->second->m_status)
                    {
                        boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                        it = m_wcs.erase(it);
                    }
                    else ++it;
            }
            catch(TException& ex)
            {
                ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
                ex.LineCode() = __LINE__;
                m_own.WriteLog(ex);
            }
            catch (std::exception& ex)
            {
                m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch (...)
            {
                m_own.WriteLogFatal(m_own.getHint("Unknown_exception"), __CURR_CODE_PLACE_C__);
            }
    }
}

// ping线程
void CCtrlSSE::PingThrd(void)
{
    boost::this_thread::interruption_point();
    asio::io_context ioSrv;
    asio::deadline_timer timer(ioSrv, posix_time::milliseconds(2222));
    for (int iWait = m_runPing ? 666 : 666666; m_running; )
    {
        timer.wait();
        timer.expires_at(timer.expires_at() + posix_time::milliseconds(iWait));
        try
        {
            if (m_runPing)
            {
                // 循环每个有效链接
                vector<std::shared_ptr<TSSEconn>> vWcs;
                {
                    boost::shared_lock<boost::shared_mutex> lck(m_mtx);
                    vWcs.reserve(m_wcs.size());
                    for (auto &wc : m_wcs)
                        if (TSSEconn::ewsValid == wc.second->m_status)
                            vWcs.push_back(wc.second);
                }
                // 发送ping
                for (auto &wc : vWcs)
                {
                    wc->m_pt = posix_time::microsec_clock::local_time();
                    TSendMQInfo tMxM;
                    tMxM.m_wcid = wc->m_wcid;
                    tMxM.m_type = 9;
                    CUtilFunc::SendMxMessage(m_own, m_sendMQname, tMxM, SMQInfoToStr(tMxM, __CURR_CODE_PLACE_C__));
                    iWait = min(666666, max(iWait, wc->m_keepalive_secs * 1000 / 6666));
                }
            }
        }
        catch(TException& ex)
        {
            ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
            ex.LineCode() = __LINE__;
            m_own.WriteLog(ex);
            cout << ex.error_info() << endl;
        }
        catch (std::exception& ex)
        {
            m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            cout << ex.what() << endl;
        }
        catch (...)
        {
            m_own.WriteLogFatal(m_own.getHint("Unknown_exception"), __CURR_CODE_PLACE_C__);
            cout << m_own.getHint("Unknown_exception") << endl;
        }
    }
}

// 消息队列线程
void CCtrlSSE::MQThrd(void)
{
    boost::this_thread::interruption_point();
    boost::this_thread::sleep(boost::posix_time::milliseconds(2222));
    // 启动队列
    m_own.WriteLogDebug("Start SendMQ", __CURR_CODE_PLACE_C__, m_sendMQname.c_str());
    //cout << "Start SendMQ: " << m_sendMQname << endl;
    // 创建发送消息队列
    boost::shared_ptr<interprocess::message_queue> mqPtr;
    auto fCreateMQ = [&]()
    {
        try
        {
            interprocess::message_queue::remove(m_sendMQname.c_str());
            mqPtr.reset(new interprocess::message_queue(interprocess::create_only, m_sendMQname.c_str(), m_sendMQsize, sizeof(TSendMQInfo)));
        }
        catch (interprocess::interprocess_exception& /*ex*/)
        {
            //m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, m_sendMQname.c_str());
            throw;
        }
    };
    while (m_running)
    try
    {
        boost::this_thread::sleep(posix_time::milliseconds(6));
        // 获取消息队列
        //interprocess::message_queue mq(interprocess::open_or_create, m_sendMQname.c_str(), m_sendMQsize, sizeof(TSendMQInfo));
        if (mqPtr.get() == nullptr) fCreateMQ();
        interprocess::message_queue &mq = *mqPtr.get();
        // 接收
        TSendMQInfo revVal;
        unsigned int priority = 0;
        interprocess::message_queue::size_type recvd_size = 0;
        mq.receive(&revVal, sizeof(TSendMQInfo), recvd_size, priority);
        // 队列大小
        unsigned iCurrNum = static_cast<unsigned>(mq.get_num_msg());
        unsigned iMsgSZ = static_cast<unsigned>(mq.get_max_msg_size());
        unsigned iMaxSZ = static_cast<unsigned>(mq.get_max_msg());
        if (iMaxSZ * 0.9 < iCurrNum)
            m_own.WriteLogDebug("SendMQ Full", __CURR_CODE_PLACE_C__, (format("[%d/%d %d]") % iCurrNum % iMaxSZ % iMsgSZ).str().c_str());
        // 处理
        if (m_running) SrvSend(revVal);
    }
    catch(boost::interprocess::interprocess_exception &ex)
    {
        mqPtr.reset();
        m_own.WriteLogFatal((boost::format("receive message_queue exception [%s] %d: %s") % m_sendMQname % ex.get_error_code() % ex.what()).str().c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        //cout << ex.what() << endl;
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = "receive message_queue exception [" + m_sendMQname + "] ";
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.WriteLog(ex);
        //cout << ex.error_info() << endl;
    }
    catch (std::exception& ex)
    {
        m_own.WriteLogFatal((boost::format("receive message_queue exception [%s] %s") % m_sendMQname % ex.what()).str().c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        //cout << ex.what() << endl;
    }
    catch (...)
    {
        m_own.WriteLogFatal((boost::format("receive message_queue exception [%s] %s") % m_sendMQname % m_own.getHint("Unknown_exception")).str().c_str(), __CURR_CODE_PLACE_C__);
        //cout << m_own.getHint("Unknown_exception") << endl;
    }
}

// 消息描述
string CCtrlSSE::SMQInfoToStr(TSendMQInfo& mxm, string sPos)
{
    return (format("[%s][id=%d,size=%d]%s") % sPos % mxm.m_wcid % static_cast<int>(mxm.m_size) % mxm.m_info).str();
}

// 发消息
int CCtrlSSE::SendMxMessage(int pid, long wcid, int type, string msg)
{
    // 消息结构
    TSendMQInfo tMxM;
    tMxM.m_type = type;
    tMxM.m_wcid = wcid;
    tMxM.m_size = 99 == type ? 0 : (88 == type ? 127 : min(static_cast<int>(msg.size()), 126));
    memset(tMxM.m_info, 0, sizeof(tMxM.m_info));
    // 消息
    unsigned iSz = static_cast<unsigned>(min(msg.size(), sizeof(tMxM.m_info)));
    if (iSz > 0) memcpy(tMxM.m_info, msg.c_str(), iSz);
    // 进程对应的消息队列名称
    string sMQname = MQname(pid);
    // 发送
    int iResult = CUtilFunc::SendMxMessage(m_own, sMQname, tMxM, SMQInfoToStr(tMxM, __CURR_CODE_PLACE_C__));
    cout << "send[" << sMQname << "][" << tMxM.m_wcid << "]" << tMxM.m_info << "[" << static_cast<int>(tMxM.m_size) << "]" << iResult << endl;
    return iResult;
}
// 服务器发送
void CCtrlSSE::SrvSend(TSendMQInfo& revVal)
{
    // 获取链接
    //decltype(m_wcs.begin()) it;
    TSSEconn *pWSconn = nullptr;
    {
        boost::shared_lock<boost::shared_mutex> lck(m_mtx);
        // 获取链接
        auto it = m_wcs.find(revVal.m_wcid);
        if (m_wcs.end() != it && TSSEconn::ewsValid == it->second->m_status)
            pWSconn = it->second.get();
    }
    // 发送数据
    if (nullptr != pWSconn)
    {
        string sBuf;
        // 文本、其他指令
        if (1 == revVal.m_type || 91 == revVal.m_type)
            sBuf.append(revVal.m_info, revVal.m_size);
        // 通过全局共享session中转
        else if (88 == revVal.m_type)
        {
            // 获取全局session的值
            IServiceReference* pRef = nullptr;
            CAutoRelease _auto([&](){ m_own.getContext().freeServiceReference(pRef); });
            IKCSessionCookie& sess = Session(pRef);
            sBuf = CUtilFunc::PCharSafeToStr(sess.GetSessionVal(revVal.m_info));
            sess.SetSessionVal(revVal.m_info, "");
        }
        // ping
        else if (9 == revVal.m_type)
            sBuf = ":ping\n\n";
        // 激活链接
        else if (99 == revVal.m_type)
        {
            pWSconn->CallSubCtrl("$active");
            cout << "activate[" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "] - " << pWSconn->m_sid << endl;
        }
        // Socket发送
        if (!sBuf.empty())
        {
            // 正常数据的sse格式化
            if (1 == revVal.m_type || 88 == revVal.m_type)
            {
                // 去掉头尾回车键
                algorithm::trim_if(sBuf, boost::is_any_of("\r\n"));
                // 每行添加行头
                algorithm::replace_all(sBuf, "\r\n", "\n");
                algorithm::replace_all(sBuf, "\n", "\ndata: ");
                // 开头补充行头
                sBuf = "data: " + sBuf;
                // 结尾补充2个回车键
                sBuf += "\n\n";
            }
            // 推送
            if (1 == revVal.m_type || 88 == revVal.m_type)
                cout << "send[" << pWSconn->m_sid << "]" << revVal.m_info << "[" << sBuf.size() << "]" << endl;
            else if (9 == revVal.m_type)
                cout << "ping[■■■" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "■■■] - " << pWSconn->m_sid << endl;
            // 向客户端发送消息
            auto stt = pWSconn->WC().Send(sBuf.c_str(), static_cast<unsigned>(sBuf.size()));
            if (IKCWebLongConn::ecsDisconn == stt) pWSconn->SetDisable();
        }
    }
    // 删除链接
    else CallSubCtrl("$invalid", CUtilFunc::GetUniqueConnID(m_own.getContext().GetSrvID(), revVal.m_wcid));
}

// todo: 调用子控制器（取消链接）
void CCtrlSSE::CallSubCtrl(string /*sName*/, string ssseid)
{
    try
    {
        /*
        // 请求响应上下文
        CRequestRespondTmp reTmp;
        // 附加控制器信息
        reTmp.AddExSingleInfo("$__KCCtrl_Main_Header__", sName);
        // 调用控制器
        CCltrFunc::CallCtrl(m_own.getContext(), reTmp);
        */
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = ssseid;
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.WriteLog(ex);
    }
    catch (std::exception& ex)
    {
        string sErr = "[" + ssseid + "] " + ex.what();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception");
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, ssseid.c_str());
    }
}

// 执行控制器
void CCtrlSSE::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pContext = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    auto fExceptInfo = [&](void)
    {
        string sAct(nullptr != pAct ? pAct : "");
        return "\n" + sAct + " - \t" + objCtrlD.GetPostStr();
    };
    try
    {
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, "send"));
        // 解析控制器
        map<string, string> mpCfgs;
        vector<string> lineVec;
        algorithm::split(lineVec, pContext, is_any_of(";\n"));
        for (auto &sLine : lineVec)
        {
            vector<string> cfgVec;
            algorithm::split(cfgVec, trim_copy(sLine), is_any_of(":"));
            if (cfgVec.size() > 1)
            {
                auto it = mpCfgs.find(cfgVec[0]);
                if (mpCfgs.end() != it) mpCfgs.erase(it);
                mpCfgs.insert(make_pair(trim_copy(cfgVec[0]), trim_copy(cfgVec[1])));
            }
        }
        m_own.WriteLogInfo(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, pMethod);
        // 注册链接（method="register"）
        if ("register" == sMethod) RegConn(objCtrlD, mpCfgs);
        // 发送消息（method="send"）
        else SendMsg(objCtrlD, mpCfgs);
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.WriteLog(ex);
        objCtrlD.SetJsonRespond(ex.error_id(), ex.what());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSSEWS + 6, ex.what());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSSEWS + 7, (string("Session ") + m_own.getHint("Unknown_exception")).c_str());
    }
}

// 注册SSE
void CCtrlSSE::RegConn(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    ICtrlApiDataX *pObjCtrlDX = dynamic_cast<ICtrlApiDataX*>(&objCtrlD);
    if (nullptr == pObjCtrlDX)
        throw TKCSSEWSException(ecd_ErrCode_KCSSEWS + 5, __CURR_CODE_PLACE_C__, m_own.getHint("Null_Point_Data_"), m_own);
    IKCRequestRespond& re = pObjCtrlDX->GetRequestRespond();
    IActionData& act = objCtrlD.ActionData();
    // 失败返回
    auto fRespondFail = [&](string err)
    {
        re.SetResponseStatus(500);
        re.AddResponseBody(err.c_str(), static_cast<int>(err.size()));
    };
    // 延迟将链接置为有效
    auto fDelaySetEnable = [&](long wcid, string sWsid)
    {
        boost::thread([&](long id, string sid){
            boost::this_thread::sleep(boost::posix_time::milliseconds(666));
            string sMsg = (format("event: BeginWork\ndata: %s\n\n") % sid).str();
            //SendMxMessage(this_process::get_id(), id, 1, sMsg);
            re.AddResponseBody(sMsg.c_str(), static_cast<int>(sMsg.size()));
            boost::shared_lock<boost::shared_mutex> lck(m_mtx);
            auto it = m_wcs.find(id);
            if (m_wcs.end() != it && TSSEconn::ewsProcess == it->second->m_status)
                it->second->m_status = TSSEconn::ewsValid;
        }, wcid, sWsid);
    };
    // 服务错误，关闭链接
    auto fDelayClose = [&](TSSEconn &wsc, long /*wcid*/, string sErr)
    {
        algorithm::replace_all(sErr, "\"", "'");
        algorithm::replace_all(sErr, ":", "|");
        algorithm::replace_all(sErr, "\n", "\t");
        algorithm::replace_all(sErr, "\r", "\t");
        string sMsg = (format("event: ServerError\ndata: %s\n\n") % sErr.substr(0, 99)).str();
        re.AddResponseBody(sMsg.c_str(), static_cast<int>(sMsg.size()));
        wsc.SetDisable();
    };
    // 成功返回
    auto fRespondSucc = [&](void)
    {
        // 链接编号
        long wcid = atol(act.GetSingleInfo("connection_id"));
        // 链接的指针
        TSSEconn *pWsc = nullptr;
        {
            boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtx);
            // 删除无效链接
            auto it = m_wcs.find(wcid);
            if (m_wcs.end() != it && TSSEconn::ewsInvalid == it->second->m_status)
            {
                boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                m_wcs.erase(it);
            }
            it = m_wcs.find(wcid);
            // 创建链接实例
            if (m_wcs.end() == it)
            {
                 IKCWebLongConn& wc = m_own.getContext().GetMain().PackWebConn(dynamic_cast<IKCRequestRespondData&>(re).getRe(), IKCStartWork::ewctSSE);
                 pWsc = new TSSEconn(*this, wc, atoi(act.GetSingleInfo("server_keep_alive_timeout")));
                 std::shared_ptr<TSSEconn> WscPtr(pWsc);
                 wcid = wc.GetID();
                 boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                 m_wcs.insert(make_pair(wcid, WscPtr));
            }
            else
            {
                pWsc = it->second.get();
                pWsc->m_status = TSSEconn::ewsProcess;
            }
            if (nullptr == pWsc)
                throw TKCSSEWSException(ecd_ErrCode_KCSSEWS + 8, __CURR_CODE_PLACE_C__, (format("%s[%d_%ld]") % m_own.getHint("Create_Fail_") % this_process::get_id() % wcid).str(), m_own);
        }
        string sSSEid = pWsc->m_sid;
        cout << "sseid: " << sSSEid << endl;
        // 调用后置控制器
        const int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true);
        // 成功注册
        if (0 == iErrCode)
        {
            // 延迟将链接置为有效
            fDelaySetEnable(wcid, sSSEid);
            // 添加sse返回头
            re.SetResponseContentType(c_SSEResponseContentType);
            re.AddResponseHeader("Cache-Control", "no-cache");
            //re.AddResponseHeader("Transfer-Encoding", "chunked");
            // 阻塞
            pWsc->WC().Start();
        }
        // 注册失败
        else
        {
            string sErr = (format("❌ [$after]%d-%s") % iErrCode % objCtrlD.JsonRespond().GetErrMsg()).str();
            m_own.WriteLogError(sErr.c_str(), "CCtrlSSE::RegConn::fRespondSucc", sSSEid.c_str());
            //fRespondFail(sErr);
            fDelayClose(*pWsc, wcid, sErr);
        }
    };
    // 调用子控制器（取消或激活）
    auto fCallSubCtrl = [&](string sName)
    {
        string ssseid = act.GetSingleInfo("UniqueConnID");
        // 调用子控制器
        const int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, sName);
        // 失败
        if (0 != iErrCode)
            throw TKCSSEWSException(iErrCode, __CURR_CODE_PLACE_C__, "[" + sName + "][" + ssseid + "] " + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    };
    // 执行
    string sMainCtrl = act.GetSingleInfo("$__KCCtrl_Main_Header__");
    // 首次链接，需保存链接
    if (sMainCtrl.empty())
        try
        {
            fRespondSucc();
        }
        catch(TException& ex)
        {
            ex.LineCode() = __LINE__;
            fRespondFail(ex.error_info());
            throw;
        }
        catch (std::exception& ex)
        {
            fRespondFail(ex.what());
            throw;
        }
        catch (...)
        {
            fRespondFail("unknown error");
            throw;
        }
    // 取消或激活链接
    else if ("$cancel" == sMainCtrl || "$active" == sMainCtrl)
        fCallSubCtrl(sMainCtrl);
}

// 发送消息
void CCtrlSSE::SendMsg(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    IActionData& act = objCtrlD.ActionData();
    // 类型和消息
    auto fTypeMsg = [&](string &type, string &msg)
    {
        // 激活的指令，不需要消息
        if ("activate" == type)
            msg.clear();
        // 大于等于126字节的消息，通过全局共享session中转
        else if (msg.size() >= 126)
        {
            IServiceReference* pRef = nullptr;
            CAutoRelease _auto([&](){ m_own.getContext().freeServiceReference(pRef); });
            IKCSessionCookie& ssjk = Session(pRef);
            string sName = (format("%s__%s_%d_%lld_msg") % m_sessionCtrl % m_type % this_process::get_id() % ++m_msgMaxID).str();
            ssjk.SetSessionVal(sName.c_str(), msg.c_str(), static_cast<unsigned>(msg.size()));
            ssjk.SetSessionExpire(sName.c_str(), "666s");
            type = "session";
            msg = sName;
        }
    };
    // 发消息
    auto fSend = [&](string sseid, string type, string msg)
    {
        // 分解进程和链接编号
        vector<string> idVec;
        algorithm::split(idVec, sseid, is_any_of("_"));
        if (idVec.size() == 3)
        {
            // 本服务器，向客户端推送消息
            if (m_own.getContext().GetSrvID() == lexical_cast<unsigned>(idVec[0]))
            {
                // 转换类型和消息
                fTypeMsg(type, msg);
                // 发送（如果消息队列不存在，重试3次）
                int iStt = 0;
                for (int i = 0; -1 == (iStt = SendMxMessage(lexical_cast<int>(idVec[1]), lexical_cast<long>(idVec[2]), "activate" == type ? 99 : ("session" == type ? 88 : 1), msg)) && i < 3; ++i)
                    boost::this_thread::sleep(posix_time::milliseconds(666));
                // 如果对应的进程失效（消息队列不存在），则关闭链接
                if (-1 == iStt)
                {
                    objCtrlD.JsonRespond().SetStr("sid", sseid.c_str());
                    int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$invalid");
                    if (0 != iErrCode)
                        m_own.WriteLogWarning("[$invalid] Failure", "CCtrlSSE::SendMsg::fSend", sseid.c_str());
                }
                // 如果发送不成功，则删除全局共享session
                if (0 != iStt && "session" == type)
                {
                    IServiceReference* pRef = nullptr;
                    CAutoRelease _auto([&](){ m_own.getContext().freeServiceReference(pRef); });
                    Session(pRef).SetSessionVal(msg.c_str(), "");
                }
            }
            // todo: 其他服务器，转发请求
            else
            {
                string sRelayUri = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$relay");
            }
        }
    };
    // 发送信息（逗号分隔）
    auto fSendList = [&](void)
    {
        string sErr = "";
        // 类型和消息
        string sType = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$ctype");
        string sMsg = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$msg");
        // 链接编号
        string ssseidList = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$sid");
        vector<string> sseidVec;
        algorithm::split(sseidVec, ssseidList, is_any_of(","));
        // 循环每个链接
        for (auto &sseid : sseidVec)
        {
            try
            {
                // 链接编号
                string ssseid = trim_copy(sseid);
                // 发送
                if (!ssseid.empty()) fSend(ssseid, sType, sMsg);
                boost::this_thread::sleep(boost::posix_time::milliseconds(6));
            }
            catch (std::exception& ex)
            {
                sErr += ex.what() + string("\n");
            }
        }
        if (!sErr.empty())
        {
            std::cout << sErr << std::endl;
            throw std::runtime_error(sErr);
        }
    };
    // 发送信息（数据集）
    auto fSendVals = [&](void)
    {
        auto &jsonVals = objCtrlD.GetBatchArrayJson();
        // 多消息集合，循环发送
        if (jsonVals.IsValid())
        {
            string sErr = "";
            // 类型和消息
            string sType = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$ctype");
            string sMsg = CCltrFunc::GetParm(objCtrlD, mpCfgs, "$msg");
            // 循环每个链接
            for (int i = 0, c = jsonVals.GetItemCount(); i < c; ++i)
            try
            {
                auto &itm = jsonVals.GetItem(i);
                // 类型和消息
                string sTypeNew = itm.GetStr("ctype", "");
                if (!sTypeNew.empty()) sType = sTypeNew;
                string sMsgNew = itm.GetStr("msg", "");
                if (!sMsgNew.empty()) sMsg = sMsgNew;
                // 链接编号
                string ssseid = itm.GetStr("sid", "");
                trim(ssseid);
                // 发送
                if (!ssseid.empty()) fSend(ssseid, sType, sMsg);
                boost::this_thread::sleep(boost::posix_time::milliseconds(6));
            }
            catch (std::exception& ex)
            {
                sErr += ex.what() + string("\n");
            }
            if (!sErr.empty())
            {
                std::cout << sErr << std::endl;
                throw std::runtime_error(sErr);
            }
        }
    };
    // 调用前置控制器
    {
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$before", true);
        if (0 != iErrCode)
            throw TKCSSEWSException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$before] ") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    }
    // 发消息
    fSendVals();
    fSendList();
    // 调用后置控制器
    {
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true);
        if (0 != iErrCode)
            throw TKCSSEWSException(iErrCode, __CURR_CODE_PLACE_C__, act.GetSingleInfo("the_request") + string("\n[$after] ") + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    }
}
