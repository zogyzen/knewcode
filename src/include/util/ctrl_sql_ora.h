#pragma once

#include "util/ctrl_sql.h"

namespace KC
{
    // 兼容oracle控制器公共函数
    struct CSqlFuncOra : CSqlFunc
    {
        // 页面活动对象（用于保存数据库连接，在整个页面执行期间有效）
        struct TOraConnObjBase : public IActionData::TActObj
        {
            // 连接编号
            const unsigned m_connid = 0;

            // 临时对象（临时表、临时视图、临时函数，等等）
            map<string, string> mapTmpObjs;
            // 临时对象计数
            std::atomic_uint m_tmpID;

            TOraConnObjBase(unsigned connid) : m_connid(connid), m_tmpID(0) {}

            virtual IKCSql& Srv(void) = 0;

            // 单独执行sql语句
            virtual void ExecSQL(const char*) = 0;

            // 删除临时对象
            void DropTmpObj(void)
            {
                std::string sCurrDrop;
                auto fExceptInfo = [&](string sMsg)
                {
                    string sInfo = sMsg + "\n";
                    for (auto &tmpObj : mapTmpObjs)
                        sInfo += tmpObj.first + "<=>" + tmpObj.second + "\n";
                    return sInfo + "=>" + sCurrDrop;
                };
                // 循环删除临时对象
                for (auto &tmpObj : mapTmpObjs)
                    try
                    {
                        sCurrDrop = tmpObj.first;
                        std::string sDropSQL = (boost::format(R"(
                            begin
                                  for x in (select object_name, object_type from user_objects where upper(object_name) = upper('%1%')) loop
                                      execute immediate 'drop ' || x.object_type || ' ' || x.object_name;
                                  end loop;
                            end;
                        )") % tmpObj.second).str();
                        ExecSQL(sDropSQL.c_str());
                        Srv().WriteLogDebug("Del Temp Object", __CURR_CODE_PLACE_C__, (sCurrDrop + "<=>" + tmpObj.second).c_str());
                    }
                    catch (std::exception& ex)
                    {
                        Srv().WriteLogFatal(fExceptInfo(ex.what()).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                    }
                    catch (...)
                    {
                        Srv().WriteLogFatal(fExceptInfo(Srv().getHint("Unknown_exception")).c_str(), __CURR_CODE_PLACE_C__);
                    }
                mapTmpObjs.clear();
            }
        };

        // 参数组
        template<typename TExcept>
        struct TParmGrpOraBase : CSqlFunc::TParmGrp<TExcept>
        {
            using CSqlFunc::TParmGrp<TExcept>::TParmGrp;

            // 是否需要绑定参数
            bool IsNeedParmBindInVal(void) const override
            {
                return c_RESTful_sql_DQL == this->sSQLWorkType || c_RESTful_sql_DML == this->sSQLWorkType || c_RESTful_sql_DDL == this->sSQLWorkType;
            }
        };

        // 数据库执行命令
        template<typename TExcept>
        struct TDBCommandOraBase : CSqlFunc::TDBCommand<TExcept>
        {
            // 页面活动对象
            virtual TOraConnObjBase& ConnObj(void) = 0;

            //  控制器 信息
            virtual std::string ActName(void) = 0;

            // 临时对象的随机名称
            virtual std::string GetTmpObjRandName(std::string sName)
            {
                // 日期
                boost::gregorian::date dtToday = boost::gregorian::day_clock::local_day();
                // 根据数据库版本，设置标识符最大长度
                unsigned sIdnLen = 128;
                // 临时包名称
                return (format("Z$TEMP%02d%02d%02d_%x_%X_%X_%s") % (dtToday.year() % 100) % (int)dtToday.month() % dtToday.day() % boost::this_process::get_id() % ConnObj().m_connid % (++ConnObj().m_tmpID) % sName).str().substr(0, sIdnLen);
            }
            // 替换临时对象名
            std::string ReplaceTmpObjName(std::string sSQL)
            {
                auto &parms = this->GetParms();
                // 解析临时名字
                parms.ParseSqlTmpTab(sSQL, std::string("##") + c_RESTful_KCTmpObj + "\\w+");
                // 循环每个临时对象
                for (auto &tmpTab : parms.mapTmpTabs)
                {
                    // 原临时对象名
                    std::string sName = tmpTab.first.substr(2, tmpTab.first.size() - 2);
                    // 在本次链接里是否存在
                    auto it = ConnObj().mapTmpObjs.find(sName);
                    // 不存在
                    if (ConnObj().mapTmpObjs.end() == it)
                    {
                        // 生成临时对象名
                        tmpTab.second = this->GetTmpObjRandName(sName);
                        // 插入本次链接里
                        ConnObj().mapTmpObjs.insert(make_pair(sName, tmpTab.second));
                        Srv().WriteLogDebug("Add Temp Object", __CURR_CODE_PLACE_C__, (sName + "<=>" + tmpTab.second).c_str());
                    }
                    // 存在
                    else
                    {
                        // 取出本次链接里的临时对象名
                        tmpTab.second = it->second;
                    }
                    // 替换临时对象名
                    boost::algorithm::replace_all(sSQL, tmpTab.first, tmpTab.second);
                }
                return sSQL;
            }

            // 创建临时包
            void CreateTmpPackage(ICtrlApiData& objCtrlD, std::string sSQL)
            {
                string sSQLPack, sSQLBody;
                auto fExceptInfo = [&](void)
                {
                    return "\n" + sSQLPack + "\n\n/\n\n" + sSQLBody + "\n";
                };
                try
                {
                    std::string sSQLPack = objCtrlD.GetSubInfo("declare");
                    boost::algorithm::trim(sSQLPack);
                    if (!sSQLPack.empty())
                    {
                        std::string sSQLBody = sSQL;
                        // 临时包名称
                        std::string sTmpName = c_RESTful_KCTmpObj + ActName(), sTmpPack;
                        auto itExists = ConnObj().mapTmpObjs.find(sTmpName);
                        if (ConnObj().mapTmpObjs.end() != itExists)
                            // throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 4, __CURR_CODE_PLACE_C__, Srv().getHint("Repeating_Define_") + sTmpName, Srv());
                            sTmpPack = itExists->second;
                        else
                        {
                            sTmpPack = this->GetTmpObjRandName(sTmpName);
                            ConnObj().mapTmpObjs.insert(make_pair(sTmpName, sTmpPack));
                        }
                        // 生成SQL语句
                        sSQLPack = "CREATE OR REPLACE PACKAGE " + sTmpPack + " IS " + sSQLPack + " \nEND;";
                        sSQLBody = "CREATE OR REPLACE PACKAGE BODY " + sTmpPack + " AS " + sSQLBody + " \nEND;";
                        // 执行包定义
                        ExecSQL(sSQLPack.c_str());
                        ExecSQL(sSQLBody.c_str());
                        // Srv().WriteLogTrace(sSQLBody.c_str(), __CURR_CODE_PLACE_C__, sSQLPack.c_str());
                    }
                }
                catch (std::exception& ex)
                {
                    std::string sErr = (boost::format("<%s> %s") % typeid(ex).name() % ex.what()).str();
                    Srv().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, fExceptInfo().c_str());
                    throw;
                }
            }

            // 针对DDL语句，创建临时对象和临时参数包
            std::string CreateTmpObjAndParmPack(std::string sSQL)
            {
                std::string sSQLPack, sSQLBody, sSQLCreate, sSQLAssignVal;
                auto fExceptInfo = [&](void)
                {
                    return "\n" + sSQLPack + "\n\n/\n\n" + sSQLBody + "\n\n/\n\n" + sSQLCreate + "\n\n/\n\n" + sSQLAssignVal + "\n";
                };
                try
                {
                    sSQLCreate = sSQL;
                    // 临时包名称
                    std::string sTmpName = c_RESTful_KCTmpObj + ActName(), sTmpPack;
                    auto itExists = ConnObj().mapTmpObjs.find(sTmpName);
                    if (ConnObj().mapTmpObjs.end() != itExists)
                        // throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 3, __CURR_CODE_PLACE_C__, m_own.getHint("Repeating_Define_") + sTmpName, m_own);
                        sTmpPack = itExists->second;
                    else
                    {
                        sTmpPack = this->GetTmpObjRandName(sTmpName);
                        ConnObj().mapTmpObjs.insert(make_pair(sTmpName, sTmpPack));
                    }
                    // 生成SQL语句
                    sSQLPack = "CREATE OR REPLACE PACKAGE " + sTmpPack + " IS\n";
                    sSQLBody = "CREATE OR REPLACE PACKAGE BODY " + sTmpPack + " AS\n";
                    sSQLAssignVal = "BEGIN\n";
                    auto &parms = this->GetParms();
                    for (auto &parm : parms.mapParms)
                    {
                        // 参数类型
                        std::string sType = "varchar2";
                        if (parm.second->eDTp == CSqlFunc::EDataType::edtInt) sType = "int";
                        else if (parm.second->eDTp == CSqlFunc::EDataType::edtNumber) sType = "number";
                        else if (parm.second->eDTp == CSqlFunc::EDataType::edtClob) sType = "clob";
                        // 字符串宽度
                        std::string sLen = "";
                        if ("varchar2" == sType)
                        {
                            std::string strVal = BoostAnyToStdString(parm.second->anyVal);
                            // string strVal = parm.second->sVal;
                            unsigned iLen = static_cast<unsigned>(strVal.size());
                            if (iLen < 100) sLen = "(100)";
                            else if (iLen < 500) sLen = "(500)";
                            else if (iLen < 4000) sLen = "(4000)";
                            else sLen = "(32767)";
                        }
                        // 包函数定义
                        std::string sFunc = "function f$" + parm.first + " return " + sType;
                        // 拼SQL语句
                        sSQLPack += parm.first + " " + sType + sLen + "; " + sFunc + ";\n";
                        sSQLBody += sFunc + " is begin return " + parm.first + "; end;\n";
                        sSQLAssignVal += sTmpPack + "." + parm.first + " := :" + parm.first + ";\n";
                        // 用包函数替换原SQL里的参数
                        boost::algorithm::replace_all(sSQLCreate, ":" + parm.first, sTmpPack + ".f$" + parm.first);
                    }
                    sSQLPack += "END;";
                    sSQLBody += "END;";
                    sSQLAssignVal += std::string(parms.mapParms.empty() ? "NULL;\n" : "") + "END;";
                    // 执行包定义
                    if (!parms.mapParms.empty())
                    {
                        ExecSQL(sSQLPack.c_str());
                        ExecSQL(sSQLBody.c_str());
                    }
                    // 替换临时对象名
                    sSQLCreate = ReplaceTmpObjName(sSQLCreate);
                    // 创建临时对象
                    ExecSQL(sSQLCreate.c_str());
                }
                catch (std::exception& ex)
                {
                    std::string sErr = (boost::format("<%s> %s") % typeid(ex).name() % ex.what()).str();
                    Srv().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, fExceptInfo().c_str());
                    throw;
                }
                return sSQLAssignVal;
            }

        };
    };
}
