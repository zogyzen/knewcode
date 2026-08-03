/*
apache模块开发：
#配置httpd.conf或apache2.conf

#新增
LoadModule dlib_kc_module "D:/mycode/knewcode_project/trunk/debug/kc_apache_mod.dll"
AddHandler dlib_kc_extname .kc
dlib_kc_path "D:/mycode/knewcode_project/trunk/debug/"

#修改（Linux下，需设置主页目录权限，添加「www-data」用户组的读写权限）
DocumentRoot "C:/KCPage/"
<IfModule dir_module>
    DirectoryIndex index.kc index.html
</IfModule>
<Directory />
	<FilesMatch "\.(dll|so)">
		Order Deny,Allow
		Deny from all
	</FilesMatch>
    Options FollowSymLinks
    AllowOverride None
    Order deny,allow
    Allow from all
</Directory>
*/
/*
Windows下CodeBlocks的调试：
1、启动Apache的httpd进程，找到2个httpd进程中线程数最多的PID值（用“httpd -X”命令只启动1个进程）；
2、打开CodeBlocks工具的菜单项【Debug -> Attach to process...】，输入上述的PID值；
3、启动【kc_apache_mod】工程，开始调试。
*/

#include "std.h"
#include "apache_work.h"

// Apache模块方法
// 转换为完整路径
static string toFullPath(cmd_parms *cmd, string sPth);

// 各路径
string g_strServerRootPath = "";             // apache主目录
string g_strKCFullPath = "";                 // KC系统路径
string g_strKCConfigFile = "";               // 配置文件完整路径，默认为当前“KC系统路径”下的“config.xml”
string g_strPageRootFullPath = "主页路径";

