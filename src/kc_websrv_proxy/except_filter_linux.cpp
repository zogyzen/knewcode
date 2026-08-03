#include "std.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <execinfo.h>

#define BACKTRACE_SIZE 64
static string g_strCrashTmpLogPath = boost::filesystem::initial_path<boost::filesystem::path>().string();

string ShowStack(void)
{
    string sResult = "";
    void *buffer[BACKTRACE_SIZE + 1];
    memset(buffer, 0, BACKTRACE_SIZE + 1);
    int n = backtrace(buffer, BACKTRACE_SIZE);
    char **symbols = backtrace_symbols(buffer, n);
    if(nullptr == symbols)
    {
        perror("backtrace symbols");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; ++i)
        sResult += (format("%d: %s\n") % i % symbols[i]).str();
    free(symbols);
    return sResult;
}

void sigsegv_handler(int signo)
{
    if (signo == SIGSEGV)
    {
        CTempLog::WriteInSubDirByDate(g_strCrashTmpLogPath, "crash", "Crash", __CURR_CODE_PLACE_C__, ShowStack());
        exit(-1);
    }
}

extern "C" void SetExceptFilter(string sPath)
{
    if (boost::filesystem::exists(sPath)) g_strCrashTmpLogPath = sPath;
    signal(SIGSEGV, sigsegv_handler);
}
