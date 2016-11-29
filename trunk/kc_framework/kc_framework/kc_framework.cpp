#include "std.h"
#include "framework.h"

extern "C"
{
	// 启动核心框架
	IFramework& CALL_TYPE GetFramework(const char* Path)
	{
	    // 转换为绝对路径
	    boost::filesystem::path _path(Path);
	    _path = boost::filesystem::system_complete(_path);
        string sPath = _path.string();
        // 格式化路径字符串
        sPath = CUtilFunc::FormatPath(sPath);
        // 创建并启动核心框架
        static CFramework fw(sPath);
		return fw;
	}
}
