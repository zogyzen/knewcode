#pragma once

#include <csignal>

namespace KC
{
	// 获取栈信息
	class CRegisterSignal
	{
	public:
    #ifdef WIN32    // Windows环境
		static void Signal(int _SigNum,__p_sig_fn_t _Func)
		{
		    std::signal(_SigNum, _Func);
		}
		static void ReSignal(int _SigNum,__p_sig_fn_t _Func)
		{
		    std::signal(_SigNum, _Func);
		}
    #else           // linux环境
		static void Signal(int _SigNum,__sighandler_t _Func)
		{
		    struct sigaction act;
            act.sa_handler = _Func;
            act.sa_flags = SA_NODEFER | SA_NOMASK | SA_RESTART;
		    sigaction(_SigNum, &act, 0);
		}
		static void ReSignal(int _SigNum,__sighandler_t _Func)
		{
		}
    #endif
	};
}
