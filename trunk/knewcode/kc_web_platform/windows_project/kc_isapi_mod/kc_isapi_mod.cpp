/*
����IIS6��
Ĭ����վ->����->��Ŀ¼->��Ӧ�ó������ã�����->ӳ��->���
    ��ִ���ļ� = C:\KnewCode\kc_isapi_mod.dll
    ��չ��     = .kc

����IIS7��
Ĭ����վ->�������ӳ��->��ӽű�ӳ��
    ��ִ���ļ� = C:\KnewCode\kc_isapi_mod.dll
    ����·��   = .kc
��Ӧ�ó�����ж�Ӧ��Ӧ�ó���ĸ߼����ã������� -> ����32λӦ�ó�����Ϊtrue��
*/

#include "std.h"
#include "isapi_work.h"
using namespace KC;

extern "C"
{
    string g_strKCFullPath = "KCϵͳ·��";

    // ��̬�����
    BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
    {

        switch (ul_reason_for_call)
        {
        case DLL_PROCESS_ATTACH:
            // �õ���̬���·��
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

    // ��������
    DWORD HttpExtensionProc(EXTENSION_CONTROL_BLOCK *pECB)
    {
        try
        {
            char* err = (char*)("Internal server error. (IIS)" + g_strKCFullPath).c_str();
            DWORD dwSize = strlen(err);
            pECB->WriteClient(pECB->ConnID, err, &dwSize, 0);
            return HSE_STATUS_SUCCESS;

            // ��ʽ��·���ַ���
            g_strKCFullPath = CUtilFunc::FormatPath(g_strKCFullPath);
            // ��������ʵ��
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

    // ����ģ��
    BOOL GetExtensionVersion(HSE_VERSION_INFO *pVer)
    {
        return TRUE;
    }
    // ж��ģ��
    BOOL TerminateExtension(DWORD dwFlags)
    {
        return TRUE;
    }
}
