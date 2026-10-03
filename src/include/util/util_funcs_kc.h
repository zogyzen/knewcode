#pragma once

#include "util/util_funcs.h"
#include "kc_web/kc_main_work_i.h"
#include "kc_web/kc_web_work_i.h"

namespace KC
{
    // 业务相关的公共函数
    struct CUtilFuncKC
	{
        // 添加响应头
        static void AddCfgHeader(std::map<string, string> &cfgHeader, string key, string val)
        {
            if (!key.empty() /*&& !val.empty()*/)
            {
                auto iter = cfgHeader.find(key);
                if (cfgHeader.end() == iter)
                    cfgHeader.insert(make_pair(key, val));
                else iter->second = val;
            }
        }
        // 得到配置的响应头
        static void GetCfgHeadeBase(std::map<string, string> &cfgHeader, IBundleContext& context, std::string sBundleName)
        {
            string sCfgMod = string("Config.Modules.") + sBundleName;
            // 配置中的响应头信息
            string sHeaderNode = sCfgMod + ".Header";
            for (int i = 0, c = context.GetCfgSubCount(sHeaderNode.c_str()); i < c; ++i)
                if (context.IsCfgSubValid(sHeaderNode.c_str(), i))
                    AddCfgHeader(cfgHeader, context.GetCfgSubInfo(sHeaderNode.c_str(), i, "key", ""), context.GetCfgSubInfo(sHeaderNode.c_str(), i, "value", ""));
        }

        // 得到配置的静态响应头
        static void GetCfgHeadeStatic(std::map<string, string> &cfgHeader, IBundleContext& context, std::string sBundleName)
        {
            // 版本信息
            AddCfgHeader(cfgHeader, "Knewcode-Api-Ver", context.VersionInfo());
            AddCfgHeader(cfgHeader, "Server-Api-Ext", c_DefaultWorkUriExtension);
            // 配置中的响应头信息
            GetCfgHeadeBase(cfgHeader, context, sBundleName);
        }

        // 得到配置的动态响应头

    };
}
