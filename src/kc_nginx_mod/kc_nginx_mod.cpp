#include "std.h"
#include "nginx_work.h"

// 写日志。0Trace、1Debug、2Info、3Warning、4Error
void WriteLog(int lv, string sFlag, string sErr, string sPos)
{
    cout << sFlag << " - " << sErr << endl;
    if (g_work.get() != nullptr && g_work->m_load.WriteLog(lv, sFlag.c_str(), sPos.c_str(), sErr.c_str()) != 0)
        CTempLog::WriteInDir(boost::filesystem::exists(CNginxHelper::s_strKCSoPath) ? CNginxHelper::s_strKCSoPath : "", sFlag, sPos, sErr);
}

extern "C"
{
    // 设置nginx内置函数（内置全局变量）
    // void KC_set_ngx_function(const char *name, void *func)
    // {
    //     CNginxHelper::SetNgxFunction(name, func);
    // }

    // 本模块配置回调函数
    // char *KC_self_conf_main(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
    // {
    //     return CNginxHelper::SelfConfMain(cf, cmd, conf);
    // }
    // char *KC_self_conf_loc(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
    // {
    //     return CNginxHelper::SelfConfLoc(cf, cmd, conf);
    // }

    // “主配置项”引出函数“KC_mod_main_conf”的类型
    // char* KC_mod_main_conf(ngx_conf_t* cf, void* conf)
    // {
    //     return CNginxHelper::ModMainConf(cf, conf);
    // }
    // “Server配置项”引出函数“KC_mod_srv_conf”的类型
    // char* KC_mod_srv_conf(ngx_conf_t* cf, void* conf)
    // {
    //     return CNginxHelper::ModSrvConf(cf, conf);
    // }
    // “location”引出函数“KC_mod_loc_conf”的类型
    // char* KC_mod_loc_conf(ngx_conf_t* cf, void* parent, void* child)
    // {
    //     return CNginxHelper::ModLocConf(cf, parent, child);
    // }

    // “模块初始化”和“卸载”引出函数
    int KC_mod_init(TNgxSrvInfo *ngxInfo)
    {
        if (nullptr == ngxInfo)
        {
            WriteLog(4, "Init Error", "TNgxSrvInfo is nullptr", __CURR_CODE_PLACE_C__);
            return -1;
        }
        try
        {
            // 整理目录
            CNginxHelper::ModInit(*ngxInfo);
            // 创建和初始化实例
            g_work.reset(new CNginxWork);
            CNginxWork::m_self = g_work;
            g_work->Init();
            cout << endl << "load knewcode mod finish\t" << ngxInfo->ngx_cycle << endl << endl;
            return 0;
        }
        catch (std::exception &ex)
        {
            g_work.reset();
            cout << "Init Error - " << CNginxHelper::s_strKCSoPath << " \t" << ex.what() << endl;
            WriteLog(4, "Init Error", ex.what(), __CURR_CODE_PLACE_C__);
            ngxInfo->WriteNgxLog(4, ngxInfo->ngx_log, (boost::format("Init Error - <%s> %s") % __CURR_CODE_PLACE_C__ % ex.what()).str().c_str());
        }
        catch (...)
        {
            g_work.reset();
            cout << "Init Error - " << CNginxHelper::s_strKCSoPath << endl;
            WriteLog(4, "Init Error", "Unknown", __CURR_CODE_PLACE_C__);
            ngxInfo->WriteNgxLog(4, ngxInfo->ngx_log, (boost::format("Init Error - <%s>.") % __CURR_CODE_PLACE_C__).str().c_str());
        }
        return -1;
    }
    void KC_mod_free(void *cycle)
    {
        try
        {
            g_work.reset();
            cout << "free knewcode mod - " << CNginxHelper::s_strKCSoPath << " \t" << cycle << endl;
        }
        catch (...) {}
    }

    // “web请求”引出函数
    int KC_mod_handler(void *r, TNgxStr uri)
    {
        // WriteLog(0, "Begin Request ", (char*)(r->unparsed_uri.data), __CURR_CODE_PLACE_C__);
        // CAutoRelease _auto([&](){ WriteLog(0, "End Request ", (char*)(r->unparsed_uri.data), __CURR_CODE_PLACE_C__); });

        // return NGX_DECLINED;

        TNgxRequestData *pData = new TNgxRequestData;
        TNgxRequestData &rData = *pData;
        auto fRespondErr = [&](int iCode)
        {
            string sJsn = (boost::format(R"({"%s":%d,"%s":"Work Error - %s"})") % c_RESTful_errCode % iCode % c_RESTful_errMsg % CNginxHelper::NgxStrToStdStr(rData.unparsed_uri)).str();
            CNginxHelper::FAddResponseBody(rData, sJsn.c_str(), static_cast<unsigned>(sJsn.size()));
        };
        try
        {
            // 请求为空，放弃执行
            if (nullptr == r || nullptr == uri.data || 0 == uri.len) return -5;
            // cout << "*[nginx] - " << CNginxHelper::NgxStrToStdStr(uri) << endl;
            // 判断uri扩展名
            string sUri = CNginxHelper::NgxStrToStdStr(uri);
            string sExt = boost::filesystem::path(sUri).extension().string();
            if (boost::algorithm::to_lower_copy(sExt) != c_DefaultWorkUriExtension)
                return -5;      // 扩展名不为.kc，放弃执行
            // 扩展名为.kc继续执行
            memset(&rData, 0, sizeof(TNgxRequestData));
            rData.m_stampMS = CUtilFunc::GetCurrentStampMS();
            CNginxHelper::NgxInfo().GetRequestData(r, &rData);
            cout << "*[nginx] begin " <<  reinterpret_cast<intptr_t>(r) << " - " << rData.m_connection.m_id << " - " << CNginxHelper::NgxStrToStdStr(rData.request_line) << endl;
            CAutoRelease _auto([&](){
                cout << "*[nginx] end " <<  reinterpret_cast<intptr_t>(r) << " - " << "(" << (CUtilFunc::GetCurrentStampMS() - rData.m_stampMS) << "ms) - " << rData.m_connection.m_id << " - " << CNginxHelper::NgxStrToStdStr(rData.request_line) << endl;
                // boost::this_thread::sleep(boost::posix_time::milliseconds(166));
            });
            if (nullptr != rData.m_responseStatus) *rData.m_responseStatus = 200;
            // 加载核心处理功能
            int iErrCode = -5;
            if (g_work.get() == nullptr || (iErrCode = g_work->Work(rData)) > 0)
                throw std::runtime_error("Uninitialized - " + std::to_string(iErrCode));
            if (-4 == iErrCode) return iErrCode;
            // 返回
            if (nullptr != rData.m_responseStatus && 200 != *rData.m_responseStatus && 101 != *rData.m_responseStatus)
                cout << "Http Status - " << *rData.m_responseStatus << endl;
            // return iErrCode;
            return 0 == iErrCode ? 0 : -1;
        }
        catch (std::exception &ex)
        {
            string sErr = CNginxHelper::NgxStrToStdStr(rData.unparsed_uri) + "\n" + ex.what();
            WriteLog(4, "Work Error", sErr, __CURR_CODE_PLACE_C__);
            fRespondErr(-1);
        }
        catch (...)
        {
            string sErr = CNginxHelper::NgxStrToStdStr(rData.unparsed_uri);
            WriteLog(4, "Work Error", sErr, __CURR_CODE_PLACE_C__);
            fRespondErr(-2);
        }
        return -1;
    }
}
