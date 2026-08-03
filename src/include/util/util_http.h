#pragma once

#include <boost/date_time.hpp>

#include <httplib.h>

#include "util/util_funcs_ex.h"

namespace KC
{
    // http功能公共函数
    struct CUtilHttp
	{
        // 分解域名
        static bool splitUrl(std::string sUrl, std::string &sProto, std::string &sHost, int &iPort)
        {
            sProto = "http";
            std::string sPort = "";
            // 分解
            std::vector<std::string> srvVec;
            boost::algorithm::split(srvVec, sUrl, boost::is_any_of(":"));
            if (srvVec.size() >= 3)
            {
                sProto = srvVec[0];
                sHost = srvVec[1];
                sPort = srvVec[2];
            }
            else if (srvVec.size() == 1)
                sHost = srvVec[0];
            else if (srvVec[0].substr(0, 4) == "http")
            {
                sProto = srvVec[0];
                sHost = srvVec[1];
            }
            else
            {
                sHost = srvVec[0];
                sPort = srvVec[1];
            }
            boost::trim_if(sHost, boost::is_any_of("/"));
            boost::trim_if(sPort, boost::is_any_of("/"));
            bool isSSL = "https" == sProto;
            if (sPort.empty()) sPort = isSSL ? "443" : "80";
            // 端口
            iPort = isSSL ? 443 : 80;
            try
            {
                iPort = boost::lexical_cast<int>(sPort);
            }
            catch (...) {}
            return isSSL;
        }

        // 客户端cookie
        struct TKCClnCookie
        {
            // 名称、值
            std::string m_name, m_val;
            // 期限
            boost::posix_time::ptime m_expires;
            // 路径
            std::string m_path;
        };
        typedef std::shared_ptr<TKCClnCookie> TKCClnCookiePtr;
        // 客户端cookie列表
        struct TKCClnCookieList
        {
            // cookie列表
            std::map<std::string, TKCClnCookiePtr> m_cookies;
            boost::shared_mutex m_mtxCks;

            // 设置cookie值（用服务器返回的“Set-Cookie”）
            void setCookie(std::string sCookieInfo)
            {
                auto ck = CUtilHttp::extractRespondCookie(sCookieInfo);
                if (ck.get() != nullptr)
                {
                    // 查询时，用可升级的共享锁
                    boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtxCks);
                    auto it = m_cookies.find(ck->m_name);
                    if (m_cookies.end() != it) it->second = ck;
                    else
                    {
                        // 插入时，升级为独占锁
                        //boost::unique_lock<boost::mutex> lck(m_mtxSrv);
                        boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                        m_cookies.insert(std::make_pair(ck->m_name, ck));
                    }
                }
            }
            // 得到cookie值
            TKCClnCookiePtr getCookie(std::string name)
            {
                TKCClnCookiePtr ptrResult;
                // 查询时，用可升级的共享锁
                boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtxCks);
                auto it = m_cookies.find(name);
                if (m_cookies.end() != it) ptrResult = it->second;
                return ptrResult;
            }

