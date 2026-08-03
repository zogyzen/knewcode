#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <map>
#include <memory>
#include <regex>
#include <functional>

#include <boost/any.hpp>
#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/json.hpp>
#include <boost/locale.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/regex.hpp>

#include "for_user/page_interface.h"
#include "util/util_funcs_ex.h"
#include "util/request_respond_tmp.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/kc_web_work_i.h"
#include "kc_controller/kc_sql_i.h"

namespace KC
{
    // 控制器公共函数
    struct CCtrlCommon
    {
        // 设置参数值
        static void SetParmVal(ICtrlApiData& objCtrlD, std::string name, std::string val)
        {
            // 全局参数
            if (objCtrlD.ActionData().IsGlobalVal(name.c_str()))
            {
                objCtrlD.ActionData().SetGlobalVal(name.c_str(), val.c_str());
            }
            // 用户参数
            else
            {
                objCtrlD.JsonRespond().SetStr(name.c_str(), val.c_str());
            }
        }


        // 获取本次会话编号
        static const char* GetKCSSID(IActionData& act)
        {
            //  session编号
            const char* pSessID = act.GetKCSSID();
            if (nullptr == pSessID || strlen(pSessID) == 0)
                throw TException(2, __CURR_CODE_PLACE_C__, std::string("Not Exists ") + c_RESTful_KCSessoinID);
            return pSessID;
        }

        // 生成结果json
        static const char* GetRespondJson(int errNo, std::string errMsg, std::string errNoName = c_RESTful_errCode, std::string errMsgName = c_RESTful_errMsg)
        {
            // static thread_local std::string sResult;
            // boost::algorithm::replace_all(errMsg, "\"", "\\\"");
            // boost::algorithm::replace_all(errMsg, "\n", "\\n");
            // boost::algorithm::replace_all(errMsg, "\r", "\\r");
            // boost::algorithm::replace_all(errMsg, "\t", "\\t");
            // sResult = (boost::format(R"({"%s":%d,"%s":"%s"})") % errNoName % errNo % errMsgName % errMsg).str();
            // return sResult.c_str();
            static thread_local std::string sResult;
            boost::json::object jResult = {{errNoName, errNo}, {errMsgName, errMsg}};
            sResult = boost::json::serialize(jResult);
            return sResult.c_str();
        }

