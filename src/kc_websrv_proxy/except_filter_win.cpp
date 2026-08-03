#include "std.h"

static LONG WINAPI TopLevelExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
    cout << "Enter TopLevelExceptionFilter Function" << endl;
    HANDLE hFile = CreateFile(L"knewcode.dmp",GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    MINIDUMP_EXCEPTION_INFORMATION stExceptionParam;
    stExceptionParam.ThreadId    = GetCurrentThreadId();
    stExceptionParam.ExceptionPointers = pExceptionInfo;
    stExceptionParam.ClientPointers    = FALSE;
    MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),hFile,MiniDumpWithFullMemory,&stExceptionParam,NULL,NULL);
    CloseHandle(hFile);
    getchar();
    return EXCEPTION_EXECUTE_HANDLER;
}

extern "C" void SetExceptFilter(string)
{
    SetUnhandledExceptionFilter(TopLevelExceptionFilter);
}
