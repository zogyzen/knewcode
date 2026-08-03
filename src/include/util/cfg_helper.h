#pragma once

#include <string>
#include <filesystem>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "util/util_funcs.h"

namespace KC
{
    // 配置帮助类
    class CfgHelper
    {
    public:
        // 构造函数
        CfgHelper(void) = default;

        // 初始化
        std::string init(std::string sCfgFile)
        {
            // 配置文件
            m_cfgFile = sCfgFile;
            #ifdef WIN32
            sCfgFile  = CUtilFunc::GbkToUtf8(sCfgFile);
            #endif
            std::string sWarn;
            std::filesystem::path *u8_path = nullptr;
            try
            {
                u8_path = new std::filesystem::path(sCfgFile, std::locale("zh_CN.UTF8"));
            }
            catch (...)
            {
                u8_path = new std::filesystem::path(sCfgFile);
                sWarn = "系统不存在编码：zh_CN.UTF8";
                std::cout << "配置文件错误：" << m_cfgFile << "。" << sWarn << std::endl;
            }
            std::shared_ptr<std::filesystem::path> _autoDel(u8_path);
            if (!std::filesystem::exists(*u8_path)) return "不存在配置文件：" + m_cfgFile + ". " + sWarn;
            // 读配置
            try
            {
                //boost::property_tree::read_xml(m_cfgFile, m_cfgPt, boost::property_tree::xml_parser::trim_whitespace);
                boost::property_tree::read_xml(u8_path->string(), m_cfgPt);
                if (m_cfgPt.empty()) return "空配置文件：" + m_cfgFile;
                return "";
            }
            catch (std::exception& ex)
            {
                return "配置文件错误：" + m_cfgFile + ". " + ex.what() + ". " + sWarn;
            }
            catch (...)
            {
                return "配置文件错误：" + m_cfgFile + "..." + sWarn;
            }
        }
        void free(void)
        {
            // 删除配置
            m_cfgPt.clear();
        }

        // 配置信息
        const char* GetCfgInfo(std::string strNode, std::string strAttr, std::string sDefault = "") const
        {
            static thread_local std::string sResult;
            sResult = sDefault;
            if (!strNode.empty() && !m_cfgPt.empty())
            {
                std::string strNodeAttr = strNode;
                if (!strAttr.empty())
                    strNodeAttr = strNode + ".<xmlattr>." + strAttr;
                if (m_cfgPt.get_child_optional(strNodeAttr))
                    sResult = m_cfgPt.get<std::string>(strNodeAttr);
            }
            return sResult.c_str();
        }
        bool SetCfgInfo(std::string strNode, std::string strAttr, std::string strVal)
        {
            bool bResult = false;
            if (!strNode.empty() && !m_cfgPt.empty())
            {
                std::string strNodeAttr = strNode;
                if (!strAttr.empty())
                    strNodeAttr = strNode + ".<xmlattr>." + strAttr;
                if (m_cfgPt.get_child_optional(strNodeAttr))
                {
                    m_cfgPt.put(strNodeAttr, strVal);
                    boost::property_tree::write_xml(m_cfgFile, m_cfgPt, std::locale(), boost::property_tree::xml_writer_make_settings<std::string> ('\t', 1));
                    bResult = true;
                }
            }
            return bResult;
        }
        // 子配置项
        int GetCfgSubCount(std::string strNode) const
        {
            int iResult = 0;
            if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
                iResult = m_cfgPt.get_child(strNode).size();
            return iResult;
        }
        bool IsCfgSubNode(std::string strNode, int id) const
        {
            bool bResult = false;
            if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
            {
                auto ptNode = m_cfgPt.get_child(strNode);
                auto iter = ptNode.begin();
                for (; ptNode.end() != iter && id > 0; ++iter, --id);
                if (ptNode.end() != iter && 0 == id)
                    bResult = "<xmlcomment>" != iter->first && "<xmlattr>" != iter->first;
            }
            return bResult;
        }
        const char* GetCfgSubName(std::string strNode, int id) const
        {
            static thread_local std::string sResult;
            sResult.clear();
            if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
            {
                auto ptNode = m_cfgPt.get_child(strNode);
                auto iter = ptNode.begin();
                for (; ptNode.end() != iter && id > 0; ++iter, --id);
                if (ptNode.end() != iter && 0 == id && "<xmlcomment>" != iter->first)
                    sResult = iter->first;
            }
            return sResult.c_str();
        }
        const char* GetCfgSubInfo(std::string strNode, int id, std::string strAttr, std::string sDefault = "") const
        {
            static thread_local std::string sResult;
            sResult = sDefault;
            if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
            {
                auto ptNode = m_cfgPt.get_child(strNode);
                auto iter = ptNode.begin();
                for (; ptNode.end() != iter && id > 0; ++iter, --id);
                if (ptNode.end() != iter && 0 == id && "<xmlcomment>" != iter->first)
                {
                    if (strAttr.empty())
                        sResult = iter->second.get_value<std::string>();
                    else
                    {
                        auto ptAttr = iter->second.get_child_optional("<xmlattr>." + strAttr);
                        if (ptAttr) sResult = ptAttr.get().get_value<std::string>();
                    }

                }
            }
            return sResult.c_str();
        }

    private:
        // 配置文件
        std::string m_cfgFile;
        boost::property_tree::ptree m_cfgPt;
    };
}
