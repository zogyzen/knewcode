#pragma once

#ifdef WIN32    // Windows环境
    #include <shlobj.h>
#else           // linux环境
    #include <pwd.h>
#endif

namespace KC
{
	// 获取系统信息
	class CGetSysInfo
	{
	public:
    #ifdef WIN32    // Windows环境
		static string GetLogDir(void)
		{
		    char szPath[MAX_PATH];
            ::SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath);
            string sResult = szPath;
            sResult += "\\";
		    return sResult;
		}
    #else           // linux环境
		static string GetLogDir(void)
		{
            /*
		    string sResult = getpwuid(getuid())->pw_dir;
            sResult += "/";
		    return sResult;
		    */
		    return "/tmp/";
		}
    #endif
	};
}
