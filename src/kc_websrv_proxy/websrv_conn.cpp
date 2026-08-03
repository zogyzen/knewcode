#include "websrv_conn.h"
#include "kc_websrv_proxy.h"
#include "request_respond.h"

////////////////////////////////////////////////////////////////////////////////
// TWebSrvConn类
TKCWebConn::TKCWebConn(KCWebSrvProxy& own, IWSProxyRequestCB& recb)
    : m_own(own), m_connCB(own.SrvCB().MakeConnCB(recb))
    , m_uniqueID(CUtilFunc::GetUniqueConnID(own.GetContext().GetSrvID(), m_connCB.GetID()))
{
    // 设置断开的回调函数
    m_pCdb = new TDisconData(this);
    m_connCB.SetBreakCB(m_pCdb, reinterpret_cast<bool(*)(void*)>(TKCWebConn::DisconCB));
    // 设置长连接
    m_connCB.SetKeepalive();
}

TKCWebConn::~TKCWebConn(void)
{
    m_connCB.SetKeepalive(false);
    m_condForClose.notify_all();
    m_pCdb->pWebConn = nullptr;
    m_recvCB = nullptr;
    m_connCB.CloseConn();
    m_own.SrvCB().ReleaseConnCB(m_connCB);
}

// 断开链接的回调函数
bool TKCWebConn::DisconCB(TDisconData* cdb)
{
    try
    {
        TKCWebConn *wca = cdb->pWebConn;
        if (nullptr != wca)
        {
            wca->m_condForClose.notify_all();
            cout << "Offline: " << wca->GetUniqueID() << endl;
        }
        delete cdb;
    }
    catch(...) {}
    return true;
}

// 返回状态
IKCWebLongConn::EWebConnStatus TKCWebConn::GetStatus(int stt, string sFunc, string sBuf)
{
    IKCWebLongConn::EWebConnStatus eResult = IKCWebLongConn::ecsNormal;
    const char* sErr = m_own.SrvCB().GetStatus(stt);
    // 失败
    if (nullptr != sErr)
    {
        IKCObject *obj = m_recvCB;
        auto &recvCB = dynamic_cast<IKCWebLongConn::IRecvCB&>(*obj);
        string sErrInfo = (format("[%d_%ld]%s Error: ") % boost::this_process::get_id() % GetID() % sFunc).str();
        recvCB.WriteLogError(sErrInfo.c_str(), __CURR_CODE_PLACE_C__, (format("%d - %s (%s)\n%s\n") % stt % sErr % CUtilFunc::GbkToUtf8(sErr) % sBuf).str().c_str());
        cout << sErrInfo << stt << " - " << sErr << endl;
        eResult = m_own.SrvCB().isDisconn(stt) || m_connCB.isCloseConn() ? IKCWebLongConn::ecsDisconn : IKCWebLongConn::ecsError;
    }
    return eResult;
}

// 关闭前锁住
void TKCWebConn::LockForClose(void)
{
    posix_time::ptime ptTm = posix_time::microsec_clock::local_time();
    if (nullptr != m_recvCB)
    {
        m_recvCB->Cmd("Warning", "PreClose");
        // 等待断线通知
        boost::mutex::scoped_lock lck(m_mtxForClose);
        m_condForClose.wait(lck);
    }
    // 回调
    if (nullptr != m_recvCB) m_recvCB->Discon();
    cout << "LockForClose Termination: " << (posix_time::microsec_clock::local_time() - ptTm).total_milliseconds() << endl;
}

// 链接编号
long TKCWebConn::GetID(void)
{
    return m_connCB.GetID();
}
const char* TKCWebConn::GetUniqueID(void)
{
    return m_uniqueID.c_str();
}

// 设置接收消息回调接口
void TKCWebConn::SetRecvIF(IKCWebLongConn::IRecvCB* cb)
{
    m_recvCB = cb;
    if (nullptr == m_recvCB) m_condForClose.notify_all();
}

// 请求应答上下文
IBaseRequestRespond& TKCWebConn::MakeRe(void)
{
    return *new CWebRequestRespond(m_own, m_connCB.MakeReCB(), m_own.SrvCB());
}
void TKCWebConn::ReleaseRe(IBaseRequestRespond& re)
{
    CWebRequestRespond& wr = dynamic_cast<CWebRequestRespond&>(re);
    m_connCB.ReleaseReCB(wr.m_recb);
    delete &wr;
}

// 启动
void TKCWebConn::Start(void)
{
    boost::mutex::scoped_lock lck(m_mtxForClose);
    m_condForClose.wait(lck);
}

////////////////////////////////////////////////////////////////////////////////
// TWebSrvConnWebsocket 类
TWebSrvConnWebsocket::TWebSrvConnWebsocket(KCWebSrvProxy& own, IWSProxyRequestCB& recb) : TKCWebConn(own, recb)
{
}