        // 获取控制器
        typedef map<string, std::shared_ptr<IKCController>> TKCControllerMap;
        static std::string GetCtrlNodeName(IKCCtrlWork& ctrl)
        {
            return std::string("Config.Modules.") + c_LoadOutLibPrefixName + ctrl.getBundle().getName();
        }
        static std::string GetCtrlNodeCtrlsName(IKCCtrlWork& ctrl)
        {
            return CCtrlCommon::GetCtrlNodeName(ctrl) + "." + c_RESTful_controllers;
        }
        static boost::property_tree::ptree& GetCtrlNode(boost::property_tree::ptree &pt, IKCCtrlWork& ctrl, std::string sCfgFile, std::string &sSrvAliasList)
        {
            if (pt.empty())
                boost::property_tree::read_xml(sCfgFile, pt);
            // 服务别名
            std::string sCtrlNode = GetCtrlNodeName(ctrl);
            std::string sCtrlNodeSrv = sCtrlNode + "." + c_RESTful_xmlattr + ".srv";
            if (pt.get_child_optional(sCtrlNodeSrv))
                sSrvAliasList = pt.get<string>(sCtrlNodeSrv);
            if (sSrvAliasList.empty()) sSrvAliasList = ctrl.getGUID();
            // 插件包含的控制器子节点列表
            sCtrlNode = GetCtrlNodeCtrlsName(ctrl);
            ctrl.WriteLogDebug(sCtrlNode.c_str(), __CURR_CODE_PLACE_C__, sCfgFile.c_str());
            // boost::property_tree::read_xml(sCfgFile, pt);
            if (!pt.get_child_optional(sCtrlNode)) throw std::runtime_error("Not Exists Node - " + sCtrlNode);
            return pt.get_child(sCtrlNode);
        }
        static boost::property_tree::ptree& GetBundleNode(boost::property_tree::ptree &pt, IKCCtrlWork& ctrl, std::string sCfgFile, std::string &sSrvAliasList)
        {
            if (pt.empty())
                boost::property_tree::read_xml(sCfgFile, pt);
            // 服务别名
            std::string sCtrlNode = GetCtrlNodeName(ctrl);
            std::string sCtrlNodeSrv = sCtrlNode + "." + c_RESTful_xmlattr + ".srv";
            if (pt.get_child_optional(sCtrlNodeSrv))
                sSrvAliasList = pt.get<string>(sCtrlNodeSrv);
            if (sSrvAliasList.empty()) sSrvAliasList = ctrl.getGUID();
            // 插件节点
            ctrl.WriteLogDebug(sCtrlNode.c_str(), __CURR_CODE_PLACE_C__, sCfgFile.c_str());
            // boost::property_tree::read_xml(sCfgFile, pt);
            if (!pt.get_child_optional(sCtrlNode)) throw std::runtime_error("Not Exists Node - " + sCtrlNode);
            return pt.get_child(sCtrlNode);
        }
        static std::string GetAllCtrl(IKCCtrlWork& ctrl, boost::property_tree::ptree &pt, std::string sCfgFile, TKCControllerMap &ctrls,
                               boost::function<IKCController*(std::string, boost::property_tree::ptree&)> fCon)
        {
            std::string sCtrlNode = GetCtrlNodeCtrlsName(ctrl);
            std::string sSrvAliasList;
            std::string sName = c_const_default;
            try
            {
                auto fAddCtrl = [&](boost::property_tree::ptree& pt)
                {
                    ctrl.WriteLogDebug(("------ " + sName).c_str(), __CURR_CODE_PLACE_C__, sCtrlNode.c_str());
                    IKCController *pCtrl = fCon(sName, pt);
                    if (nullptr != pCtrl)
                        ctrls.insert(make_pair(sName, std::shared_ptr<IKCController>(pCtrl)));
                };
                // boost::property_tree::ptree pt;
                try
                {
                    // 插件包含的控制器节点
                    boost::property_tree::ptree& ptCtrlNode = GetCtrlNode(pt, ctrl, sCfgFile, sSrvAliasList);
                    // 循环所有控制器
                    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, ptCtrlNode)
                    {
                        sName = v.first;
                        if (c_RESTful_xmlcomment != sName && c_RESTful_xmlattr != sName)
                            fAddCtrl(v.second);
                    }
                }
                catch (std::runtime_error&)
                {
                    // 插件节点
                    boost::property_tree::ptree& ptBundleNode = GetBundleNode(pt, ctrl, sCfgFile, sSrvAliasList);
                    fAddCtrl(ptBundleNode);
                }
            }
            catch (std::exception &ex)
            {
                ctrl.WriteLogFatal((sCtrlNode + "." + sName).c_str(), __CURR_CODE_PLACE_C__, ex.what());
                throw;
            }
            catch (...)
            {
                ctrl.WriteLogFatal((sCtrlNode + "." + sName).c_str(), __CURR_CODE_PLACE_C__);
                throw;
            }
            return sSrvAliasList;
        }
        static bool hasCtrl(const TKCControllerMap &ctrls, const char* pName)
        {
            if (nullptr == pName || strlen(pName) == 0) return !ctrls.empty();
            else
            {
                auto it = ctrls.find(pName);
                return ctrls.end() != it;
            }
        }
        template<typename TKCSrvException, typename TSrv>
        static IKCController& getCtrl(TSrv &srv, TKCControllerMap &ctrls, const char* pName)
        {
            if (ctrls.empty())
                throw TKCSrvException(ecd_ErrCode_Ctrl_NotAssigned, __CURR_CODE_PLACE_C__, string(srv.getHint("Not_Assigned_Controller_")) + (nullptr != pName ? pName : ""), srv);
            string sName(nullptr != pName && strlen(pName) > 0 ? pName : ctrls.begin()->first);
            auto it = ctrls.find(sName);
            if (ctrls.end() == it || it->second.get() == nullptr)
            {
                srv.WriteLogError(srv.getHint("No_Controller_"), __CURR_CODE_PLACE_C__, pName);
                throw TKCSrvException(ecd_ErrCode_Ctrl_NoExists, __CURR_CODE_PLACE_C__, string(srv.getHint("No_Controller_")) + sName, srv);
            }
            return *it->second;
        }

