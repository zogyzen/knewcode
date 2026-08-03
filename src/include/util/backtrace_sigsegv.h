#pragma once

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32    // Windows环境
#include <Windows.h>
#include <DbgHelp.h>
#else           // linux环境
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>
#endif

#include <iostream>

#include <boost/format.hpp>

#include "util/temp_log.h"

namespace KC
{
    // 段错误退出时的调用堆栈
    class ExceptBacktrace
    {
    public:
#ifdef WIN32    // Windows环境
        static LONG WINAPI TopLevelExceptionFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
        {
            std::cout << "Enter TopLevelExceptionFilter Function" << std::endl;
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

        static void SetExceptFilter(std::string)
        {
            SetUnhandledExceptionFilter(TopLevelExceptionFilter);
        }
#else           // linux环境
        static inline std::string s_strCrashTmpLogPath = boost::filesystem::initial_path<boost::filesystem::path>().string();
        #define BACKTRACE_SIZE 128

        // 进程基础地址
        #define MAX_LIB_NUM 256
        static inline int max_lib = 0;
        struct library_maps_t
        {
            char not_care[4][32];
            char library_path[128];
            char *offset_start;
            char *offset_end;
        };
        static inline library_maps_t lib[MAX_LIB_NUM];
        static int dump_library_maps()
        {
            char cmd[64] = {0};
            char maps_line[512] = { 0 };
            // char *last_library_path = nullptr;
            int num_exe = 0;
            int maps_column_num = 0;
            library_maps_t* temp = &lib[0];
            FILE* fd_maps = nullptr;

            memset(&lib, 0, sizeof(lib));
            memset(maps_line, 0, sizeof(maps_line));

            /* 1 get maps info to file*/
            snprintf(cmd, sizeof(cmd), "/proc/%d/maps", getpid());
            fd_maps=fopen(cmd, "r");
            if (fd_maps == NULL)
            {
                printf("ERROR\n");
                return -1;
            }

            /* 2 save maps info to lib*/
            while(NULL!=fgets(maps_line,sizeof(maps_line),fd_maps))
            {
                maps_column_num = sscanf(maps_line,"%p-%p\t%s\t%s\t%s\t%s\t%s"
                                         ,&temp->offset_start
                                         ,&temp->offset_end
                                         ,temp->not_care[0]
                                         ,temp->not_care[1]
                                         ,temp->not_care[2]
                                         ,temp->not_care[3]
                                         ,temp->library_path);
                #ifdef BT_MOD_DEBUG_ON
                printf("%p-%p\t%s\t%s\t%s\t%s\t%s\n",temp->offset_start,temp->offset_end,temp->not_care[0],
                       temp->not_care[1],temp->not_care[2],temp->not_care[3],temp->library_path);
                #endif
                if(maps_column_num == 7 &&( (num_exe == 0 && 0==strcmp("r-xp",temp->not_care[0]))
                                             || strcmp(temp->library_path,temp[-1].library_path)) )
                {
                    if(num_exe == MAX_LIB_NUM)
                    {
                        printf("Error MAX_LIB_NUM is %d!!!\n", MAX_LIB_NUM);
                        break;
                    }
                    temp++;
                    num_exe++;
                }
                else
                {   /*so文件 是记录[min_offset, r-xp:offset_end]*/
                    if(  0==strcmp("r-xp",temp->not_care[0])
                        && 0==strcmp(temp->library_path,temp[-1].library_path))
                    {
                        temp[-1].offset_end = temp->offset_end;
                    }
                    memset(temp, 0, sizeof(*temp));
                }

                memset(maps_line, 0, sizeof(maps_line));
            }

            fclose(fd_maps);
            max_lib = num_exe;
            #ifdef BT_MOD_DEBUG_ON
            printf("[%s][%d]debug_backtrace_init success,num_exe:%d - %s\n", __FUNCTION__, __LINE__, num_exe, lib[num_exe - 1].library_path);
            num_exe = 0;
            printf("\n============= so lib info ===========\n");
            while(num_exe < max_lib)
            {
                printf("%p-%p\t%s\t%s\n", lib[num_exe].offset_start, lib[num_exe].offset_end, temp->not_care[0], lib[num_exe].library_path);
                ++num_exe;
            }
            printf("=============     end     ===========\n");
            #endif
            return 0;
        }

        // 调用堆栈
        struct bt_t
        {
            char exe[300] = { 0 };
            char* offset = nullptr;
        };
        static void output_addrline(bt_t *bt, char ret[], size_t len)
        {
            char cmd[256]     = {0};
            char line[2][256] = {{0}};
            // char addrline[32] = {0};
            int idx = 0;
            int i   = 0;
            FILE* file;
            char* offset = bt->offset;

            if (bt == NULL)
            {
                printf("[%s][%d]Error bt:%p,exe:%p\n", __FUNCTION__, __LINE__, bt, bt->exe);
                return;
            }

            if(offset < lib[idx].offset_start || offset > lib[idx].offset_end)
            {
                ++idx;
                while(idx < max_lib && (offset < lib[idx].offset_start || offset > lib[idx].offset_end))
                    ++idx;
                if(idx == max_lib)
                    return;
                offset = (char*)(offset - lib[idx].offset_start);
            }

            snprintf(cmd, sizeof(cmd), "addr2line -Cif -e %s %p ", bt->exe, offset);
            printf("{cmd:%s}\n", cmd);
            file = popen(cmd, "r");
            if(file == NULL) return;

            while(i < 2 && NULL != fgets(line[i], 256, file)) ++i;

            pclose(file);
            line[0][strlen(line[0]) - 1] = 0;
            if(ret == NULL || len == 0)
            {
        #ifdef __cplusplus
                printf("%s %s", line[0], line[1]);
        #else
                printf("%s() %s", line[0], line[1]);
        #endif
            }
            else
            {
        #ifdef __cplusplus
                snprintf(ret, len, "%s %s", line[0], line[1]);
        #else
                snprintf(ret, len, "%s() %s", line[0], line[1]);
        #endif
            }
        }
        static string ShowStack(void)
        {
            string sResult = "";
            void *buffer[BACKTRACE_SIZE + 1] = { 0 };
            memset(buffer, 0, BACKTRACE_SIZE + 1);
            int n = backtrace(buffer, BACKTRACE_SIZE);
            char **symbols = backtrace_symbols(buffer, n);
            bt_t bt;
            memset(&bt, 0, sizeof(bt));

            if(nullptr == symbols)
            {
                perror("backtrace symbols");
                exit(EXIT_FAILURE);
            }
            for (int i = 0; i < n; ++i)
            {
                // int num = sscanf(symbols[i], "%[^(]%*[^ ] [%p]", bt.exe, &bt.offset);
                char ret[1025] = { 0 };
                output_addrline(&bt, ret, 1024);
                std::string sMsg = (boost::format("%d: %s \t%s\n") % i % symbols[i] % ret).str();
                sResult += sMsg;
                // printf("%s \t{%d,%s,%p} \t%s\n", symbols[i], num, bt.exe, bt.offset, ret);
            }
            free(symbols);
            return sResult;
        }

        // 信号处理
        static void sigsegv_handler(int signo)
        {
            // if (signo == SIGSEGV || signo == SIGBUS)
            {
                CTempLog::WriteInSubDirByDate(s_strCrashTmpLogPath, "crash", "Signo-" + std::to_string(signo), __CURR_CODE_PLACE_C__, ShowStack());
                exit(EXIT_FAILURE);
            }
        }

        static void SetExceptFilter(string sPath)
        {
            if (boost::filesystem::exists(sPath)) s_strCrashTmpLogPath = sPath;
            signal(SIGSEGV, sigsegv_handler);
            signal(SIGBUS, sigsegv_handler);
            signal(SIGABRT, sigsegv_handler);
            dump_library_maps();
        }
#endif
    };
}