extern "C"
{
    // 钩子函数，用于注册处理函数
    void dlib_kc_register_hooks(apr_pool_t *p);
    // 读取LoadModule指令信息
    const char* get_mod_load_mod(cmd_parms *cmd, void *mconfig, const char *name, const char* filename);
    // 读取配置的KC系统路径
    const char* get_mod_kc_path(cmd_parms *cmd, void *mconfig, const char *name);
    // 读取配置的主页目录
    const char* get_mod_rt_path(cmd_parms *cmd, void *mconfig, const char *name);
    // 读取配置的端口
    const char* get_mod_rt_port(cmd_parms *cmd, void *mconfig, const char *name);
    // 读取配置的KC配置文件路径
    const char* get_mod_kc_config(cmd_parms *cmd, void *mconfig, const char *name);

    //配置文件http.conf中的配置项的读取方法。
    static const command_rec authn_jira_cmds[] =
    {
        AP_INIT_TAKE2("LoadModule", (cmd_func)get_mod_load_mod, NULL, RSRC_CONF | EXEC_ON_READ, "dlib_kc_module"),
        AP_INIT_TAKE1("Listen", (cmd_func)get_mod_rt_port, NULL, OR_FILEINFO, "Listen"),
        AP_INIT_TAKE1("DocumentRoot", (cmd_func)get_mod_rt_path, NULL, OR_FILEINFO, "DocumentRoot"),
        AP_INIT_TAKE1("dlib_kc_path", (cmd_func)get_mod_kc_path, NULL, OR_FILEINFO, "dlib_kc_path"),
        AP_INIT_TAKE1("dlib_kc_config", (cmd_func)get_mod_kc_config, NULL, OR_FILEINFO, "dlib_kc_config"),
        { NULL }
    };

    // 注册钩子函数的结构
    //AP_DECLARE_MODULE(dlib_kc)
    module AP_MODULE_DECLARE_DATA dlib_kc_module = {
        STANDARD20_MODULE_STUFF,
        NULL,                   /* create per-dir    config structures */
        NULL,                   /* merge  per-dir    config structures */
        NULL,                   /* create per-server config structures */
        NULL,                   /* merge  per-server config structures */
        authn_jira_cmds,        /* table of config file commands       */
        dlib_kc_register_hooks  /* register hooks                      */
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 读取配置的apache扩展模块（无效）
    const char* get_mod_load_mod(cmd_parms *cmd, void *mconfig, const char *name, const char* filename)
    {
        cout << "LoadModule - " << name << " \t " << filename << "\t" << cmd->path << "\t" << mconfig << endl;
        return NULL;
    }

    // 读取配置的主页目录
    const char* get_mod_rt_path(cmd_parms *cmd, void *mconfig, const char *name)
    {
        g_strPageRootFullPath = CUtilFunc::PCharSafeToStr(name);
        // g_strPageRootFullPath = toFullPath(cmd, g_strPageRootFullPath);
        cout << "DocumentRoot - " << g_strPageRootFullPath << " (" << name << ")\t" << mconfig << endl;
        return NULL;
    }

    // 读取端口
    const char* get_mod_rt_port(cmd_parms *cmd, void *, const char *name)
    {
        boost::filesystem::path pth(cmd->directive->filename);
        string sFilename = boost::algorithm::to_lower_copy(pth.filename().string());
        string sProto = sFilename.find("ssl") != string::npos ? "https" : "http";
        cout << "URL - \t" << sProto << "://127.0.0.1:" << name << "/" << endl;
        return NULL;
    }

    // 读取配置的KC系统路径
    const char* get_mod_kc_path(cmd_parms *cmd, void *mconfig, const char *name)
    {
        g_strKCFullPath = CUtilFunc::PCharSafeToStr(name);
        // g_strKCFullPath = toFullPath(cmd, g_strKCFullPath);
        cout << "dlib_kc_path - " << g_strKCFullPath << " (" << name << ")\t" << mconfig << endl;
        return NULL;
    }

    // 读取配置的KC配置文件路径
    const char* get_mod_kc_config(cmd_parms *cmd, void *mconfig, const char *name)
    {
        g_strKCConfigFile = CUtilFunc::PCharSafeToStr(name);
        // g_strKCConfigFile = toFullPath(cmd, g_strKCConfigFile);
        cout << "dlib_kc_config - " << g_strKCConfigFile << " (" << name << ")\t" << mconfig << endl;
        return NULL;
    }

    // ***初始化***
    //void SetExceptFilter(string);
    void dlib_kc_child_init(apr_pool_t *pchild, server_rec *s)
    {
        try
        {
            // apache主目录
            if (g_strServerRootPath.empty())
            {
                g_strServerRootPath = CUtilFunc::FormatPath(ap_server_root_relative(s->process->pool, ""));     // apache的安装路径
                cout << "ServerRoot - " << g_strServerRootPath << endl;
            }
            // 加载主处理模块
            cout << "load knewcode mod begin\t" << pchild << endl;
            // ExceptBacktrace::SetExceptFilter(g_strKCFullPath);
            g_work.reset(new CApacheWork(dlib_kc_module, *s));
            CApacheWork::m_self = g_work;
            g_work->Init();
            cout << endl << "load knewcode mod finish\t" << pchild << endl << endl;
        }
        catch (std::exception &ex)
        {
            cout << "Init Error - " << g_strKCFullPath << " \t" << ex.what() << endl;
            CTempLog::Write("Init Error", __CURR_CODE_PLACE_C__, ex.what());
            g_work.reset();
            throw;
        }
        catch (...)
        {
            cout << "Init Error - " << g_strKCFullPath << endl;
            CTempLog::Write("Init Error", __CURR_CODE_PLACE_C__);
            g_work.reset();
            throw;
        }
    }

    // ***实际的处理函数***
    int dlib_kc_handler(request_rec *r)
    {
        // 判断是否本模块需要处理
        if (nullptr != r->handler && strcmp("dlib_kc_extname", r->handler) == 0)
            try
            {
                cout << endl << (format("[%ld]request: \t%s \t\t%s \t\t%s:%d") % r->connection->id % CUtilFunc::PCharSafeToStr(r->handler) % CUtilFunc::PCharSafeToStr(r->unparsed_uri) % CUtilFunc::PCharSafeToStr(r->connection->client_ip) % r->connection->client_addr->port).str() << endl << (nullptr != r->filename ? r->filename : "") << endl;
                // 加载核心处理功能
                int iErrCode = -5;
                if (g_work.get() == nullptr || (iErrCode = g_work->Work(*r)) > 0)
                    throw std::runtime_error("Uninitialized - " + std::to_string(iErrCode));
                // 返回
                if (200 != r->status && 101 != r->status) cout << r->status << endl;
                return OK;
            }
            catch (std::exception &ex)
            {
                cout << "Work Error - " << r->unparsed_uri << " \t" << ex.what() << endl;
                if (g_work.get() != nullptr && g_work->m_load.WriteLog(4, "Work Error", __CURR_CODE_PLACE_C__, (r->unparsed_uri + string("\n") + ex.what()).c_str()) != 0)
                    CTempLog::WriteInDir(boost::filesystem::exists(g_strKCFullPath) ? g_strKCFullPath : g_strPageRootFullPath, "Work Error", __CURR_CODE_PLACE_C__, r->unparsed_uri + string("\n") + ex.what());
                throw;
            }
            catch (...)
            {
                cout << "Work Error - " << r->unparsed_uri << endl;
                if (g_work.get() != nullptr && g_work->m_load.WriteLog(4, "Work Error", __CURR_CODE_PLACE_C__, r->unparsed_uri) != 0)
                    CTempLog::WriteInDir(boost::filesystem::exists(g_strKCFullPath) ? g_strKCFullPath : g_strPageRootFullPath, "Work Error", __CURR_CODE_PLACE_C__, r->unparsed_uri);
                throw;
            }
        else return DECLINED;
    }
    int dlib_kc_quick_handler(request_rec *r, int lookup_uri)
    {
        // 判断是否本模块需要处理
        if (nullptr != r->handler && strcmp("dlib_kc_extname", r->handler) == 0)
        {
            cout << endl << (format("%d[%ld]request: \t%s \t\t%s \t\t%s:%d") % lookup_uri % r->connection->id % CUtilFunc::PCharSafeToStr(r->handler) % CUtilFunc::PCharSafeToStr(r->unparsed_uri) % CUtilFunc::PCharSafeToStr(r->connection->client_ip) % r->connection->client_addr->port).str() << endl << (nullptr != r->filename ? r->filename : "") << endl;
            return OK;
        }
        else return DECLINED;
    }

    // 链接关闭前
    int dlib_kc_pre_close_connection(conn_rec*)
    {
        //cout << "dlib_kc_pre_close_connection: " << boost::this_process::get_id() << "_" << c->id << "\t\t" << c->aborted << endl;
        // if (g_work.get() != nullptr) g_work->PreClose(c);
        return DONE;
    }

    // 钩子函数，用于注册处理函数
    void dlib_kc_register_hooks(apr_pool_t *p)
    {
        // 设置windows控制台显示utf8中文编码
        #ifdef WIN32
        std::system("chcp 65001");
        #endif
        // 注册钩子函数
        cout << "load knewcode mod begin [" << ap_find_module_short_name(dlib_kc_module.module_index) << " - " << ap_find_module_name(&dlib_kc_module) << "] "
             << p << "\t\t" << ap_get_server_description() << "\t" << ap_get_server_banner() << "\t" << ap_get_server_built() << endl;
        //ap_hook_quick_handler(dlib_kc_quick_handler, NULL, NULL, APR_HOOK_MIDDLE);
        ap_hook_handler(dlib_kc_handler, NULL, NULL, APR_HOOK_MIDDLE);
        ap_hook_child_init(dlib_kc_child_init, NULL, NULL, APR_HOOK_MIDDLE);
        //ap_hook_child_exit(dlib_kc_child_exit, NULL, NULL, APR_HOOK_MIDDLE);
        //ap_hook_pre_close_connection(dlib_kc_pre_close_connection, NULL, NULL, APR_HOOK_MIDDLE);
    }
}

// 转换为完整路径
static string toFullPath(cmd_parms *cmd, string sPth)
{
    if (g_strServerRootPath.empty())
    {
        g_strServerRootPath = CUtilFunc::FormatPath(ap_server_root_relative(cmd->pool, ""));     // apache的安装路径
        cout << "ServerRoot - " << g_strServerRootPath << endl;
    }
    try
    {
        sPth = CUtilFunc::FormatPath(sPth);
        if (!boost::filesystem::path(sPth).is_absolute())
            sPth = boost::filesystem::canonical(g_strServerRootPath + "/" + sPth).string();
    }
    catch (std::exception &ex)
    {
        cout << sPth << "\t\t" << ex.what() << endl;
        throw;
    }
    catch (...)
    {
        cout << sPth << endl;
        throw;
    }
    return sPth;
}
