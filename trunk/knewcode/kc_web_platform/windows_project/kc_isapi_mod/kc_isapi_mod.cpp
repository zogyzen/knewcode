/*
配置IIS6：
默认网站->属性->主目录->（应用程序设置）配置->映射->添加
    可执行文件 = C:\KnewCode\kc_isapi_mod.dll
    扩展名     = .kc

配置IIS7：
默认网站->处理程序映射->添加脚本映射
    可执行文件 = C:\KnewCode\kc_isapi_mod.dll
    请求路径   = .kc
打开应用程序池中对应的应用程序的高级设置，“常规 -> 启用32位应用程序”设为true。
*/

#include "std.h"
#include "isapi_work.h"
using namespace KC;

extern "C"
{
    string g_strKCFullPath = "KC系统路径";

    // 动态库入口
    BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
    {

        switch (ul_reason_for_call)
        {
        case DLL_PROCESS_ATTACH:
            // 得到动态库的路径
            {
                char szDllFullPath[MAX_PATH];
                ::GetModuleFileName(hModule, szDllFullPath, MAX_PATH);
                char* pFileName = strrchr(szDllFullPath, '\\');
                *pFileName = '\0';
                g_strKCFullPath = szDllFullPath;
            }
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
        }
        return TRUE;
    }

    // 处理请求
    DWORD HttpExtensionProc(EXTENSION_CONTROL_BLOCK *pECB)
    {
        try
        {
            char* err = (char*)("Internal server error. (IIS)" + g_strKCFullPath).c_str();
            DWORD dwSize = strlen(err);
            pECB->WriteClient(pECB->ConnID, err, &dwSize, 0);
            return HSE_STATUS_SUCCESS;

            // 格式化路径字符串
            g_strKCFullPath = CUtilFunc::FormatPath(g_strKCFullPath);
            // 主处理类实例
            static CISAPIWork g_work(g_strKCFullPath.c_str());
            g_work.Work(*pECB);
        }
        catch(std::exception ex)
        {
            char* err = (char*)(string("Internal server error. (IIS)") + ex.what()).c_str();
            DWORD dwSize = strlen(err);
            pECB->WriteClient(pECB->ConnID, err, &dwSize, 0);
        }
        catch(const char* ex)
        {
            char* err = (char*)(string("Internal server error. (IIS)") + ex).c_str();
            DWORD dwSize = strlen(err);
            pECB->WriteClient(pECB->ConnID, err, &dwSize, 0);
        }
        catch(...)
        {
            char err[] = "Internal server error. (IIS)";
            DWORD dwSize = strlen(err);
            pECB->WriteClient(pECB->ConnID, err, &dwSize, 0);
        }
        return HSE_STATUS_SUCCESS;
    }

    // 加载模块
    BOOL GetExtensionVersion(HSE_VERSION_INFO *pVer)
    {
        return TRUE;
    }
    // 卸载模块
    BOOL TerminateExtension(DWORD dwFlags)
    {
        return TRUE;
    }
}
