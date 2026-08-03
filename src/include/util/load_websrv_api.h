#pragma once

#include <string>
#include <iostream>

#include <boost/dll.hpp>
#include <boost/format.hpp>

#include "util/util_funcs.h"
#include "util/temp_log.h"
#include "util/load_library.h"
#include "for_user/kc_websrv_api_cb.h"

namespace KC
{
    template<typename TOwn>
    class LoadWebSrvApi
    {
    public:
        // 初始化
        void Init(std::string fxPath)
        {
            // 加载api动态库
            std::string sDllPath = fxPath + "/kc_websrv_c_api" + c_so_ext_name;
            boost::system::error_code ec;
            std::cout << "*[knewcode] Load Webserve API - " << sDllPath << std::endl;
            m_lib.load(sDllPath, ec/*, boost::dll::load_mode::rtld_now*/);
            if (!m_lib.is_loaded())
            {
                std::string sErrMsg;
                if (ec)
                    sErrMsg = (boost::format("%d-%s") % ec.value() % ec.message()).str();
                else sErrMsg = KLoadInfo::_GetLoadDllError();
                std::cout << "\tLoad Webserve API Fail - " << sDllPath << " - " << sErrMsg << std::endl;
                throw std::runtime_error("Load Webserve API Fail - " + sDllPath + " - " + sErrMsg);
            }
            // 设置回调函数
            if (m_lib.has("SetCBFuncPointer"))
            {
                auto fsetcb = m_lib.get<int(int, const char*, void*)>("SetCBFuncPointer");
                /////////////////////////////////// 设置回调函数 ///////////////////////////////////
                // 获取服务器配置信息的回调函数
                fsetcb(3, "FGetSrvInfoCB", reinterpret_cast<void*>(GetSrvInfoCB));

                // 获取链接信息的回调函数
                fsetcb(4, "FGetConnInfoCB", reinterpret_cast<void*>(GetConnInfoCB));

                // 获取请求信息的回调函数
                fsetcb(4, "FGetRequestHeadCB", reinterpret_cast<void*>(GetRequestHeadCB));
                // 获取请求内容的回调函数
                fsetcb(3, "FGetRequestBodyCB", reinterpret_cast<void*>(GetRequestBodyCB));
                // 获取请其他求信息的回调函数
                fsetcb(4, "FGetRequestOtherCB", reinterpret_cast<void*>(GetRequestOtherCB));
                // 获取请求的链接句柄的回调函数
                fsetcb(3, "FGetRequestConnCB", reinterpret_cast<void*>(GetRequestConnCB));

                // 设置应答头信息的回调函数
                fsetcb(5, "FSetRespondHeadCB", reinterpret_cast<void*>(SetRespondHeadCB));
                // 设置应答内容的回调函数
                fsetcb(5, "FSetRespondBodyCB", reinterpret_cast<void*>(SetRespondBodyCB));
                // 设置其他应答信息的回调函数
                fsetcb(5, "FSetRespondOtherCB", reinterpret_cast<void*>(SetRespondOtherCB));
                // 设置提交应答的回调函数
                fsetcb(3, "FSetCommitResponseCB", reinterpret_cast<void*>(CommitResponseCB));

                // 通过错误码判断是否掉线的回调函数
                fsetcb(3, "FSetIsDisconnByErrCodeCB", reinterpret_cast<void*>(IsDisconnByErrCodeCB));
                // 通过错误码得到错误信息的回调函数
                fsetcb(3, "FSetGetStatusByErrCodeCB", reinterpret_cast<void*>(GetStatusByErrCodeCB));

                // 设置链接为长连接的回调函数
                fsetcb(4, "FSetKeepaliveCB", reinterpret_cast<void*>(SetKeepaliveCB));
                // 设置连接断开事件的回调函数
                fsetcb(5, "FSetBreakConnCB", reinterpret_cast<void*>(SetBreakConnCB));
                // 关闭连接的回调函数
                fsetcb(3, "FCloseConnCB", reinterpret_cast<void*>(CloseConnCB));
                // 得到是否连接的回调函数
                fsetcb(3, "FIsCloseConnCB", reinterpret_cast<void*>(IsCloseConnCB));
                // 重置链接时间的回调函数
                fsetcb(4, "FResetConnTimeCB", reinterpret_cast<void*>(ResetConnTimeCB));
                // 通过链接发送数据的回调函数
                fsetcb(5, "FSendByConnCB", reinterpret_cast<void*>(SendByConnCB));
                // 通过链接进行Socket发送数据的回调函数
                fsetcb(5, "FSockSendByConnCB", reinterpret_cast<void*>(SockSendByConnCB));
                // 通过链接进行websocket接收数据的回调函数
                fsetcb(5, "FWscRecvByConnCB", reinterpret_cast<void*>(WscRecvByConnCB));
            }
            // 初始化
            if (m_lib.has("Init"))
            {
                const int iErrCode = m_lib.get<int(void)>("Init")();
                if (0 != iErrCode) throw std::runtime_error("Error Code (" + std::to_string(iErrCode) + ").");
            }
            else throw std::runtime_error("Not Exists Function - Init, In kc_websrv_api.");
        }

