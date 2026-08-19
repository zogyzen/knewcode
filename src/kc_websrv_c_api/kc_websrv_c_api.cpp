#include "api_work.h"
#include "work_proxy_cb.h"

// 主框架
static std::shared_ptr<CApiWork> g_work;
// 动态库目录
static std::string g_dllPth;

extern "C"
{
    // 写日志。0Trace、1Debug、2Info、3Warning、4Error
    int WriteLog(int lv, const char* msg, const char* pos, const char* type)
    {
        try
        {
            if (g_work.get() != nullptr)
            {
                switch(lv)
                {
                case 0:
                    g_work->GetContext().WriteLogTrace(msg, pos, type);
                    break;
                case 1:
                    g_work->GetContext().WriteLogDebug(msg, pos, type);
                    break;
                case 2:
                    g_work->GetContext().WriteLogInfo(msg, pos, type);
                    break;
                case 3:
                    g_work->GetContext().WriteLogWarning(msg, pos, type);
                    break;
                case 4:
                    g_work->GetContext().WriteLogError(msg, pos, type);
                    break;
                default:
                    g_work->GetContext().WriteLogTrace(msg, pos, ("(" + std::to_string(lv) + ")" + type).c_str());
                    break;
                }
                return 0;
            }
            else
            {
                string sMsg = CUtilFunc::PCharSafeToStr(msg);
                string sPos = CUtilFunc::PCharSafeToStr(pos);
                string sType = CUtilFunc::PCharSafeToStr(type);
                if (g_dllPth.empty())
                    CTempLog::Write(sMsg, sPos, sType);
                else
                    CTempLog::WriteInDir(g_dllPth, sMsg, sPos, sType);
                return 0;
            }
        }
        catch (...) {}
        return -1;
    }

    // 申请线程静态字符串空间（为C语言提供）
    char* GetThreadStaticSpace(unsigned len)
    {
        return CUtilFunc::MakeKeepStr(len);
    }

    // 设置回调函数指针
    int SetCBFuncPointer(int argCount, const char* flag, void* func)
    {
        try
        {
            printf("*[knewcode] SetCBFuncPointer: %s %d %#0x\n", CUtilFunc::PCharSafeToPChar(flag), argCount, static_cast<unsigned>((intptr_t)func));
            return CApiWork::SetCBFuncPointer(argCount, CUtilFunc::PCharSafeToStr(flag), func);
        }
        catch (...) {}
        return -1;
    }

    // 初始化
    int Init(void)
    {
        int iResult = 0;
        auto fThrdRun = [&](void)
        {
            try
            {
                cout << "*[knewcode] Init..." << endl;
                if (!CApiWork::CheckVitalCBFuncPtr())
                {
                    cout << "*[knewcode] Init Fail - CheckVitalCBFuncPtr." << endl;
                    WriteLog(4, "Init Fail - CheckVitalCBFuncPtr.", __CURR_CODE_PLACE_C__, "NullPtr");
                    iResult = -1;
                    return;
                }
                // if (CUtilFunc::PCharSafeToStr(CApiWork::GetSrvInfo("MainExeModRoot")).empty())
                // {
                //     cout << "*[knewcode] Init Fail - MainExeModRoot's Value Is Empty." << endl;
                //     WriteLog(4, "Init Fail - MainExeModRoot's Value Is Empty.", __CURR_CODE_PLACE_C__, "NullVal");
                //     iResult = -2;
                //     return;
                // }
                g_work.reset(new CApiWork);
                iResult = 0;
            }
            catch (std::exception &ex)
            {
                iResult = -1;
                cout << "*[knewcode] Init Fail - " << ex.what() << endl;
                WriteLog(4, ("Init Fail - " + string(ex.what())).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch (...)
            {
                iResult = -1;
                cout << "*[knewcode] Init Fail - Unknown." << endl;
                WriteLog(4, "Init Fail", __CURR_CODE_PLACE_C__, "Unknown");
            }
        };
        // fThrdRun();
        boost::thread::attributes attrs;
        attrs.set_stack_size(1024 * 1024 * 16);
        boost::thread thrd(attrs, fThrdRun);
        // std::thread thrd(fThrdRun);
        thrd.join();
        return iResult;
    }
    // 释放
    int Free(void)
    {
        try
        {
            g_work.reset();
            printf("*[knewcode] Free.\n");
        }
        catch (...) {}
        return -1;
    }

    // 请求
    int Request(intptr_t re)
    {
        int iResult = 15;   // 处理失败
        auto fThrdRun = [&](void)
        {
            try
            {
                printf("*[knewcode] Request: %lld\n", re);
                if (g_work.get() != nullptr) g_work->Work(re);
                else throw std::runtime_error("Not Init.");
                iResult = 0;    // 处理成功
            }
            catch (std::exception &ex)
            {
                WriteLog(4, ("Request Fail - " + string(ex.what())).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
            }
            catch (...)
            {
                WriteLog(4, "Request Fail", __CURR_CODE_PLACE_C__, "Unknown Error");
            }
        };
        fThrdRun();
        // std::thread thrd(fThrdRun);
        // thrd.join();
        return iResult;
    }
}