        // 按字符串，添加直接子项
        static IKCJson& AddSonItem(IKCJson& json, std::string sSonName, bool caseSensitive = false)
        {
            if (sSonName.empty() || !json.IsValid()) return json;
            if (json.GetType() == IKCJson::edtArray)
            {
                int iPos = atoi(sSonName.c_str());
                return json.AddItem(iPos, true);
            }
            return json.AddItem(sSonName.c_str(), false, caseSensitive);
        };
        // 按字符串数组，循环添加子项
        static IKCJson& AddSubItem(IKCJson& json, std::deque<std::string> &subVec, bool caseSensitive = false)
        {
            if (subVec.empty()) return json;
            std::string sFirst = subVec.front();
            IKCJson& jsnFirst = AddSonItem(json, sFirst, caseSensitive);
            subVec.pop_front();
            return subVec.empty() ? jsnFirst : AddSubItem(jsnFirst, subVec, caseSensitive);

        };
        // 按分隔符，循环加入子项
        static IKCJson& AddSubItem(IKCJson& json, std::string sName, std::string sNameSplit = "\\.", bool caseSensitive = false)
        {
            std::deque<std::string> subVec;
            // boost::algorithm::split(vctRsetPos, str, boost::is_any_of(sNameSplit));
            boost::algorithm::split_regex(subVec, sName, boost::regex(sNameSplit));
            return AddSubItem(json, subVec, caseSensitive);
        };
        static std::tuple<IKCJson*, std::string> AddSubItemPath(IKCJson& json, std::string sName, std::string sNameSplit = "\\.", bool caseSensitive = false)
        {
            std::deque<std::string> subVec;
            boost::algorithm::split_regex(subVec, sName, boost::regex(sNameSplit));
            std::string sLeaf;
            if (!subVec.empty())
            {
                sLeaf = subVec.back();
                subVec.pop_back();
            }
            return std::make_tuple(&AddSubItem(json, subVec, caseSensitive), sLeaf);
        };
        static void AddSubItem(IKCJson& json, std::string sName, std::function<void(IKCJson& jsnParent, std::string sLeaf)> fAddVal, std::string sNameSplit = "\\.", bool caseSensitive = false)
        {
            auto [jsnParent, sLeaf] = CCtrlCommon::AddSubItemPath(json, sName, sNameSplit, caseSensitive);
            if (nullptr != jsnParent) fAddVal(*jsnParent, sLeaf);
        };
        // 按字符串，获取直接子项
        static IKCJson& GetSonItem(IKCJson& json, std::string sSonName, bool caseSensitive = false)
        {
            if (sSonName.empty() || !json.IsValid()) return json;
            if (json.GetType() == IKCJson::edtArray)
                return json.GetItem(atoi(sSonName.c_str()));
            return json.GetItem(sSonName.c_str(), caseSensitive);
        };
        // 按字符串数组，循环获取子项
        static IKCJson& GetSubItem(IKCJson& json, std::deque<std::string> &subVec, bool caseSensitive = false)
        {
            if (subVec.empty()) return json;
            std::string sFirst = subVec.front();
            IKCJson& jsnFirst = GetSonItem(json, sFirst, caseSensitive);
            subVec.pop_front();
            return subVec.empty() ? jsnFirst : GetSubItem(jsnFirst, subVec, caseSensitive);
        };
        // 按分隔符，循环获取子项
        static IKCJson& GetSubItem(IKCJson& json, std::string sName, std::string sNameSplit = "\\.", bool caseSensitive = false)
        {
            std::deque<std::string> subVec;
            boost::algorithm::split_regex(subVec, sName, boost::regex(sNameSplit));
            return GetSubItem(json, subVec, caseSensitive);
        };
        static std::tuple<IKCJson*, std::string> GetSubItemPath(IKCJson& json, std::string sName, std::string sNameSplit = "\\.", bool caseSensitive = false)
        {
            std::deque<std::string> subVec;
            boost::algorithm::split_regex(subVec, sName, boost::regex(sNameSplit));
            std::string sLeaf;
            if (!subVec.empty())
            {
                sLeaf = subVec.back();
                subVec.pop_back();
            }
            return std::make_tuple(&GetSubItem(json, subVec, caseSensitive), sLeaf);
        };