        // 卸载
        void Free(void)
        {
            try
            {
                if (m_lib.is_loaded())
                {
                    if (m_lib.has("Free"))
                        m_lib.get<int(void)>("Free")();
                    m_lib.unload();
                }
            }
            catch (...) {}
        }

        // 处理请求
        int Request(intptr_t hRequest)
        {
            try
            {
                if (m_lib.is_loaded() && m_lib.has("Request"))
                    return m_lib.get<int(intptr_t)>("Request")(hRequest);
            }
            catch (...) { throw; }
            return 1;
        }

        // 写日志。0Trace、1Debug、2Info、3Warning、4Error
        int WriteLog(int lv, const char* msg, const char* pos, const char* type = "") const
        {
            try
            {
                if (m_lib.is_loaded() && m_lib.has("WriteLog"))
                    return m_lib.get<int(int, const char*, const char*, const char*)>("WriteLog")(lv, msg, pos, type);
            }
            catch (...) {}
            return -1;
        }

    protected:
        // 写临时日志
        static void WriteTmpLog(string type, string pos, string msg, int lv = 4)
        {
            if (!TOwn::m_self.expired())
            {
                if (TOwn::m_self.lock()->m_load.WriteLog(lv, msg.c_str(), pos.c_str(), type.c_str()) != 0)
                    CTempLog::WriteInDir(TOwn::m_self.lock()->FxPath(), type, pos, msg);
            }
            else CTempLog::Write(type, pos, msg);
        }
        // 检查回调是否正确
        static bool CheckArgFlag(int argCountLocal, const char* flagLocal, int argCountCall, const char* flagCall)
        {
            if (argCountLocal != argCountCall || CUtilFunc::PCharSafeToStr(flagLocal) != flagCall)
            {
                std::string sMsg = (boost::format("Arg Count: %d <> %d. Function: %s <> %s.\n") % argCountLocal % argCountCall % flagLocal % flagCall).str();
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, sMsg);
                return false;
            }
            else return true;
        }

