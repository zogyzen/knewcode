#include "std.h"
#include "websrv_work.h"

int main(int argc, char *argv[])
{
    std::string sCommFmt = R"(/\*((?!\*/)[\s\S])*\*/)";
    //sSQL = boost::regex_replace(sSQL, boost::regex(sCommFmt), " ");
    boost::regex pattern(sCommFmt);
    cout << "begin" << endl;
    try
    {
        string sCfgFile = argc >= 2 ? argv[1] : "../website/my-prj.xml";
        g_work.reset(new CWebSrvWork(argv[0], sCfgFile));
        CAutoRelease _auto([=](){g_work.reset(); });
        g_work->Init();
        CAutoRelease _auto2([=](){g_work->Free(); });
        for (string sIn; "quit" != sIn; cin >> sIn)
            boost::this_thread::sleep(boost::posix_time::milliseconds(999));
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
    cout << "end" << endl;
    return 0;
}
