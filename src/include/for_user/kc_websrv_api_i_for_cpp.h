#pragma once

#include "kc_websrv_api_cb.h"

extern "C"
{
    // 1、获取api动态库的版本。版本初始为1。当调用方的回调函数或api动态库的引出函数有格式变化时，双方需将各自的版本加1。双方的版本不一致时，调用方需报错退出。参数为：动态库路径。
    int GetApiVersion(const char* pth);

    // 2、设置回调函数指针。参数为：本回调函数的参数数量；标记名称；函数指针（见“kc_websrv_api_cb.h”文件）。返回错误码：0无错误；>0设置失败（参数数量或标识名称不一致）。
    int SetCBFuncPointer(int, const char*, void*);

    // 3、初始化，系统启动时调用。返回错误码：0初始化成功；-1未初始化（有重要的回调函数未设置）；>0其他错误。
    int Init(void);
    // 6、释放，系统关闭前的清理。返回错误码：0无错误。
    int Free(void);

    // 4、请求。参数为请求的句柄。返回错误码：0处理成功；-1不处理（请求页面的扩展名不是“.kc”）；>0处理失败。
    int Request(intptr_t);

    // 5、写日志。参数依次为：日志等级（0 trace, 1 debug, 2 info, 3 warning, 4 error）；日志信息；日志位置（函数名称、文件名和行号）；错误类型（异常类名称等）
    int WriteLog(int, const char*, const char*, const char*);
}