    protected:
        /////////////////////////////////// 回调函数 ///////////////////////////////////
        // 获取服务器配置信息的回调函数指针类型。参数依次为：3；"FGetSrvInfoCB"；信息名称。返回信息内容。
        static const char* GetSrvInfoCB(int argCount, const char* flag, const char* name)
        {
            try
            {
                if (CheckArgFlag(3, "FGetSrvInfoCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->GetSrvInfoCB(CUtilFunc::PCharSafeToStr(name));
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%s - %s\n%s") % flag % name % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%s - %s") % flag % name).str());
            }
            return nullptr;
        }

        // 获取链接信息的回调函数指针类型。参数依次为：4；"FGetConnInfoCB"；链接的句柄；信息名称。返回信息内容。
        static const char* GetConnInfoCB(int argCount, const char* flag, intptr_t hConn, const char* name)
        {
            try
            {
                if (CheckArgFlag(4, "FGetConnInfoCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->GetConnInfoCB(hConn, CUtilFunc::PCharSafeToStr(name));
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s\n%s") % hConn % flag % name % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s") % hConn % flag % name).str());
            }
            return nullptr;
        }

        // 获取请求信息的回调函数指针类型。参数依次为：4；"FGetRequestHeadCB"；请求的句柄；信息名称。返回信息内容。
        static const char* GetRequestHeadCB(int argCount, const char* flag, intptr_t hRequest, const char* name)
        {
            try
            {
                if (CheckArgFlag(4, "FGetRequestHeadCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->GetRequestHeadCB(hRequest, CUtilFunc::PCharSafeToStr(name));
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s\n%s") % hRequest % flag % name % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s") % hRequest % flag % name).str());
            }
            return nullptr;
        }
        // 获取请求内容（post参数、多表单数据、其他二进制流等）的回调函数指针类型。参数依次为：3；"FGetRequestBodyCB"；请求的句柄。返回内容。
        // 内容的类型可通过头名称“content_type”获取，内容的长度可通过“content_length”获取。
        static const char* GetRequestBodyCB(int argCount, const char* flag, intptr_t hRequest)
        {
            try
            {
                if (CheckArgFlag(3, "FGetRequestBodyCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->GetRequestBodyCB(hRequest);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s\n%s") % hRequest % flag % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s") % hRequest % flag).str());
            }
            return "";
        }
        // 获取请其他求信息（get参数、本地文件等）的回调函数指针类型。参数依次为：4；"FGetRequestOtherCB"；请求的句柄；信息名称。返回信息内容。
        static const char* GetRequestOtherCB(int argCount, const char* flag, intptr_t hRequest, const char* name)
        {
            try
            {
                if (CheckArgFlag(4, "FGetRequestOtherCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->GetRequestOtherCB(hRequest, CUtilFunc::PCharSafeToStr(name));
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s\n%s") % hRequest % flag % name % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s") % hRequest % flag % name).str());
            }
            return nullptr;
        }
        // 获取请求的链接句柄的回调函数指针类型。参数依次为：3；"FGetRequestConnCB"；请求的句柄。返回链接句柄。
        static intptr_t GetRequestConnCB(int argCount, const char* flag, intptr_t hRequest)
        {
            try
            {
                if (CheckArgFlag(3, "FGetRequestConnCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->GetRequestConnCB(hRequest);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s\n%s") % hRequest % flag % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s") % hRequest % flag).str());
            }
            return 0;
        }

        // 设置应答头信息的回调函数指针类型。参数依次为：5；"FSetRespondHeadCB"；请求的句柄；头名称；内容（为空时，删除）。返回错误码：0无错误。
        static int SetRespondHeadCB(int argCount, const char* flag, intptr_t hRequest, const char* name, const char* val)
        {
            try
            {
                if (CheckArgFlag(5, "FSetRespondHeadCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->SetRespondHeadCB(hRequest, CUtilFunc::PCharSafeToStr(name), CUtilFunc::PCharSafeToStr(val));
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s = %s\n%s") % hRequest % flag % name % val % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s = %s") % hRequest % flag % name % val).str());
            }
            return -1;
        }
        // 设置应答内容的回调函数指针类型。参数依次为：5；"FSetRespondBodyCB"；请求的句柄；内容的类型（json数据、二进制流等）；内容；内容的字节数。返回错误码：0无错误。
        static int SetRespondBodyCB(int argCount, const char* flag, intptr_t hRequest, const char* content, int len)
        {
            try
            {
                string sContent;
                if (nullptr != content && len > 0) sContent.append(content, len);
                else if (nullptr != content) sContent = content;
                if (CheckArgFlag(5, "FSetRespondBodyCB", argCount, flag) && !sContent.empty() && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->SetRespondBodyCB(hRequest, sContent);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s => %d\n%s") % hRequest % flag % len % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s => %d") % hRequest % flag % len).str());
            }
            return -1;
        }
        // 设置其他应答信息（200或500状态等）的回调函数指针类型。参数依次为：5；"FSetRespondOtherCB"；请求的句柄；头名称；内容（为空时，删除）。返回错误码：0无错误。
        static int SetRespondOtherCB(int argCount, const char* flag, intptr_t hRequest, const char* name, const char* val)
        {
            try
            {
                if (CheckArgFlag(5, "FSetRespondOtherCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->SetRespondOtherCB(hRequest, CUtilFunc::PCharSafeToStr(name), CUtilFunc::PCharSafeToStr(val));
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s = %s\n%s") % hRequest % flag % name % val % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %s = %s") % hRequest % flag % name % val).str());
            }
            return -1;
        }
        // 设置提交应答的回调函数指针类型。参数依次为：3；"FSetCommitResponseCB"；请求的句柄。返回错误码：0无错误。
        static int CommitResponseCB(int argCount, const char* flag, intptr_t hRequest)
        {
            try
            {
                if (CheckArgFlag(3, "FSetCommitResponseCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->CommitResponseCB(hRequest);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s\n%s") % hRequest % flag % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s") % hRequest % flag).str());
            }
            return 1;
        }

        // 设置通过错误码判断是否掉线的回调函数指针类型。参数依次为：3；"FSetIsDisconnByErrCodeCB"；错误码。返回：true已掉线；false未掉线。
        static bool IsDisconnByErrCodeCB(int argCount, const char* flag, int errCode)
        {
            try
            {
                if (CheckArgFlag(3, "FSetIsDisconnByErrCodeCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->IsDisconnByErrCodeCB(errCode);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s\n%s") % errCode % flag % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s") % errCode % flag).str());
            }
            return false;
        }
        // 设置通过错误码得到错误信息的回调函数指针类型。参数依次为：3；"FSetGetStatusByErrCodeCB"；错误码。返回错误信息。
        static const char* GetStatusByErrCodeCB(int argCount, const char* flag, int errCode)
        {
            try
            {
                if (CheckArgFlag(3, "FSetGetStatusByErrCodeCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->GetStatusByErrCodeCB(errCode);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s\n%s") % errCode % flag % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s") % errCode % flag).str());
            }
            return "";
        }

        // 设置链接为长连接的回调函数指针类型。参数依次为：4；"FSetKeepaliveCB"；链接的句柄；启用或关闭长连接。返回错误码：0无错误。
        static int SetKeepaliveCB(int argCount, const char* flag, intptr_t hConn, bool enable)
        {
            try
            {
                if (CheckArgFlag(4, "FSetKeepaliveCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->SetKeepaliveCB(hConn, enable);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %s\n%s") % hConn % flag % (enable ? "true" : "false") % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %s") % hConn % flag % (enable ? "true" : "false")).str());
            }
            return -1;
        }
        // 设置连接断开事件的回调函数。参数依次为：5；"FSetBreakConnCB"；链接的句柄；回调的实例句柄；回调函数。返回错误码：0无错误。
        static int SetBreakConnCB(int argCount, const char* flag, intptr_t hConn, intptr_t hInstance, FBreakConnCBFromOwn fcb)
        {
            try
            {
                if (CheckArgFlag(5, "FSetBreakConnCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->SetBreakConnCB(hConn, hInstance, fcb);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %d\n%s") % hConn % flag % hInstance % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %d") % hConn % flag % hInstance).str());
            }
            return -1;
        }
        // 关闭连接的回调函数。参数依次为：3；"FCloseConnCB"；链接的句柄。返回错误码：0无错误。
        static int CloseConnCB(int argCount, const char* flag, intptr_t hConn)
        {
            try
            {
                if (CheckArgFlag(3, "FCloseConnCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->CloseConnCB(hConn);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s\n%s") % hConn % flag % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s") % hConn % flag).str());
            }
            return -1;
        }
        // 得到是否关闭连接的回调函数。参数依次为：3；"FIsCloseConnCB"；链接的句柄。返回：true连接已关闭；false未关闭。
        static bool IsCloseConnCB(int argCount, const char* flag, intptr_t hConn)
        {
            try
            {
                if (CheckArgFlag(3, "FIsCloseConnCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->IsCloseConnCB(hConn);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s\n%s") % hConn % flag % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s") % hConn % flag).str());
            }
            return false;
        }
        // 重置链接时间的回调函数。参数依次为：4；"FResetConnTimeCB"；链接的句柄；超时的时间（毫秒）。返回错误码：0无错误。
        static int ResetConnTimeCB(int argCount, const char* flag, intptr_t hConn, unsigned ms)
        {
            try
            {
                if (CheckArgFlag(4, "FResetConnTimeCB", argCount, flag) && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->ResetConnTimeCB(hConn, ms);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %d\n%s") % hConn % flag % ms % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s - %d") % hConn % flag % ms).str());
            }
            return -1;
        }
        // 通过链接发送数据的回调函数。参数依次为：5；"FSendByConnCB"；链接的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
        static int SendByConnCB(int argCount, const char* flag, intptr_t hConn, const char* buf, unsigned len)
        {
            try
            {
                string sBuf;
                if (nullptr != buf && len > 0) sBuf.append(buf, len);
                else if (nullptr != buf) sBuf = buf;
                if (CheckArgFlag(5, "FSendByConnCB", argCount, flag) && !sBuf.empty() && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->SendByConnCB(hConn, sBuf);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %d\n%s") % hConn % flag % len % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %d") % hConn % flag % len).str());
            }
            return -1;
        }
        // 通过链接进行Socket发送数据的回调函数。参数依次为：5；"FSockSendByConnCB"；链接的句柄；发送缓冲器；数据字节数。返回错误码：0无错误。
        static int SockSendByConnCB(int argCount, const char* flag, intptr_t hConn, const char* buf, unsigned len)
        {
            try
            {
                string sBuf;
                if (nullptr != buf && len > 0) sBuf.append(buf, len);
                else if (nullptr != buf) sBuf = buf;
                if (CheckArgFlag(5, "FSockSendByConnCB", argCount, flag) && !sBuf.empty() && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->SockSendByConnCB(hConn, sBuf);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %d\n%s") % hConn % flag % len % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %d") % hConn % flag % len).str());
            }
            return -1;
        }
        // 通过链接进行websocket接收数据的回调函数。参数依次为：5；"FWscRecvByConnCB"；请求的句柄；接收缓冲器；缓存区最大字节数。返回实际接收数据的字节数。
        static int WscRecvByConnCB(int argCount, const char* flag, intptr_t hConn, char* buf, unsigned len)
        {
            try
            {
                string sBuf;
                if (nullptr != buf && len > 0) sBuf.append(buf, len);
                else if (nullptr != buf) sBuf = buf;
                if (CheckArgFlag(5, "FWscRecvByConnCB", argCount, flag) && !sBuf.empty() && !TOwn::m_self.expired())
                    return TOwn::m_self.lock()->WscRecvByConnCB(hConn, buf, len);
            }
            catch (std::exception &ex)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %d\n%s") % hConn % flag % len % ex.what()).str());
            }
            catch (...)
            {
                WriteTmpLog("Call Back Error", __CURR_CODE_PLACE_C__, (boost::format("%d - %s -> %d") % hConn % flag % len).str());
            }
            return -1;
        }

    private:
        boost::dll::shared_library m_lib;

        friend TOwn;
    };
}
