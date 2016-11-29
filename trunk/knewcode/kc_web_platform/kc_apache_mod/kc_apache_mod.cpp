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

extern "C"
{
    string g_strKCFullPath = "KC系统路径";
    string g_strPageRootFullPath = "主页路径";

    // 实际的处理函数
    static int dlib_kc_handler(request_rec *r)
    {
        // 判断是否本模块需要处理
        if (nullptr == r->handler || strcmp("dlib_kc_extname", r->handler) != 0)
            return DECLINED;
        // 加载核心处理功能
        static CAPACHEWork g_work(g_strKCFullPath, g_strPageRootFullPath);
        g_work.Work(*r);
        // 返回
        return OK;
    }

    // 钩子函数，用于注册处理函数
    static void dlib_kc_register_hooks(apr_pool_t *p)
    {
        cout << "begin" << endl;
        ap_hook_handler(dlib_kc_handler, NULL, NULL, APR_HOOK_MIDDLE);
    }

    // 读取配置的KC系统路径
    static const char *get_mod_kc_path(cmd_parms *cmd, void *mconfig, const char *name)
    {
        //char *szSrvRootPath = ap_server_root_relative(cmd->pool, "");     // apache的安装路径
        g_strKCFullPath = name;    // 从配置中读取KC系统路径
        // 格式化路径字符串
        g_strKCFullPath = CUtilFunc::FormatPath(g_strKCFullPath);
        return NULL;
    }

    // 读取配置的主页目录
    static const char *get_mod_rt_path(cmd_parms *cmd, void *mconfig, const char *name)
    {
        g_strPageRootFullPath = name;    // 从配置中读取主页目录
        // 格式化路径字符串
        g_strPageRootFullPath = CUtilFunc::FormatPath(g_strPageRootFullPath);
        return NULL;
    }

    //配置文件http.conf中的配置项的读取方法。
    static const command_rec authn_jira_cmds[] =
    {
        AP_INIT_TAKE1("dlib_kc_path", get_mod_kc_path, NULL, OR_FILEINFO, "dlib_kc_path"),
        AP_INIT_TAKE1("DocumentRoot", get_mod_rt_path, NULL, OR_FILEINFO, "DocumentRoot"),
        { NULL }
    };

    // 注册钩子函数的结构
    module AP_MODULE_DECLARE_DATA dlib_kc_module = {
        STANDARD20_MODULE_STUFF,
        NULL,                   /* create per-dir    config structures */
        NULL,                   /* merge  per-dir    config structures */
        NULL,                   /* create per-server config structures */
        NULL,                   /* merge  per-server config structures */
        authn_jira_cmds,        /* table of config file commands       */
        dlib_kc_register_hooks  /* register hooks                      */
    };
}
