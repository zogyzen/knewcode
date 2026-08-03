#include "std.h"
#include "framework.h"

extern "C"
{
	// 启动核心框架
    IFramework& CALL_TYPE GetFramework(IKCStartWork& own, const char* Path, const char* Cfg)
	{
        // 初始化 Boost.Locale 库
        // std::locale::global(std::locale("").combine<std::numpunct<char> >(std::locale::classic()));
        // boost::locale::generator gen;
        // std::locale::global(gen(""));
        // 路径
        string sPth = CUtilFunc::FormatPath(CUtilFunc::PCharSafeToStr(Path));
        // ExceptBacktrace::SetExceptFilter(sPth);
        // 配置文件
        string sCfg = CUtilFunc::PCharSafeToStr(Cfg);
        if (sCfg.empty()) sCfg = sPth + "/" + g_ModuleConfigFile;
        // cout << "*[knewcode] GetFramework: " << sPth << endl << "\t" << sCfg << endl;
        // 创建并启动核心框架
        static CFramework fw(own, CUtilFunc::ToAbsPath(sPth), CUtilFunc::ToAbsPath(sCfg));
		return fw;
	}
}
