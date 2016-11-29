#pragma once

#ifdef WIN32    // Windows环境
    #include <windows.h>
    #include <dbghelp.h>
#else           // linux环境
    #include <execinfo.h>
#endif

#define BACKTRACESIZE 1024

namespace KC
{
	// 获取栈信息
	class CBacktraceSymbols
	{
	public:
    #ifdef WIN32    // Windows环境
		static string Get(void)
		{
		    string sResult = "";
            HANDLE process = GetCurrentProcess();
            DWORD dwOpts = SymGetOptions();
            dwOpts |= SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_ANYTHING;
            SymSetOptions ( dwOpts );
            SymInitialize(process, NULL, TRUE);
            CONTEXT context = {CONTEXT_FULL};
            ::GetThreadContext( GetCurrentThread(), &context );
            // ::RtlCaptureContext(&context);
            sResult = CBacktraceSymbols::StackTrace( &context, process );
		    return sResult;
		}
        static string addressToString( PVOID address, HANDLE& hProcess )
        {
            std::ostringstream oss;
            // First the raw address
            oss << address;
            // Then any name for the symbol
            struct tagSymInfo
            {
                IMAGEHLP_SYMBOL symInfo;
                char nameBuffer[BACKTRACESIZE];
            } SymInfo = { { sizeof( IMAGEHLP_SYMBOL ) } };
            IMAGEHLP_SYMBOL * pSym = &SymInfo.symInfo;
            pSym->MaxNameLength = sizeof( SymInfo ) - offsetof( tagSymInfo, symInfo.Name );
            DWORD dwDisplacement;
            if ( SymGetSymFromAddr( hProcess, (DWORD)address, &dwDisplacement, pSym) )
            {
                oss << " " << pSym->Name;
                if ( dwDisplacement != 0 )
                    oss << "+0x" << std::hex << dwDisplacement << std::dec;
            }

            // Finally any file/line number
            IMAGEHLP_LINE lineInfo = { sizeof( IMAGEHLP_LINE ) };
            if ( SymGetLineFromAddr( hProcess, (DWORD)address, &dwDisplacement, &lineInfo ) )
            {
                char const *pDelim = strrchr( lineInfo.FileName, '//' );
                oss << " at " << ( pDelim ? pDelim + 1 : lineInfo.FileName ) << "(" << lineInfo.LineNumber << ")";
            }
            return oss.str();
        }
        static string StackTrace( PCONTEXT pContext, HANDLE& hProcess )
        {
            std::ostringstream os;
            os << endl << " Frame     Address" << endl;
            STACKFRAME stackFrame = {0};
            stackFrame.AddrPC.Offset = pContext->Eip;
            stackFrame.AddrPC.Mode = AddrModeFlat;
            stackFrame.AddrFrame.Offset = pContext->Ebp;
            stackFrame.AddrFrame.Mode = AddrModeFlat;
            stackFrame.AddrStack.Offset = pContext->Esp;
            stackFrame.AddrStack.Mode = AddrModeFlat;
            while ( ::StackWalk(
               IMAGE_FILE_MACHINE_I386,
               hProcess,
               GetCurrentThread(), // this value doesn't matter much if previous one is a real handle
               &stackFrame,
               pContext,
               NULL,
               ::SymFunctionTableAccess,
               ::SymGetModuleBase,
               NULL ) )
            {
                os << " " << (PVOID) stackFrame.AddrFrame.Offset << " " << addressToString( (PVOID)stackFrame.AddrPC.Offset, hProcess ) << endl;
            }
            return os.str();
       }
    #else           // linux环境
		static string Get(void)
		{
		    std::ostringstream os;
            void *buffer[BACKTRACESIZE];
            int nptrs = backtrace(buffer, BACKTRACESIZE);
            char** strings = backtrace_symbols(buffer, nptrs);
            for (int j = 0; nullptr != strings && j < nptrs; j++) os << endl << strings[j];
            free(strings);
			os << endl;
            return os.str();
		}
    #endif
	};
}
