#include "ctrl_prjs.h"

// 创建项目
void CCtrlPrjs::CreatePrj(ICtrlApiData& objCtrlD, map<string, string>& /*mpCfgs*/)
{
    IActionData& act = objCtrlD.ActionData();
    // 用户参数
    string sName = objCtrlD.GetParmJson("s_projectname").GetStr();
    string sRootDir = act.GetSingleInfo("PlatformRoot");
    // 后端目录使用Git管理
    auto fInitGit = [&](string sCmd, string sSub)
    {
        string cmd = (boost::format(R"($cmd: %s %s "%s/prjs/%s/website/%s";)") % GitExe(sRootDir) % sCmd % sRootDir % sName % sSub).str();
        objCtrlD.ExecBundle(c_KCOSExecSrvGUID, "cmd", "console", cmd.c_str());
    };
    //fInitGit("clone http://mysrv:202004@2.17cpp.com:8880/r/MakeKCPrjTest.git", "backend");
    fInitGit("init", "backend");
    //fInitGit("config --global --add safe.directory", "backend");
    // 创建工程目录
    auto fCreateDir = [&](string sSub)
    {
        string sPrjDir = (boost::format("$path: %s/prjs/%s/website/%s;") % sRootDir % sName % sSub).str();
        objCtrlD.ExecBundle(c_KCOSFileSrvGUID, "disk", "CreateDir", sPrjDir.c_str());
    };
    fCreateDir("backend/kc/api");
    fCreateDir("backend/kc/inside");
    fCreateDir("backend/kc/test");
    fCreateDir("frontend");
    fCreateDir("upload");
    fCreateDir("so");
    // 拷贝默认目录
    auto fCopyDir = [&](string sSub)
    {
        string sPrjDir = (boost::format("$path: %s/prjs/%s/website/%s;") % sRootDir % sName % sSub).str();
        objCtrlD.ExecBundle(c_KCOSFileSrvGUID, "disk", "CreateDir", sPrjDir.c_str());
    };
    // 创建工程文件
}
