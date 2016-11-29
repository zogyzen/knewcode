/*
配置IIS7：
默认网站->处理程序映射->添加模块映射
    可执行文件 = C:\KnewCode\kc_fastcgi_mod.exe
    请求路径   = *.kc
    模块       = FastCgiModule
*/

#include "std.h"
#include "fastcgi_work.h"

// 主程序
int main(int argc, char* argv[])
{
    // 获取路径
    string sPath = "";
    if (argc > 0)
    {
        filesystem::path MainPath(argv[0]);
        sPath = MainPath.branch_path().string();
        sPath = CUtilFunc::FormatPath(sPath);
    }
    // 创建核心处理功能
    CFastCGIWork CgiWork(sPath.c_str());
    // 线程管理
    CSeveralThread<0> ThrdWork;
    // 初始化
    FCGX_Init();
    // 处理函数
    auto Ftor = boost::lambda::bind(&CFastCGIWork::Work, &CgiWork);
    Ftor();
    // 启动50个线程
    //for (int i = 0; i < 50; ++i) ThrdWork.NewThread(Ftor);
    // 等待所有线程结束
    //ThrdWork.JoinAllThread();
    return 0;
}
