#include "std.h"
#include "ctrl_ws.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlWS::TWSconn 类
CCtrlWS::TWSconn::TWSconn(CCtrlWS& own, IKCWebLongConn &wc, int kl) : CCtrlSSE::TSSEconn(own, wc, kl)
{
}

// 接收信息
void CCtrlWS::TWSconn::Recv(char* buf, unsigned /*len*/)
{
    string sTxt;
    if (ewsValid == m_status)
    try
    {
        // 帧类型
        unsigned char bOpcode = buf[0];
        bOpcode &= 0x0f;
        // 文本帧
        if (0x1 == bOpcode)
        {
            // 文本长度
            unsigned bLen = buf[1];
            bLen &= 0x7f;
            // 掩码
            unsigned char bMask[4] = { 0 };
            memcpy(bMask, buf + 2, 4);
            // 数据
            unsigned char *pDB = new unsigned char[bLen + 1]{ 0 };
            boost::shared_array<unsigned char> ptrDB(pDB);
            memcpy(pDB, buf + 6, bLen);
            // 解掩码
            for (unsigned char i = 0; i < bLen; ++i)
                pDB[i] ^= bMask[i % 4];
            sTxt.append(reinterpret_cast<char*>(pDB), bLen);
            // 解析json
            cJSON *pJson = cJSON_Parse(sTxt.c_str());
            if (nullptr == pJson) throw std::runtime_error(cJSON_GetErrorPtr());
            // 浏览器发送的“ping”消息
            auto fPingMsg = [&](void)
            {
                cJSON *pJsonPing = cJSON_GetObjectItemCaseSensitive(pJson, "ping");
                return (nullptr != pJsonPing && CUtilFunc::PCharSafeToStr(cJSON_GetStringValue(pJsonPing)) == this->m_sid);
            };
            if (fPingMsg())
            {
                m_pt = posix_time::microsec_clock::local_time();
                cout << "RecvInfoThrd[△△△" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "△△△]: ping - " << sTxt << endl;
            }
            // 浏览器发送的其他消息
            else
                cout << "RecvInfoThrd[" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "]: " << bLen << " - " << this->m_sid  << " \t\t" << sTxt << endl;
        }
        // ping
        else if (0x9 == bOpcode)
        {
            m_pt = posix_time::microsec_clock::local_time();
            cout << "RecvInfoThrd[" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "]: ping - " << this->m_sid << endl;
        }
        // pong
        else if (0xa == bOpcode)
        {
            m_pt = posix_time::microsec_clock::local_time();
            cout << "RecvInfoThrd[▲▲▲" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "▲▲▲]: pong - " << this->m_sid << endl;
        }
    }
    catch(TException& ex)
    {
        ex.OtherInfo() = "[" + m_sid + "] " + sTxt;
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.own().WriteLog(ex);
    }
    catch (std::exception& ex)
    {
        string sErr = "[" + m_sid + "] " + sTxt + "\n" + ex.what();
        m_own.own().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        string sErr = m_own.own().getHint("Unknown_exception");
        m_own.own().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, ("[" + m_sid + "] " + sTxt).c_str());
    }
}

