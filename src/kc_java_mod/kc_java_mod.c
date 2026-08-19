#include "std.h"
#include "load_kc.h"
#include "func_cb.h"

// 路径
static char s_apiSoDir[301] = { 0 };
static char s_apiSoFile[500] = { 0 };

// 写日志。0Trace、1Debug、2Info、3Warning、4Error
int WriteLog(int lv, const char* msg, const char* pos, const char* type);

///////////////////////////////////////////////// 引出函数 ////////////////////////////////////////////////////
// 获取api动态库的版本
int GetApiVersion(const char* pth)
{
    memcpy(s_apiSoDir, pth, (int)fmin((int)strlen(pth), 300));
    sprintf(s_apiSoFile, "%s/%s", s_apiSoDir, "kc_websrv_c_api");
    printf("*[JavaMod] Java Mod Version: %d --- %s\n", KCAPIVERSION, s_apiSoFile);
    // WriteLog(0, "GetApiVersion", __FUNCTION__, s_apiSoFile);
    // 加载knewcode模块
    const char* sErr = KC_LoadLib(s_apiSoFile);
    if (NULL != sErr && strlen(sErr) > 0)
        WriteLog(4, sErr, __FUNCTION__, s_apiSoFile);
    return KCAPIVERSION;
}

// 设置回调函数指针
int SetCBFuncPointer(int argCount, const char* flag, void* func)
{
    return __SetCBFuncPointer(argCount, flag, func);
}

// 初始化
int Init(void)
{
    printf("*[JavaMod] Init...\n");
    // 调用初始化
    if (KC_HasLoad())
    {
        // 初始化
        typedef int (*FInit)(void);
        return ((FInit)KC_GetLibFunc("Init"))();
    }
    else WriteLog(4, "No Load kc_websrv_c_api.", __FUNCTION__, s_apiSoFile);
    return -1;
}
// 释放
int Free(void)
{
    printf("*[JavaMod] Free.\n");
    if (KC_HasLoad())
    {
        typedef int (*FFree)(void);
        int iResult = ((FFree)KC_GetLibFunc("Free"))();
        KC_UnLoadLib();
        return iResult;
    }
    return -1;
}

// 请求
int Request(long int re)
{
    printf("*[JavaMod] Request: %ld >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n", re);
    int iResult = 5;    // 处理失败
    if (KC_HasLoad())
    {
        typedef int (*FRequest)(long int);
        iResult = ((FRequest)KC_GetLibFunc("Request"))(re);
        printf("*[JavaMod] Respond: %ld <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< \n", re);
    }
    return iResult;
}

// 写日志
int WriteLog(int lv, const char* msg, const char* pos, const char* type)
{
    char sMsg[1025] = { 0 }, sPos[1025] = { 0 }, sType[1025] = { 0 };
    memcpy(sMsg, msg, (int)fmin((int)strlen(msg), 1024));
    memcpy(sPos, pos, (int)fmin((int)strlen(pos), 1024));
    memcpy(sType, type, (int)fmin((int)strlen(type), 1024));
    if (KC_HasLoad())
    {
        typedef int (*FWriteLog)(int, const char*, const char*, const char*);
        return ((FWriteLog)KC_GetLibFunc("WriteLog"))(lv, sMsg, sPos, sType);
    }
    else
    {
        char sLogFile[500] = { 0 };
        sprintf(sLogFile, "%s/kc_java_mod.%d.log", s_apiSoDir, KCAPIVERSION);
        FILE *fp = fopen(sLogFile, "w"); // 打开文件用于写入，如果文件不存在则创建
        if (fp != NULL)
        {
            printf("*[JavaMod] %s <%s>\n%s\n", sMsg, sPos, sType);
            fprintf(fp, "%lld: (%d) %s <%s>\n%s", (long long)(time(NULL)), lv, sMsg, sPos, sType);
            fclose(fp); // 关闭文件
            return 0;
        }
    }
    return -1;
}
