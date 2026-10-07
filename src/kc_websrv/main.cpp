#include "std.h"
#include "websrv_work.h"

int main(int argc, char *argv[])
{
    cout << "begin" << endl;
    try
    {
        // 测试boost库是否正常运行
        // std::string sCommFmt = R"(/\*((?!\*/)[\s\S])*\*/)";
        // //sSQL = boost::regex_replace(sSQL, boost::regex(sCommFmt), " ");
        // boost::regex pattern(sCommFmt);

        string sCfgFile = argc >= 2 ? argv[1] : "../website/my-prj.xml";
        g_work.reset(new CWebSrvWork(argv[0], sCfgFile));
        CAutoRelease _auto([=](){g_work.reset(); });
        g_work->Init();
        CAutoRelease _auto2([=](){g_work->Free(); });
        g_work->Block();
    }
    catch (std::exception &ex)
    {
        cout << ex.what() << "!" << endl;
        WriteLog(typeid(ex).name(), __CURR_CODE_PLACE_C__, ex.what());
    }
    catch (...)
    {
        cout << "Unknown error!" << endl;
        WriteLog("kc_websrv", __CURR_CODE_PLACE_C__, "Unknown error!");
    }
    cout << endl << "end" << endl;
    return 0;
}