// 其他指令
void CCtrlWS::TWSconn::Cmd(const char* c, const char* msg)
{
    string sCmd = CUtilFunc::PCharSafeToStr(c);
    if ("Warning" == sCmd)
    {
        string sMsg = (format(R"({"%s":"%s"})") % sCmd % msg).str();
        m_own.SendMxMessage(this_process::get_id(), m_wcid, 1, sMsg);
        m_own.own().WriteLogWarning(c, __CURR_CODE_PLACE_C__, msg);
    }
    else
    {
        string sMsg = (format(R"({"%s":"%s"})") % sCmd % msg).str();
        m_own.SendMxMessage(this_process::get_id(), m_wcid, 1, sMsg);
    }
    cout << __CURR_CODE_PLACE_C__ << ": " << sCmd << " - " << msg << endl;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlWS类
CCtrlWS::CCtrlWS(IKCSSEWS& own, string sName, property_tree::ptree& pt) : CCtrlSSE(own, sName, pt)
{
}

// 服务器发送
void CCtrlWS::SrvSend(TSendMQInfo& revVal)
{
    // 获取链接
    //decltype(m_wcs.begin()) it;
    TWSconn *pWSconn = nullptr;
    {
        boost::shared_lock<boost::shared_mutex> lck(m_mtx);
        // 获取链接
        auto it = m_wcs.find(revVal.m_wcid);
        if (m_wcs.end() != it && TWSconn::ewsValid == it->second->m_status)
            pWSconn = dynamic_cast<TWSconn*>(it->second.get());
    }
    // 发送数据
    if (nullptr != pWSconn)
    {
        string sBuf;
        // 文本
        if (1 == revVal.m_type)
        {
            cout << "send: " << revVal.m_info << "[" << static_cast<unsigned>(revVal.m_size) << "]" << endl;
            // websocket消息
            char buf[2] = { 0 };
            buf[0] = (char)0x81;
            buf[1] = revVal.m_size;
            sBuf.append(buf, 2);
            sBuf.append(revVal.m_info, revVal.m_size);
        }
        // 通过全局共享session中转
        else if (88 == revVal.m_type)
        {
            // 获取全局session的值
            string sMsg;
            {
                IServiceReference* pRef = nullptr;
                CAutoRelease _auto([&](){ m_own.getContext().freeServiceReference(pRef); });
                IKCSessionCookie& sess = Session(pRef);
                sMsg = CUtilFunc::PCharSafeToStr(sess.GetSessionVal(revVal.m_info));
                sess.SetSessionVal(revVal.m_info, "");
            }
            if (!sMsg.empty())
            {
                // 协议头
                char buf[10] = { 0 };
                buf[0] = (char)0x81;
                // 数据长度可存储在16位无符号整型里
                if (sMsg.size() < pow(2, 16))
                {
                    buf[1] = 126;
                    unsigned short iSz = CUtilFunc::RotateT(static_cast<unsigned short>(sMsg.size()));
                    memcpy(&buf[2], &iSz, 2);
                    sBuf.append(buf, 4);
                }
                // 数据长度需存储在64位无符号整型里
                else
                {
                    buf[1] = 127;
                    unsigned long long iSz = CUtilFunc::RotateT(static_cast<unsigned long long>(sMsg.size()));
                    memcpy(&buf[2], &iSz, 8);
                    sBuf.append(buf, 10);
                }
                // 数据
                sBuf.append(sMsg);
            }
        }
        // ping
        else if (9 == revVal.m_type)
        {
            char buf[2] = { 0 };
            buf[0] = (char)0x89;
            sBuf.append(buf, 2);
        }
        // 激活链接
        else if (99 == revVal.m_type)
        {
            pWSconn->CallSubCtrl("$active");
            cout << "activate[" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "] - " << pWSconn->m_sid << endl;
        }
        // Socket发送
        if (!sBuf.empty())
        {
            if (1 == revVal.m_type || 88 == revVal.m_type)
                cout << "send[" << pWSconn->m_sid << "]" << revVal.m_info << "[" << sBuf.size() << "]" << endl;
            else if (9 == revVal.m_type)
                cout << "ping[■■■" << posix_time::to_simple_string(posix_time::microsec_clock::local_time()) << "■■■] - " << pWSconn->m_sid << endl;
            // 向客户端发送消息
            auto stt = pWSconn->WC().Send(sBuf.c_str(), static_cast<unsigned>(sBuf.size()));
            if (IKCWebLongConn::ecsDisconn == stt)
            {
                cout << "send[" << pWSconn->m_sid << "] Disconnect. " << revVal.m_info << "[" << sBuf.size() << "]" << endl;
                //pWSconn->SetDisable();
            }
        }
    }
    // 删除链接
    else CallSubCtrl("$invalid", CUtilFunc::GetUniqueConnID(m_own.getContext().GetSrvID(), revVal.m_wcid));
}

// 注册websocket
void CCtrlWS::RegConn(ICtrlApiData& objCtrlD, map<string, string>& mpCfgs)
{
    ICtrlApiDataX *pObjCtrlDX = dynamic_cast<ICtrlApiDataX*>(&objCtrlD);
    if (nullptr == pObjCtrlDX)
        throw TKCSSEWSException(ecd_ErrCode_KCSSEWS + 5, __CURR_CODE_PLACE_C__, m_own.getHint("Null_Point_Data_"), m_own);
    IKCRequestRespond& re = pObjCtrlDX->GetRequestRespond();
    IActionData& act = objCtrlD.ActionData();
    // 延迟将链接置为有效
    auto fDelaySetEnable = [=](long wcid, string sWsid)
    {
        boost::thread([&](long id, string sid){
            boost::this_thread::sleep(boost::posix_time::milliseconds(1666));
            {
                boost::shared_lock<boost::shared_mutex> lck(m_mtx);
                auto it = m_wcs.find(id);
                if (m_wcs.end() != it && TWSconn::ewsProcess == it->second->m_status)
                    it->second->m_status = TWSconn::ewsValid;
            }
            string sMsg = (format(R"({"sid":"%s"})") % sid).str();
            SendMxMessage(this_process::get_id(), id, 1, sMsg);
        }, wcid, sWsid);
    };
    // 延迟关闭链接
    auto fDelayClose = [=](TWSconn &wsc, long wcid, string sErr)
    {
        boost::thread([&](long id, string err){
            boost::this_thread::sleep(boost::posix_time::milliseconds(1666));
            {
                boost::shared_lock<boost::shared_mutex> lck(m_mtx);
                auto it = m_wcs.find(id);
                if (m_wcs.end() != it && TWSconn::ewsProcess == it->second->m_status)
                    it->second->m_status = TWSconn::ewsValid;
            }
            algorithm::replace_all(err, "\"", "'");
            algorithm::replace_all(err, ":", "|");
            algorithm::replace_all(err, "\n", " ");
            algorithm::replace_all(err, "\r", " ");
            string sMsg = (format(R"({"errWithCloseWSConn":"%s"})") % err.substr(0, 99)).str();
            SendMxMessage(this_process::get_id(), id, 1, sMsg.substr(0, 125));
            boost::this_thread::sleep(boost::posix_time::milliseconds(1888));
            wsc.SetDisable();
        }, wcid, sErr);
    };
    // 成功返回
    auto fRespondSucc = [&](void)
    {
        // 加密
        string sKey = re.GetRequestHeader("Sec-WebSocket-Key");
        string sAcc = CUtilFunc::GetSHA1(sKey + c_WebSocketPublicKey);
        // 返回
        re.AddResponseHeader("Transfer-Encoding", "chunked");
        re.AddResponseHeader("Upgrade", "websocket");
        re.AddResponseHeader("Connection", "Upgrade");
        re.AddResponseHeader("Sec-WebSocket-Accept", sAcc.c_str());
        re.SetResponseStatus(101);
        re.SetResponseContentType(c_WebSocketResponseContentType);
    };
    // 保存链接
    auto fSaveConn = [&](void)
    {
        // 链接编号
        long wcid = atol(act.GetSingleInfo("connection_id"));
        boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtx);
        auto it = m_wcs.find(wcid);
        // 删除无效链接
        if (m_wcs.end() != it && TWSconn::ewsInvalid == it->second->m_status)
        {
            boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
            m_wcs.erase(it);
        }
        it = m_wcs.find(wcid);
        // 创建链接实例
        TWSconn *pWsc = nullptr;
        if (m_wcs.end() == it)
        {
             IKCWebLongConn& wc = m_own.getContext().GetMain().PackWebConn(dynamic_cast<IKCRequestRespondData&>(re).getRe(), IKCStartWork::ewctWebsocket);
             pWsc = new TWSconn(*this, wc, atoi(act.GetSingleInfo("server_keep_alive_timeout")));
             std::shared_ptr<TWSconn> WscPtr(pWsc);
             wcid = wc.GetID();
             boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
             m_wcs.insert(make_pair(wcid, WscPtr));
        }
        else
        {
            pWsc = dynamic_cast<TWSconn*>(it->second.get());
            pWsc->m_status = TWSconn::ewsProcess;
        }
        if (nullptr == pWsc)
            throw TKCSSEWSException(ecd_ErrCode_KCSSEWS + 7, __CURR_CODE_PLACE_C__, (format("%s[%d_%ld]") % m_own.getHint("Create_Fail_") % this_process::get_id() % wcid).str(), m_own);
        string sWsid = pWsc->m_sid;
        cout << "WSID: " << sWsid << endl;
        // 设置活动页变量
        //CCltrFunc::SetParm(re, mpCfgs, "$set_sid", sWsid);
        // 调用后置控制器
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, "$after", true);
        if (0 == iErrCode)
        {
            // 成功返回
            fRespondSucc();
            // 延迟将链接置为有效
            fDelaySetEnable(wcid, sWsid);
            // 阻塞
            pWsc->WC().Start();
        }
        else
        {
            string sErr = (format("[$after][%d]%s") % iErrCode % objCtrlD.JsonRespond().GetErrMsg()).str();
            m_own.WriteLogError(sErr.c_str(), "CCtrlWS::RegWS::fSaveConn", sWsid.c_str());
            //throw TKCSSEWSException(iErrCode, __CURR_CODE_PLACE_C__, sErr, m_own);
            // 延迟关闭链接
            fDelayClose(*pWsc, wcid, sErr);
        }
    };
    // 失败返回
    auto fRespondFail = [&](string err)
    {
        re.SetResponseStatus(500);
        re.AddResponseBody(err.c_str(), static_cast<int>(err.size()));
    };
    // 调用子控制器（取消或激活）
    auto fCallSubCtrl = [&](string sName)
    {
        //string sWsid = re.GetSingleInfo("$set_sid");
        string sWsid = act.GetSingleInfo("UniqueConnID");
        // 设置活动页变量
        //CCltrFunc::SetParm(re, mpCfgs, "$set_sid", sWsid);

        // 调用子控制器
        int iErrCode = CCltrFunc::CallCtrl(objCtrlD, mpCfgs, sName);
        if (0 != iErrCode)
            throw TKCSSEWSException(iErrCode, __CURR_CODE_PLACE_C__, "[" + sName + "][" + sWsid + "] " + objCtrlD.JsonRespond().GetErrMsg(), m_own);
    };
    // 执行
    string sMainCtrl = act.GetSingleInfo("$__KCCtrl_Main_Header__");
    // 协议升级
    if (sMainCtrl.empty())
        try
        {
            fSaveConn();
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
