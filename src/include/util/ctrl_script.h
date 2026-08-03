#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <map>
#include <set>
#include <memory>
#include <regex>
#include <functional>

#include <boost/any.hpp>
#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string_regex.hpp>

#include "util/request_respond_tmp.h"
#include "framework_ex/service_reference_ex_i.h"
#include "kc_web/kc_main_work_i.h"
#include "kc_web/kc_web_work_i.h"
#include "kc_controller/kc_sql_i.h"
#include "util/ctrl_common.h"

namespace KC
{
    // 脚本公共函数
    struct CScriptFunc
    {
        // 替换参数（sql语句里）
        static void ReplaceParm(ICtrlApiData& objCtrlD, std::string &cont)
        {
            // 解析要替换的参数
            std::string sTmpCont = CCtrlCommon::RemoveCommentAndStr(cont);;
            std::set<std::string> stReParms;
            std::regex patternReParms("#@\\w+#", std::regex::icase);
            for (std::sregex_iterator it(sTmpCont.cbegin(), sTmpCont.cend(), patternReParms), end_it; end_it != it; ++it)
                stReParms.insert(it->str());
            // 替换参数
            for (auto sRp : stReParms)
            {
                // 替换参数的名称和值
                std::string sName = sRp.substr(2, sRp.size() - 3);
                std::string sVal = "";
                // 全局参数
                if (objCtrlD.ActionData().IsGlobalVal(sName.c_str()))
                {
                    const char* pGlobal = objCtrlD.ActionData().GetGlobalVal(sName.c_str(), nullptr);
                    if (nullptr == pGlobal)
                        throw TException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, objCtrlD.ActionData().GetHint("Parm_Error_") + sName, objCtrlD.SignName());
                    sVal = pGlobal;
                }
                // 用户参数
                else
                {
                    // 得到参数值
                    auto fGetParmVal = [&](void) -> std::string
                    {
                        const IKCJson& jp = objCtrlD.GetParmJson(sName.c_str());
                        if (!jp.IsValid()) throw TException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, objCtrlD.ActionData().GetHint("Parm_Error_") + sName, objCtrlD.SignName());
                        return jp.GetStr();
                    };
                    // 得到替换值
                    auto fGetReplaceVal = [&](void) -> std::string
                    {
                        const IKCJson& jType = objCtrlD.JsonAttach().GetItem(c_RESTful_replace).GetItem(sName.c_str());
                        std::string sType = c_RESTful_SQLInjection;
                        // if (!jType.IsValid()) throw TException(c_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, "[Attach] No Set Replace - " + sRp);
                        if (jType.IsValid()) sType = jType.GetStr();
                        // 不检查，直接替换
                        if (c_RESTful_NoCheck == sType)
                            return fGetParmVal();
                        // sql注入检查
                        else if (c_RESTful_SQLInjection == sType)
                        {
                            std::string sResult = fGetParmVal();
                            std::string sCheck = boost::algorithm::to_lower_copy(sResult);
                            string keys[] = { "/", "union", "drop", "delete", "update", "insert", "|", "&", "^" , "#", "/*", "*/", "--", ";", "%0a"};
                            for (auto key : keys)
                                if (sCheck.find(key) != string::npos)
                                    throw TKCSqlException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, "SQL Injection - " + sName, sResult);
                            return sResult;
                        }
                        // 替换名称
                        else if (c_RESTful_OnlyName == sType)
                        {
                            std::string sResult = fGetParmVal();
                            std::regex reg(R"(\w+)");
                            if(!std::regex_match(sResult, reg))
                                throw TException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, objCtrlD.ActionData().GetHint("Parm_Error_") + std::string("Name Error.  ") + sName + ": '" + sResult + "'  ", objCtrlD.SignName());
                            return sResult;
                        }
                        // 其他，则用正则表达式匹配
                        else
                        {
                            std::string sResult = fGetParmVal();
                            std::regex reg(sType);
                            if(!std::regex_match(sResult, reg))
                                throw TException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, objCtrlD.ActionData().GetHint("Parm_Error_") + std::string("Mismatching.  ") + sName + ": '" + sResult + "'  ", objCtrlD.SignName(), "Regex: " + sType);
                            return sResult;
                        }
                        // 类型错误
                        // else throw TException(ecd_ErrCode_KCWebApiWork + 50, __CURR_CODE_PLACE_C__, "[Attach] Replace Type error - " + sName + " => " + sType);
                    };
                    sVal = fGetReplaceVal();
                }
                // 替换
                boost::algorithm::replace_all(cont, sRp, sVal);
            }
        }

        // 获取待替换的正文
        static std::string GetReplaceContent(ICtrlApiData& objCtrlD, std::string sUrlSub, std::string sFlagB, std::string sFlagE)
        {
            // 拆分文件和控制器
            std::vector<std::string> strVec;
            boost::algorithm::split(strVec, sUrlSub, boost::is_any_of("?"));
            // 替换
            std::string sReplace = "";
            // 文件绝对路径
            sUrlSub = boost::algorithm::trim_copy(strVec[0]);
            std::string sSelfLocalFile = CUtilFunc::PCharSafeToStr(objCtrlD.LocalFile());
            std::string sLocalFile = sUrlSub.empty() ? sSelfLocalFile : CUtilFunc::PCharSafeToStr(objCtrlD.transItemUrlToFullPath(sUrlSub.c_str()));
            if (!boost::filesystem::exists(sLocalFile))
                throw std::runtime_error("Include Error. Not Exists - " + sUrlSub);
            // 用文件部分内容替换
            if (strVec.size() > 1)
            {
                std::string sCtrl = boost::trim_copy(strVec[1]);
                boost::algorithm::split(strVec, sCtrl, boost::is_any_of("="));
                if (strVec.size() < 2) throw std::runtime_error("Include Error. Controller - " + sCtrl);
                std::string sTp = boost::trim_copy(boost::algorithm::to_lower_copy(strVec[0]));
                if (sTp.empty()) throw std::runtime_error("Include Error. Controller - " + sCtrl);
                std::string sName = boost::trim_copy(strVec[1]);
                if (sName.empty()) throw std::runtime_error("Include Error. Controller - " + sCtrl);
                // 用控制器的内容替换
                if (c_RESTful_act == sTp)
                {
                    if (sSelfLocalFile == sLocalFile && objCtrlD.ActName() == sName)
                        throw std::runtime_error("Can't Include Self - " + CUtilFunc::PCharSafeToStr(objCtrlD.SignName()));
                    boost::property_tree::ptree pt;
                    boost::property_tree::read_xml(sLocalFile, pt);
                    // 读控制器内容
                    sCtrl = c_RESTful_Controllers + std::string(".") + sName;
                    if (!pt.get_child_optional(sCtrl))
                        throw std::runtime_error("Not Exists, " + sUrlSub + " - " + sCtrl);
                    sReplace = pt.get<std::string>(sCtrl);
                }
                // 用文件的部分内容替换
                else if (c_RESTful_part == sTp)
                {
                    std::string sFlTxt = CUtilFunc::ReadTxtFile(sLocalFile);
                    // 按起止标记，取文件部分内容
                    std::string strFlagB = (boost::format(sFlagB) % sName).str();
                    std::string strFlagE = (boost::format(sFlagE) % sName).str();
                    std::string sPartFmt = (boost::format(R"(%1%\s*\n((?!%2%)[\s\S])*\n\s*%2%)") % strFlagB % strFlagE).str();
                    std::regex pattern(sPartFmt);
                    std::smatch what;
                    if(!std::regex_search(sFlTxt, what, pattern))
                        throw std::runtime_error("Not Exists, " + sUrlSub + " - " + sCtrl);
                    sReplace = what[0];
                }
                // 类型错误
                else throw std::runtime_error("Include Error.. Controller - " + sTp);
            }
            else if (sSelfLocalFile == sLocalFile)
                throw std::runtime_error("Can't Include Self - " + CUtilFunc::PCharSafeToStr(objCtrlD.SignName()));
            // 用整个文件内容替换
            else sReplace = CUtilFunc::ReadTxtFile(sLocalFile);
            return sReplace;
        }
        // 替换文件
        static unsigned ReplaceInclude(ICtrlApiData& objCtrlD, std::string &sScript, std::string sFlagB, std::string sFlagE, unsigned iMaxLayer = 128, int iLayer = 0)
        {
            // 分析
            std::vector<std::string> vecInclude;
            string sPattern = (boost::format("#%s\\s*\"[^\"]+\"#") % c_RESTful_include).str();
            std::regex pattern(sPattern, std::regex::icase);
            for (std::sregex_iterator it(sScript.cbegin(), sScript.cend(), pattern), end_it; end_it != it; ++it)
                vecInclude.push_back(it->str());
            // 循环替换
            for (auto sInclude : vecInclude)
            {
                // 得到文件
                std::string sFile = sInclude.substr(sInclude.find("\""));
                if (sFile.size() <= 3) throw std::runtime_error("Include Error - " + sInclude);
                sFile = boost::trim_copy(sFile.substr(1, sFile.size() - 3));
                if (sFile.empty() || "?" == sFile)
                    throw std::runtime_error("Include Error - " + sInclude);
                // 得到待替换的内容
                std::string sReplace = GetReplaceContent(objCtrlD, sFile, sFlagB, sFlagE);
                // 递归
                if (iLayer > static_cast<int>(iMaxLayer)) throw std::runtime_error("infinite loop, " + sFile);
                CScriptFunc::ReplaceInclude(objCtrlD, sReplace, sFlagB, sFlagE, iMaxLayer, iLayer + 1);
                // 替换
                boost::algorithm::replace_all(sScript, sInclude, sReplace);
            }
            boost::algorithm::trim(sScript);
            return static_cast<unsigned>(vecInclude.size());
        }
    };

    // 非SQL控制器公共函数
    struct CCltrFunc
    {
        // 解析控制器参数
        static void ParseCtrl(std::map<std::string, std::string>& mpCfgs, std::string sContext)
        {
            std::vector<std::string> lineVec;
            // boost::algorithm::split(lineVec, sContext, boost::is_any_of(";\n"));
            boost::algorithm::split_regex(lineVec, sContext, boost::regex(";[\r\n]+"));
            for (auto sLine : lineVec)
            {
                sLine = boost::trim_copy_if(sLine, boost::is_any_of(";\n\r"));
                /*
                vector<std::string> cfgVec;
                boost::algorithm::split(cfgVec, boost::trim_copy(sLine), boost::is_any_of(":"));
                if (cfgVec.size() > 1)
                {
                    auto it = mpCfgs.find(cfgVec[0]);
                    if (mpCfgs.end() != it) mpCfgs.erase(it);
                    mpCfgs.insert(make_pair(boost::trim_copy(cfgVec[0]), boost::trim_copy(cfgVec[1])));
                }
                */
                auto iPos = sLine.find(":");
                if (std::string::npos != iPos && iPos > 0)
                {
                    std::string sName = boost::algorithm::to_lower_copy(boost::trim_copy(sLine.substr(0, iPos)));
                    std::string sVal = boost::trim_copy(sLine.substr(iPos + 1, sLine.size() - iPos - 1));
                    auto it = mpCfgs.find(sName);
                    if (mpCfgs.end() != it) mpCfgs.erase(it);
                    if (!sName.empty()) mpCfgs.insert(make_pair(sName, sVal));
                }
            }
        }

        // 调用控制器（同一个控制器内的子调用）
        static int CallCtrl(ICtrlApiData& ctrlD, std::map<std::string, std::string>& mpCfgs, std::string sCfgName, bool ignore = false)
        {
            auto it = mpCfgs.find(sCfgName);
            if (mpCfgs.end() != it)
                return ctrlD.WorkAct(boost::trim_copy(boost::trim_copy_if(it->second, boost::is_any_of("\""))).c_str(), sCfgName.c_str());
            return ignore ? 0 : -55555;
        }
        // 调用控制器（启动一个新的活动连接，全新调用）
        template<typename TRe>
        static int CallCtrl(IBundleContext &cont, TRe& re)
        {
            IServiceReference* pRef = cont.takeServiceReference(c_KCWebWorkSrvGUID);
            CAutoRelease _auto([&](){ cont.freeServiceReference(pRef); });
            IServiceReferenceEx *WebMainRef = dynamic_cast<IServiceReferenceEx*>(pRef);
            if (nullptr != WebMainRef)
            {
                IKCWebWork& wbwk = WebMainRef->getServiceSafe<IKCWebWork>();
                // 调用控制器
                return wbwk.RenderKC(re);
            }
            return -55556;
        }
        template<typename TWork>
        static std::string CallCtrl(TWork& wrk, IServiceEx &srv, std::string uri, std::string sCookie, std::string sParm)
        {
            std::string sResult;
            try
            {
                if (uri.empty()) throw TException(999899, __CURR_CODE_PLACE_C__, "Empty URL");
                // 请求响应上下文
                CRequestRespondTmp<TWork> reTmp(wrk);
                // 请求参数
                reTmp.AddPostBody(sParm);
                // cookie
                reTmp.AddCookie(sCookie);
                // 链接
                reTmp.AddTheRequest(uri);
                std::vector<std::string> uriVec;
                boost::algorithm::split(uriVec, uri, boost::is_any_of("?"));
                reTmp.AddUri(uriVec[0]);
                if (uriVec.size() > 1) reTmp.AddArgs(uriVec[1]);
                // 附加控制器信息
                //reTmp.AddExSingleInfo("$__KCCtrl_Main_Header__", sName);
                // 调用控制器
                CCltrFunc::CallCtrl(srv.getContext(), reTmp);
                sResult = reTmp.GetRespondBody();
            }
            catch(TException& ex)
            {
                ex.OtherInfo() = "cookie: " + sCookie + "\njson: " + sParm;
                ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
                srv.WriteLog(ex);
                sResult = (boost::format(R"({%s:%d,%s:"%s"})") % c_RESTful_errCode % ex.error_id() % c_RESTful_errMsg % ex.error_info()).str();
            }
            catch (std::exception& ex)
            {
                string sErr = "cookie: " + sCookie + "\njson: " + sParm + "\n" + ex.what();
                srv.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                sResult = (boost::format(R"({%s:%d,%s:"%s"})") % c_RESTful_errCode % 999899 % c_RESTful_errMsg % ex.what()).str();
            }
            catch (...)
            {
                string sErr = srv.getContext().getHint("Unknown_exception");
                srv.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, ("cookie: " + sCookie + "\njson: " + sParm).c_str());
                sResult = (boost::format(R"({%s:%d,%s:"%s"})") % c_RESTful_errCode % 999899 % c_RESTful_errMsg % sErr).str();
            }
            return sResult;
        }
        static std::string CallCtrl(IServiceEx &srv, std::string uri, std::string sCookie, std::string sParm)
        {
            return CallCtrl(srv.getContext().GetMain(), srv, uri, sCookie, sParm);
        }

        // 得到参数
        static std::string GetParmName(std::map<std::string, std::string>& mpCfgs, std::string sCfgName)
        {
            std::string sName = "";
            auto it = mpCfgs.find(boost::algorithm::to_lower_copy(sCfgName));
            if (mpCfgs.end() != it)
                sName = boost::trim_copy(it->second);
            return sName;
        }
        static std::string GetParmVal(ICtrlApiData& objCtrlD, std::string sName, std::string sCfgName)
        {
            // 参数
            if (sName.size() > 1 && '@' == sName[0])
            {
                sName = boost::trim_copy(boost::trim_left_copy_if(sName, boost::is_any_of("@")));
                if (sName.empty())
                    throw TException(ecd_ErrCode_KCWebApiWork + 51, __CURR_CODE_PLACE_C__, objCtrlD.ActionData().GetHint("Parm_Error_") + std::string("[") + sCfgName + "]");
                return boost::trim_copy(CUtilFunc::PCharSafeToStr(objCtrlD.GetParmVal(sName.c_str())));
            }
            // 非参数，直接返回（去掉前后的引号和空格）
            else return boost::trim_copy_if(sName, boost::is_any_of("\"' "));
        }
        static std::string GetParm(ICtrlApiData& objCtrlD, std::map<std::string, std::string>& mpCfgs, std::string sCfgName)
        {
            std::string sName = GetParmName(mpCfgs, sCfgName);
            return GetParmVal(objCtrlD, sName, sCfgName);
        }
        // 设置参数
        static bool SetParm(ICtrlApiData& objCtrlD, std::map<std::string, std::string>& mpCfgs, std::string sCfgName, std::string sVal)
        {
            std::string sName = GetParmName(mpCfgs, sCfgName);
            // 参数
            if (!sName.empty() && '@' == sName[0])
            {
                sName = boost::trim_copy(boost::trim_left_copy_if(sName, boost::is_any_of("@")));
                if (sName.empty()) return false;
                CCtrlCommon::SetParmVal(objCtrlD, sName, sVal);
                return true;
            }
            else return false;
        }
        // 替换参数（非sql语句里）
        static std::string ReplaceParm(ICtrlApiData& objCtrlD, map<std::string, std::string> &mpCfgs, std::string sCfgName)
        {
            // 参数
            std::string sPrms;
            std::string sPrmSrc = CCltrFunc::GetParm(objCtrlD, mpCfgs, sCfgName);
            if (!sPrmSrc.empty())
            {
                // 分解参数
                std::vector<std::string> parmsVec;
                boost::algorithm::split(parmsVec, sPrmSrc, boost::is_any_of("&"));
                for (auto prm : parmsVec)
                {
                    auto iPos = prm.find("=");
                    if (std::string::npos != iPos)
                    {
                        // 参数名、值
                        std::string sName = prm.substr(0, iPos);
                        std::string sVal = prm.substr(iPos + 1);
                        // 替换参数值
                        IActionData& act = objCtrlD.ActionData();
                        std::string sPrm = boost::trim_left_copy_if(sVal, boost::is_any_of("@"));
                        // 全局参数
                        const char* pGlobal = nullptr;
                        if ((pGlobal = act.GetGlobalVal(sPrm.c_str())) != nullptr && strlen(pGlobal) > 0)
                            sVal = pGlobal;
                        // 用户参数
                        else
                        {
                            const IKCJson& jsonParms = objCtrlD.JsonRequest().GetItem(c_RESTful_inParm);
                            if (jsonParms.IsValid())
                            {
                                const IKCJson& jsonParm = jsonParms.GetItem(sPrm.c_str());
                                if (jsonParm.IsValid()) sVal = jsonParm.GetStr("");
                            }
                            else
                            {
                                const IKCJson& jsonParm = objCtrlD.JsonRequest().GetItem(sPrm.c_str());
                                if (jsonParm.IsValid()) sVal = jsonParm.GetStr("");
                            }
                        }
                        // 拼接
                        sPrms += sName + "=" + sVal + "&";
                    }
                    else sPrms += prm + "&";
                }
                boost::trim_right_if(sPrms, boost::is_any_of("&"));
            }
            return sPrms;
        }

        // 字符集转换（PageSet页面的编码，一般为utf8；OSSet当前操作系统编码，需手工指定，中文操作系统一般为GBK）
        static std::string TranCharset(std::string str, std::string PageSet, std::string OSSet)
        {
            if (PageSet == OSSet) return str;
            else if (c_RESTful_UTF8 == PageSet && c_RESTful_GBK == OSSet)
                return CUtilFunc::Utf8ToGbk(str);
            else if (c_RESTful_GBK == PageSet && c_RESTful_UTF8 == OSSet)
                return CUtilFunc::GbkToUtf8(str);
            else return str;
        }
        // bInOrOut输入或输出。true输入，false输出
        static std::string TranCharset(std::string str, ICtrlApiData& objCtrlD, bool bInOrOut = true)
        {
            string sCurrentSet = objCtrlD.GetCharset();
            string sAttachCharset = objCtrlD.JsonAttach().GetStr(c_RESTful_Charset, c_RESTful_UTF8);
            boost::algorithm::to_upper(sAttachCharset);
            return bInOrOut ? TranCharset(str, sCurrentSet, sAttachCharset) : TranCharset(str, sAttachCharset, sCurrentSet);
        }

    };
}