TWebSrvConnWebsocket::~TWebSrvConnWebsocket(void)
{
    // 终止接收数据线程
    try
    {
        if (m_recvThrd != nullptr)
        {
            if (m_recvThrd->joinable()) m_recvThrd->interrupt();
            if (m_recvThrd->joinable()) m_recvThrd->timed_join(boost::posix_time::milliseconds(500));
            m_recvThrd.reset();
        }
    }
    catch (...) {}
}

// 启动
void TWebSrvConnWebsocket::Start(void)
{
    boost::thread::attributes attrs;
    attrs.set_stack_size(1024 * 64);
    m_recvThrd.reset(new boost::thread(attrs, boost::bind(&TWebSrvConnWebsocket::RecvInfoThrd, m_own.GetConnWPtr(GetID()))));
    TKCWebConn::Start();
}

// 发消息
IKCWebLongConn::EWebConnStatus TWebSrvConnWebsocket::Send(const char* buf, unsigned len)
{
    if (m_connCB.isCloseConn()) return IKCWebLongConn::ecsDisconn;
    size_t sz = len;
    int stt = 0;
    // 刷新链接时间
    m_connCB.ResetTime(-6);
    CAutoRelease _auto([&](){ m_connCB.ResetTime(6666); });
    // 非ssl
    if (!m_connCB.IsSSL())
        stt = m_connCB.SockSend(buf, sz);
    // 是ssl
    else
    {
        try
        {
            stt = m_connCB.Send(buf, len);
        }
        catch (...) {}
    }
    // 返回
    return GetStatus(stt, __CURR_CODE_PLACE_C__, buf);
}

// 接收数据的线程
void TWebSrvConnWebsocket::RecvInfoThrd(boost::weak_ptr<IKCWebLongConn> wp)
{
    //boost::this_thread::sleep_for(boost::chrono::milliseconds(2222));
    boost::this_thread::sleep(boost::posix_time::milliseconds(666));
    TWebSrvConnWebsocket *pThs = nullptr;
    if (!wp.empty() && !wp.expired() && wp.lock() != nullptr)
        pThs = dynamic_cast<TWebSrvConnWebsocket*>(wp.lock().get());
    if (nullptr != pThs)
    {
        long wcid = pThs->GetID();
        try
        {
            while (!wp.empty() && !wp.expired() && wp.lock() != nullptr && (pThs = dynamic_cast<TWebSrvConnWebsocket*>(wp.lock().get())) != nullptr
                   && !pThs->m_connCB.isCloseConn() && pThs->RecvData());
            cout << "RecvInfoThrd End: " << wcid << endl;
        }
        catch(...) { cout << "RecvInfoThrd Exception: " << wcid << endl; }
    }
}
// 接收数据
bool TWebSrvConnWebsocket::RecvData(void)
{
    boost::this_thread::interruption_point();
    boost::this_thread::sleep(boost::posix_time::milliseconds(6));
    // 调用回调接口
    bool bResult = true;
    if (nullptr != m_recvCB)
    {
        IKCObject *obj = m_recvCB;
        auto &recvCB = dynamic_cast<IKCWebLongConn::IRecvCB&>(*obj);
        try
        {
            // 接收
            size_t len = c_KCMaxParmBufSize;
            memset(m_recvBuf, 0, len);
            int stt = m_connCB.WscRecv(m_recvBuf, len);
            // 状态
            auto eResult = GetStatus(stt, __CURR_CODE_PLACE_C__);
            bResult = IKCWebLongConn::ecsDisconn != eResult;
            // 成功
            if (IKCWebLongConn::ecsNormal == eResult)
                // 回调
                recvCB.Recv(m_recvBuf, static_cast<unsigned>(len));
            // 失败
            else boost::this_thread::sleep_for(boost::chrono::milliseconds(6666));
        }
        catch(...) { cout << "RecvData Exception" << endl; }
    }
    else boost::this_thread::sleep_for(boost::chrono::milliseconds(1666));
    return bResult;
}

////////////////////////////////////////////////////////////////////////////////
// TWebSrvConnSSE 类
TWebSrvConnSSE::TWebSrvConnSSE(KCWebSrvProxy& own, IWSProxyRequestCB& recb) : TKCWebConn(own, recb)
{
}

// 发消息
IKCWebLongConn::EWebConnStatus TWebSrvConnSSE::Send(const char* buf, unsigned len)
{
    if (m_connCB.isCloseConn()) return IKCWebLongConn::ecsDisconn;
    // 刷新链接时间
    m_connCB.ResetTime(-6);
    CAutoRelease _auto([&](){ m_connCB.ResetTime(6666); });
    // 发送
    int stt = 0;
    try
    {
        stt = m_connCB.Send(buf, len);
    }
    catch (...) {}
    // 返回
    return GetStatus(stt, __CURR_CODE_PLACE_C__, buf);
}