        // 获取默认密钥
        static std::string GetDefaultAesKey(IKCContext& cntx)
        {
            const char keyArr[33] = { 0 };
            memcpy((void*)keyArr, c_strDefaultAesKey, std::min(static_cast<size_t>(32), sizeof(c_strDefaultAesKey)));
            std::string sCfgKey = CUtilFunc::PCharSafeToStr(cntx.GetCfgInfo("Config.Parameters.sys_flag", "customer"));
            if (!sCfgKey.empty())
                memcpy((void*)keyArr, sCfgKey.data(), std::min(static_cast<size_t>(30), sCfgKey.size()));
            return std::string(keyArr, 32);
        }
        // 默认密钥的加解密（用于加解密数据库连接串）
        static std::string EncryptConnStrDB(IKCContext& cntx, std::string str)
        {
            // return CUtilFuncEx::BaseXXEncodeByStr(str);
            return cntx.Encrypted(str.c_str(), str.size(), GetDefaultAesKey(cntx).c_str());
        }
        static std::string DecryptConnStrDB(IKCContext& cntx, std::string str)
        {
            // return CUtilFuncEx::BaseXXDecodeToStr(str);
            unsigned long sz = str.size();
            return cntx.Decrypted(str.c_str(), sz, GetDefaultAesKey(cntx).c_str());
        }

        // 移除注释
        static std::string RemoveComment(std::string sSQL)
        {
            try
            {
                // --
                sSQL = std::regex_replace(sSQL, std::regex(R"(--[^\n]*\n)"), "\n");
                // /* */
                std::string sCommFmt = R"(/\*((?!\*/)[\s\S])*\*/)";
                //sSQL = boost::regex_replace(sSQL, boost::regex(sCommFmt), " ");
                boost::regex pattern(sCommFmt);
                std::vector<std::string> vecComm;
                for (boost::sregex_iterator it(sSQL.cbegin(), sSQL.cend(), pattern), end_it; end_it != it; ++it)
                    vecComm.push_back(it->str());
                for (std::string sComm : vecComm)
                {
                    std::string sLine = boost::regex_replace(sComm, boost::regex(R"([^\r\n]*)"), "");
                    boost::algorithm::replace_all(sSQL, sComm, sLine);
                }
            }
            catch (std::exception &ex)
            {
                std::string sErr = (boost::format("RemoveComment Error: %s\n%s") % ex.what() % sSQL).str();
                std::cout << sErr << std::endl;
            }
            catch (...)
            {
                std::string sErr = (boost::format("RemoveComment Error.\n%s") % sSQL).str();
                std::cout << sErr << std::endl;
            }
            return sSQL;
        }
        static std::string RemoveCommentAndStr(std::string sSQL)
        {
            try
            {
                // 排除字符串
                sSQL = std::regex_replace(sSQL, std::regex(R"('[^']*')"), " ");
                // 排除注释
                sSQL = CCtrlCommon::RemoveComment(sSQL);
            }
            catch (std::exception &ex)
            {
                std::string sErr = (boost::format("RemoveComment Error: %s\n%s") % ex.what() % sSQL).str();
                std::cout << sErr << std::endl;
            }
            catch (...)
            {
                std::string sErr = (boost::format("RemoveComment Error.\n%s") % sSQL).str();
                std::cout << sErr << std::endl;
            }
            return sSQL;
        }

        // 移除多冒号
        static std::string RemoveMultiColon(std::string sSQL)
        {
            try
            {
                sSQL = std::regex_replace(sSQL, std::regex(R"(:{2,})"), " ");
            }
            catch (std::exception &ex)
            {
                std::string sErr = (boost::format("RemoveComment Error: %s\n%s") % ex.what() % sSQL).str();
                std::cout << sErr << std::endl;
            }
            catch (...)
            {
                std::string sErr = (boost::format("RemoveComment Error.\n%s") % sSQL).str();
                std::cout << sErr << std::endl;
            }
            return sSQL;
        }

        // 解析SQL语句是否为复合语句
        static bool IsSQLProcedures(std::string sSQL)
        {
            sSQL = boost::algorithm::to_lower_copy(RemoveCommentAndStr(sSQL));
            // 查找过程语句的“begin”
            std::smatch match;
            std::regex patternBegin("\\bbegin\\b\\s+\\w", std::regex::icase);
            if (!std::regex_search(sSQL, match, patternBegin)) return false;
            // 查找过程语句的“end”
            std::regex patternEnd("\\bend\\b\\s*;", std::regex::icase);
            auto pBegin = sSQL.begin() + match.position() + match.length() - 1, pEnd = sSQL.end();
            return std::regex_search(pBegin, pEnd, patternEnd);
        }

    };
}