            // 生成向服务器提交的cookie（客户端设置的Cookie和上次服务器端返回的Cookie，有重复时，以客户端设置的为准）
            std::string makeCookie(std::string sClnCookie)
            {
                std::string sResult;
                // 客户端提交的cookie
                std::vector<std::pair<std::string, std::string>> vctClnCk;
                CUtilHttp::extractCookieKeyVal(sClnCookie, vctClnCk);
                std::map<std::string, std::string> mapClnCk;
                for (auto ckCln: vctClnCk) if (mapClnCk.end() == mapClnCk.find(ckCln.first)) mapClnCk.insert(ckCln);
                for (auto clCln: mapClnCk) sResult += clCln.first + "=" + clCln.second + "; ";
                // 将上次服务器端返回的cookie列表拷贝到临时变量
                std::vector<TKCClnCookiePtr> vecCk;
                {
                    boost::shared_lock<boost::shared_mutex> lckShare(m_mtxCks);
                    vecCk.reserve(m_cookies.size() + 1);
                    for (auto ck: m_cookies) vecCk.push_back(ck.second);
                }
                for (auto ck: vecCk)
                {
                    // 排除掉客户端提交过的cookie
                    auto it = mapClnCk.find(ck->m_name);
                    if (mapClnCk.end() == it) sResult += ck->m_name + "=" + ck->m_val + "; ";
                }
                boost::trim_right_if(sResult, boost::is_any_of("; "));
                return sResult;
            }
        };
        typedef std::shared_ptr<TKCClnCookieList> TKCClnCookieListPtr;
        // cookie管理
        struct TKCCookieManager
        {
            // 按服务器划分的cookie
            std::map<std::string, TKCClnCookieListPtr> m_srvCookies;
            boost::shared_mutex m_mtxSrv;

            // 按服务器获取cookie列表
            TKCClnCookieListPtr getCookieList(std::string sSrv)
            {
                TKCClnCookieListPtr cks;
                // 查询时，用共享锁
                boost::shared_lock<boost::shared_mutex> lckShare(m_mtxSrv);
                auto it = m_srvCookies.find(sSrv);
                if (m_srvCookies.end() != it) cks = it->second;
                return cks;
            }
            TKCClnCookieListPtr getOrCreateCookieList(std::string sSrv)
            {
                TKCClnCookieListPtr cks;
                // 查询时，用可升级的共享锁
                boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtxSrv);
                auto it = m_srvCookies.find(sSrv);
                if (m_srvCookies.end() != it) cks = it->second;
                else
                {
                    cks.reset(new TKCClnCookieList);
                    // 插入时，升级为独占锁
                    //boost::unique_lock<boost::mutex> lck(m_mtxSrv);
                    boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                    m_srvCookies.insert(std::make_pair(sSrv, cks));
                }
                return cks;
            }
        };

        // 提取应答cookie（例如，KCCLNID=BoJeWiqCx5KZmpId; expires=Tue, 23 Apr 2175 10:24:08 GMT; path=/）
        static TKCClnCookiePtr extractRespondCookie(std::string sInfo)
        {
            TKCClnCookiePtr ck;
            // 分解
            std::vector<std::pair<std::string, std::string>> vctCk;
            extractCookieKeyVal(sInfo, vctCk);
            if (!vctCk.empty())
            {
                // 名值对
                std::string sName = boost::trim_copy(vctCk[0].first);
                if (!sName.empty())
                {
                    ck.reset(new TKCClnCookie);
                    ck->m_name = sName;
                    ck->m_val = vctCk[0].second;
                }
                // todo: 其他信息
            }
            return ck;
        }

        // 提取cookie键值对（例如，KCCLNID=BYieMSzUni4jTQNT; KCSSID=CtNlRDmQ2qF0oeVz）
        static void extractCookieKeyVal(std::string sInfo, std::vector<std::pair<std::string, std::string>> &vct)
        {
            if (!sInfo.empty())
            {
                // 分解多个cookie
                std::vector<std::string> ckVec;
                boost::algorithm::split(ckVec, sInfo, boost::is_any_of(";"));
                for (auto ck: ckVec)
                {
                    std::string sCk = boost::trim_left_copy(ck);
                    // 分解键值对
                    // std::vector<std::string> kvVec;
                    // boost::algorithm::split(kvVec, sCk, boost::is_any_of("="));
                    // if (kvVec.size() == 1) vct.push_back(std::make_pair(kvVec[0], ""));
                    // else vct.push_back(std::make_pair(kvVec[0], kvVec[1]));
                    vct.push_back(CUtilFunc::splitKeyValue(sCk));
                }
            }
        }

        // httplib库的web请求
        template<typename TCln, typename TCallBack>
        static std::string httplibPostGet(TCallBack& cb, typename TCallBack::KcHeaders &headers, std::string sRequstBody, std::string sSrv, std::string sHost, int iPort, std::string sPth, std::string sPrm, bool isPost, bool isSSL, unsigned timeout = 30)
        {
            if ('/' == sPth[0])
                boost::trim_right_if(sSrv, boost::algorithm::is_any_of("/"));
            if (!sPrm.empty() && '?' != sPrm[0]) sPrm = "?" + sPrm;
            std::string sPthPrm = sPth + sPrm;
            // api完整路径
            std::string sApiUrl = sSrv + sPthPrm;
            // cout << typeid(TCln).name() << " - " << sApiUrl << "\t\t[" << sHost << ":" << iPort << "]" << endl;
            // http客户端对象
            TCln cli(sHost.c_str(), iPort);
            // cli.set_connection_timeout(8);
            cli.set_read_timeout(timeout);
            cli.set_write_timeout(timeout);
            // 针对https的网站，不做服务器端证书有效的检查
            if (isSSL) cli.enable_server_certificate_verification(false);
            // post和get方法
            std::string sRespond;
            auto fPost = [&](void)
            {
                return cli.Post(sPthPrm.c_str(), headers, sRequstBody, "application/json;charset=UTF-8");
                // return cli.Post(sPthPrm.c_str(), headers, [=](size_t offset, typename TCallBack::DataSink& sink) -> bool {
                //     sink.write(sRequstBody.data(), sRequstBody.size());
                //     sink.done();
                //     return true;
                // }, "application/json");
                // return cli.Post(sPthPrm.c_str(), headers, sRequstBody.size(), [=](size_t offset, size_t length, typename TCallBack::DataSink& sink) -> bool {
                //     boost::this_thread::sleep(boost::posix_time::milliseconds(66));
                //     sink.write(sRequstBody.data(), sRequstBody.size());
                //     sink.done();
                //     return true;
                // }, "application/json");
            };
            auto fGet = [&](void)
            {
                return cli.Get(sPthPrm.c_str(), headers);
            };
            // 请求
            cout << " \t[Request] Begin: " << sApiUrl << endl;
            cb.WriteLogTrace((boost::format("[Request %s] %s\n%s") % (isPost ? "POST" : "GET") % sApiUrl % sRequstBody.substr(0, 1024)).str().c_str(), __CURR_CODE_PLACE_C__);
            auto res = isPost ? fPost() : fGet();
            if (res && 200 == res->status)
            {
                sRespond = res->body;
                cout << "\t[Respond] " << sRespond.substr(0, 500) << endl;
                cb.RespondHeader(sSrv, res->headers);
                cb.WriteLogTrace((boost::format("[Respond] %s\n%s") % sApiUrl % sRespond.substr(0, 4000)).str().c_str(), __CURR_CODE_PLACE_C__);
            }
            else
            {
                int iErr = 0;
                std::string sBody;
                std::string sErr = (boost::format("Request Failed - %s\t\t[%s:%d] %d - %d")
                            % sApiUrl % sHost % iPort % (res ? res->status : 0) % static_cast<int>(res.error())).str();
                if (isSSL)
                {
                    auto result = cli.get_openssl_verify_result();
                    if (result)
                        sErr += std::string("\n") + X509_verify_cert_error_string(result);
                }
                if (res)
                {
                    iErr = res->status;
                    sBody = res->body;
                    sErr += (boost::format("\n%d: %s") % iErr % cb.status_message(iErr)).str();
                }
                else
                {
                    const char* arrErr[] = {"Success",
                                            "Unknown",
                                            "Connection",
                                            "BindIPAddress",
                                            "Read",
                                            "Write",
                                            "ExceedRedirectCount",
                                            "Canceled",
                                            "SSLConnection",
                                            "SSLLoadingCerts",
                                            "SSLServerVerification",
                                            "UnsupportedMultipartBoundaryChars",
                                            "Compression"};
                    iErr = static_cast<int>(res.error());
                    sErr += (boost::format("\n%d: %s") % iErr % (0 <= iErr && iErr < sizeof (arrErr) / sizeof (arrErr[0]) ? arrErr[iErr] : "")).str();
                }
                if (0 == iErr) iErr = 504;
                cb.WriteLogError((boost::format("[Request] Fail (%s)(%s) - %s\n%s") % typeid(TCln).name() % typeid(TCallBack).name() % sApiUrl % sErr).str().c_str(), __CURR_CODE_PLACE_C__, (boost::format("\r\n%s\r\n%s\r\n") % sRequstBody % sBody).str().c_str());
                cout << "\t" << sErr << endl;
                throw std::runtime_error(sErr);
            }
            //cout << sRespond << endl;
            //cb.WriteLogDebug(sApiUrl.c_str(), __CURR_CODE_PLACE_C__, ("\r\n" + sRequstBody.substr(0, 1024) + "\r\n" + sRespond.substr(0, 1024) + "\r\n").c_str());
            return sRespond;
        }

    };
}
