#include "std.h"
#include "load_kc.h"

// 文件名长度
#define KC_FILENAME_LENGTH 500
// 库文件路径
static char s_strLibFileName[KC_FILENAME_LENGTH + 1] = { 0 };
// 获取最近的错误
static char s_errLast[2050] = { 0 };

///////////////////////////////////////////////////////////////////////////// Windows环境 /////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
	// 头文件
    #include <windows.h>
	// 扩展名
    const char *c_LoadOutLibExtName = ".dll";
	
	// 库实例句柄
	static HINSTANCE s_lib_handle = 0;
	
	// 判断是否已加载
	int KC_HasLoad(void)
	{
		return s_lib_handle != 0 ? 1 : 0;
	}
	
	// 获取最近的错误
	const char* KC_GetLastError(void)
	{
		return s_errLast;
	}
	
	// 加载动态库
	const char* KC_LoadLib(const char* strLibFileName)
	{
		memset(s_errLast, 0, 2048);
                int iFlNmLen = (int)strlen(strLibFileName);
		if (iFlNmLen > KC_FILENAME_LENGTH)
		{
			sprintf_s(s_errLast, 2048, "The file name is too long - %s\n", strLibFileName);
			return s_errLast;
		}
		// 加载动态库
		memset(s_strLibFileName, 0, KC_FILENAME_LENGTH);
		memcpy(s_strLibFileName, strLibFileName, iFlNmLen);
		strcat(s_strLibFileName, c_LoadOutLibExtName);
		s_lib_handle = LoadLibraryA(s_strLibFileName);
		// 返回错误信息
		if (0 == s_lib_handle)
		{
                    DWORD error = GetLastError();
                    // LPVOID lpMsgBuf = 0;
                    // FormatMessage(
                    //     FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    //     FORMAT_MESSAGE_FROM_SYSTEM |
                    //     FORMAT_MESSAGE_IGNORE_INSERTS,
                    //     NULL,
                    //     error,
                    //     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    //     (LPTSTR) &lpMsgBuf,
                    //     0, NULL);
                    // sprintf_s(s_errLast, 2048, "Can't load lib file - %s\n%d - %s\n", s_strLibFileName, error, (LPCTSTR)lpMsgBuf);
                    sprintf_s(s_errLast, 2048, "Can't load lib file - %s (%d)\n", s_strLibFileName, error);
                    // LocalFree(lpMsgBuf);
                    return s_errLast;
		}
		printf("Load Success - %s\n", s_strLibFileName);
		return 0;
	}
	// 卸载库文件
	void KC_UnLoadLib(void)
	{
		if (0 != s_lib_handle) FreeLibrary(s_lib_handle);
		s_lib_handle = 0;
	}
	
	// 获取引出函数
	void* KC_GetLibFunc(const char* strFuncName)
	{
		memset(s_errLast, 0, 2048);
		void* _f = 0;
		if (0 != s_lib_handle)
		{
			_f = (void*)GetProcAddress(s_lib_handle, strFuncName);
			if (0 == _f)
				sprintf_s(s_errLast, 2048, "Can't get lib function - %s::%s\n", s_strLibFileName, strFuncName);
		}
		else sprintf(s_errLast, "Not load lib file - %s\n", s_strLibFileName);
		return _f;
	}	
	
///////////////////////////////////////////////////////////////////////////// linux环境 /////////////////////////////////////////////////////////////////////////////
#else
	// 头文件
    #include <dlfcn.h>
	// 扩展名
    const char *c_LoadOutLibExtName = ".so";
	
	// 库实例句柄
    static void* s_lib_handle = 0;
	
	// 判断是否已加载
	int KC_HasLoad(void)
	{
		return s_lib_handle != 0 ? 1 : 0;
	}
	
	// 获取最近的错误
	const char* KC_GetLastError(void)
	{
		return s_errLast;
	}
	
	// 加载动态库
	const char* KC_LoadLib(const char* strLibFileName)
	{
		memset(s_errLast, 0, 2048);
		int iFlNmLen = strlen(strLibFileName);
		if (iFlNmLen > KC_FILENAME_LENGTH)
		{
			sprintf(s_errLast, "The file name is too long - %s\n", strLibFileName);
			return s_errLast;
		}
		// 加载动态库
		memset(s_strLibFileName, 0, KC_FILENAME_LENGTH);
		memcpy(s_strLibFileName, strLibFileName, iFlNmLen);
		strcat(s_strLibFileName, c_LoadOutLibExtName);
		s_lib_handle = dlopen(s_strLibFileName, RTLD_LAZY);
		// 返回错误信息
		if (0 == s_lib_handle)
		{
                        sprintf(s_errLast, "Can't load lib file - %s\n%s\n", s_strLibFileName, dlerror());
			return s_errLast;
		}
		printf("Load Success - %s\n", s_strLibFileName);
		return 0;
	}
	// 卸载库文件
	void KC_UnLoadLib(void)
	{
		if (0 != s_lib_handle) dlclose(s_lib_handle);
		s_lib_handle = 0;
	}
	
	// 获取引出函数
	void* KC_GetLibFunc(const char* strFuncName)
	{
				memset(s_errLast, 0, 2048);
		void* _f = 0;
		if (0 != s_lib_handle)
		{
			_f = (void*)dlsym(s_lib_handle, strFuncName);
			if (0 == _f)
				sprintf(s_errLast, "Can't get lib function - %s::%s\n%s\n", s_strLibFileName, strFuncName, dlerror());
		}
		else sprintf(s_errLast, "Not load lib file - %s\n", s_strLibFileName);
		return _f;
	}	
		
#endif
