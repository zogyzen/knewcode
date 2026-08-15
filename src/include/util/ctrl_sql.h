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
#include <boost/regex.hpp>
#include <boost/bind/bind.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/foreach.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "common/base_type.h"
#include "util/util_funcs.h"
#include "util/auto_release.h"
#include "util/backtrace_symbols.h"
#include "kc_web/kc_request_respond.h"
#include "kc_controller/kc_sql_i.h"
#include "util/ctrl_common.h"

namespace KC
{
    // SQL控制器公共函数
    struct CSqlFunc
    {
        // 参数类型
        enum class EParmType { eptUnknown = -1, eptInParm = 0, eptDefParm, eptOutParm, eptInOutParm, eptSessionParm };
        enum class EDataType { edtUnknown = -1, edtString = 0, edtInt, edtNumber, edtDatetime, edtClob, edtJson,
                               edtDbSet = 100, edtDbSetOne, edtDbSetArray, edtDbSetInner };

        // 多类型值类型
        // typedef std::variant<bool, char, short, int, long long, float, double, std::string> TKcValVariant;
        typedef boost::any TKcValVariant;
        template<typename TType>
        static TType* KCVariantAnyCastPtr(TKcValVariant& val)
        {
            // return std::get_if<TType>(&val);
            return boost::any_cast<TType>(&val);
        }

        // 字段
        struct TField
        {
            const std::string m_name = "field";         // 名称
            std::string m_nameShow = "field";           // 显示名称
            const int m_pos = -1;                       // 位置
            const int m_type = -1;                      // 类型
            const EDataType m_dataType;                 // 数据类型
            const unsigned m_size = 0;                  // 总字节数
            const unsigned m_decimals = 0;              // 小数位数
            const bool m_nullable = true;               // 是否为空

            TField(std::string n, int p = -1, int t = -1, EDataType dt = EDataType::edtString, unsigned sz = 0, unsigned dc = 0, bool nl = true)
                : m_name(n), m_nameShow(n), m_pos(p), m_type(t), m_dataType(dt), m_size(sz), m_decimals(dc), m_nullable(nl) {}
            virtual ~TField(void) = default;

            std::string GetTypeName(void)
            {
                switch (m_dataType)
                {
                case EDataType::edtNumber:
                    return "numeric";
                    break;
                case EDataType::edtInt:
                    return "int";
                    break;
                case EDataType::edtDatetime:
                    return "timestamp";
                    break;
                case EDataType::edtJson:
                    return "json";
                    break;
                case EDataType::edtDbSet:
                case EDataType::edtDbSetOne:
                case EDataType::edtDbSetArray:
                case EDataType::edtDbSetInner:
                    return "record";
                    break;
                case EDataType::edtString:
                default:
                    return "varchar";
                    break;
                }
            }
        };
        typedef std::shared_ptr<TField> TFieldPtr;
        typedef std::vector<TFieldPtr> TFields;

        // 字段值
        struct TFieldVal
        {
            TFieldPtr m_field;
            TKcValVariant m_val;

            TFieldVal(TFieldPtr f, TKcValVariant v) : m_field(f), m_val(v) {}
        };
        typedef std::shared_ptr<TFieldVal> TFieldValPtr;
        typedef std::map<std::string, TFieldValPtr> TFieldValMap;
        // 行数据
        struct TRowData
        {
            const int m_row = 0;
            TFieldValMap m_valMap;

            TRowData(int r) : m_row(r) {}
        };
        typedef std::shared_ptr<TRowData> TRowDataPtr;

        // 控制器添加内部数据集
        struct TRecordSet;
        typedef std::shared_ptr<TRecordSet> TRecordSetPtr;
        class ICtrlApiDataWithDbSet : public ICtrlApiDataX
        {
        public:
            // 保存内部数据集
            virtual void CALL_TYPE DataSetSaveToInner(TRecordSetPtr) = 0;
            // 获取上一个控制器的内部数据集
            virtual TRecordSetPtr CALL_TYPE DataSetInnerLast(std::string) = 0;
        };

        // 数据集
        struct TRecordSet : public std::enable_shared_from_this<TRecordSet>
        {
            std::string m_name;

            virtual ~TRecordSet(void) = default;

            // 控制器数据接口
            virtual ICtrlApiData& CtrlD(void) = 0;

            // 获取各字段基础信息（各数据库插件，在实现时，二选一）：
            // oracle数据库，在获取第1条记录前，可以获取到字段基础信息，并且可以修改字段缓冲区最大尺寸（获取第1条记录后，再修改字段缓冲区尺寸，没有作用）。选择实现GetColumnListMetaDataBeforeFetch。
            // postgresql数据库，在获取第1条记录前，不能获取到字段基础信息。选择实现GetColumnListMetaData。
            // 其他数据库待测试，目前选择实现GetColumnListMetaData。
            virtual void GetColumnListMetaData(TFields&) {}
            virtual bool GetColumnListMetaDataBeforeFetch (TFields&) { return false; }
            // 下一条
            virtual bool Next(void) = 0;
            // 添加值
            virtual void AddVal(IKCJson& jsonRow, TField&) = 0;
            // 得到值
            virtual TKcValVariant GetVal(TField&) const = 0;

            // 数据集输出
            int DataSetToOut(TFields& vecFields, std::function<void()> fInitFeilds, std::function<bool(int, int)> fAddRow)
            {
                // 提前获取各字段基础信息
                const bool bPreGetFields = this->GetColumnListMetaDataBeforeFetch(vecFields);
                if (bPreGetFields) fInitFeilds();
                // 跳转下一条记录
                auto fNextRec = [&](int &iUseTimeNext)
                {
                    // 用时统计
                    long long iNow = CUtilFunc::GetCurrentStampMS();
                    CAutoRelease _auto([&](){ iUseTimeNext = static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNow); });
                    return Next();
                };
                // 循环每条记录
                int iRecCount = 0;
                for (int iUseTimeNext = 0; fNextRec(iUseTimeNext); ++iRecCount)
                {
                    // 首条记录，初始化。获取各字段基础信息
                    if (0 == iRecCount && !bPreGetFields)
                    {
                        this->GetColumnListMetaData(vecFields);
                        fInitFeilds();
                    }
                    // 添加行记录
                    if (!fAddRow(iRecCount, iUseTimeNext)) break;
                }
                return iRecCount;
            }
            // 数据集转json
            int DataSetToJson(IKCJson& jset, bool atRoot = true)
            {
                const char* sValsName = CtrlD().ActionData().GetValsName();
                // 数据列表
                jset.DelItem(sValsName);
                IKCJson& jsonSet = jset.AddArray(sValsName);
                // 请求的字段
                TFields vecFields;
                if (atRoot) CSqlFunc::RequestFeilds(CtrlD().ActionData(), vecFields);
                return DataSetToOut(vecFields,
                    [&](){
                        // 重置字段
                        jset.DelItem(c_RESTful_feilds);
                        IKCJson& jsonFds = jset.AddItem(c_RESTful_feilds);
                        for (auto fd : vecFields) jsonFds.AddStr(fd->m_nameShow.c_str(), fd->GetTypeName().c_str());
                    }, [&](int, int){
                        // 添加行记录
                        IKCJson& jsonRow = jsonSet.AddItem("");
                        for (auto fFeild : vecFields) AddVal(jsonRow, *fFeild);
                        return true;
                    });
            }
            int DataSetToJsonOne(IKCJson& jset)
            {
                // 请求的字段
                TFields vecFields;
                // 提前获取各字段基础信息
                const bool bPreGetFields = this->GetColumnListMetaDataBeforeFetch(vecFields);
                // 循环插入记录
                int iResult = 0;
                if (Next())
                {
                    // 获取各字段类型
                    if (!bPreGetFields)
                        this->GetColumnListMetaData(vecFields);
                    // 添加记录
                    for (auto fFeild : vecFields) AddVal(jset, *fFeild);
                    iResult = 1;
                }
                return iResult;
            }
            int DataSetToJsonArray(IKCJson& jset)
            {
                // 请求的字段
                TFields vecFields;
                // 提前获取各字段基础信息
                const bool bPreGetFields = this->GetColumnListMetaDataBeforeFetch(vecFields);
                // 循环插入记录
                int iRecCount = 0;
                for (; Next(); ++iRecCount)
                {
                    // 初始化
                    if (0 == iRecCount && !bPreGetFields)
                    {
                        // 获取各字段类型
                        this->GetColumnListMetaData(vecFields);
                    }
                    // 添加行记录
                    IKCJson& jsonRow = jset.AddItem("");
                    for (auto fFeild : vecFields) AddVal(jsonRow, *fFeild);
                }
                return iRecCount;
            }
            // 数据集存入控制器，供下一个api使用
            void DataSetSaveToInner(void)
            {
                auto self(this->shared_from_this());
                ICtrlApiDataWithDbSet* pCtrl = dynamic_cast<ICtrlApiDataWithDbSet*>(&CtrlD());
                if (nullptr != pCtrl) pCtrl->DataSetSaveToInner(self);
            }
        };

        // 参数
        struct TParm
        {
            unsigned iPos = 0;  // 有效参数，从1开始
            std::string sName;
            EParmType ePTp = EParmType::eptUnknown;
            EDataType eDTp = EDataType::edtUnknown;
            bool bIsNull = true;
            // std::string sVal;
            boost::any anyVal;
            std::vector<std::string> m_path;
            bool m_isFirstDataSet = false;  // 是否首个数据集

            virtual ~TParm(void) = default;
            TParm(void) = default;
            TParm(const TParm&) = delete;
            TParm& operator=(const TParm&) = delete;

            // 设置参数值
            virtual void SetNull(void) = 0;
            virtual bool SetNumber(double) = 0;
            virtual void SetInt(int) = 0;
            virtual void SetString(std::string) = 0;
            virtual void SetClob(std::string) = 0;
            // 设置参数值的函数是否有效
            virtual bool IsValidSetXX(void) const { return true; }
            // 获取参数值
            virtual bool IsNull(void) { return bIsNull; };
            virtual double GetNumber(double = 0) = 0;
            virtual int GetInt(int = 0) = 0;
            virtual std::string GetString(std::string = "") = 0;
            virtual std::string GetDate(std::string = "") = 0;
            virtual std::string GetDateTime(std::string = "") = 0;
            virtual std::string GetClob(std::string = "") = 0;
            // 获取数据集（游标、临时表）
            virtual TRecordSetPtr GetDbSet(void) = 0;
            // 是否临时表
            virtual bool IsTempTable(void) const { return false; }

            // 初始化参数
            void Init(void)
            {
                ePTp = EParmType::eptUnknown;
                eDTp = EDataType::edtUnknown;
                // sVal.clear();
                anyVal.clear();
                bIsNull = true;
                m_isFirstDataSet = false;
            }

            // 设置参数类型
            void SetDbTp(std::string sType)
            {
                auto iPos = sType.find("(");
                if (iPos != std::string::npos) sType = sType.substr(0, iPos);
                if (sType == c_RESTful_int) SetDbTp(EDataType::edtInt);
                else if (sType == c_RESTful_number) SetDbTp(EDataType::edtNumber);
                else if (sType == c_RESTful_date) SetDbTp(EDataType::edtDatetime);
                else if (sType == c_RESTful_dbset || sType == c_RESTful_cursor || sType == c_RESTful_temp_table) SetDbTp(EDataType::edtDbSet);
                else if (sType == c_RESTful_dbset_one || sType == c_RESTful_cursor_one || sType == c_RESTful_temp_table_one) SetDbTp(EDataType::edtDbSetOne);
                else if (sType == c_RESTful_dbset_array || sType == c_RESTful_cursor_array || sType == c_RESTful_temp_table_array) SetDbTp(EDataType::edtDbSetArray);
                else if (sType == c_RESTful_dbset_inner) SetDbTp(EDataType::edtDbSetInner);
                else if (sType == c_RESTful_clob) SetDbTp(EDataType::edtClob);
                else if (sType == c_RESTful_json) SetDbTp(EDataType::edtJson);
                else SetDbTp(EDataType::edtString);
            }
            void SetDbTp(EDataType eType)
            {
                eDTp = eType;
                SetDbTp();
            }
            virtual void SetDbTp(void) = 0;

            virtual void RegOutParam(void) {};

            // 类型转换
            template<typename TPrm>
            TPrm& Get(void)
            {
                TPrm *pPrm = dynamic_cast<TPrm*>(this);
                if (nullptr == pPrm)
                    throw TKCSqlException(206, __CURR_CODE_PLACE_C__, std::string("Type Mismatching: ") + typeid(*this).name() + " => " + typeid(TPrm).name(), typeid(*this).name());
                return *pPrm;
            }
        };
        typedef std::shared_ptr<TParm> TParmPtr;
        template<typename TExcept>
        struct TParmGrp
        {
            // 参数
            std::multimap<std::string, TParmPtr, TLessStr> mapParms;
            // 全局参数
            std::map<std::string, TParmPtr, TLessStr> mapSessionParms;
            // 输出参数
            std::map<std::string, TParmPtr, TLessStr> mapOutParms;
            // 临时表（临时视图、临时函数，等等）
            std::map<std::string, std::string, TLessStr> mapTmpTabs;
            // 数据库操作类型（DML、DQL、DDL、DCL）
            std::string sSQLWorkType = c_RESTful_sql_DML;
            // 是否需要绑定参数
            bool IsNeedParmBindInVal(void) const { return c_RESTful_sql_DQL == sSQLWorkType || c_RESTful_sql_DML == sSQLWorkType; }
            // 是否批量操作
            const bool m_isBatchOp = false;
            // 参数名是否可重复多次
            const bool m_multiParm = false;
            // 预执行前绑定参数
            const bool m_isPrePrepareBind = true;

            TParmGrp(bool caseSensitive, bool bMultiParm = false, bool bIsPreBind = true)
                : mapParms(TLessStr(caseSensitive)), mapSessionParms(TLessStr(caseSensitive)), mapOutParms(TLessStr(caseSensitive)), mapTmpTabs(TLessStr(caseSensitive))
                , m_multiParm(bMultiParm), m_isPrePrepareBind(bIsPreBind)
            {
            }

            virtual ~TParmGrp(void)
            {
                mapSessionParms.clear();
                mapOutParms.clear();
                mapParms.clear();
                mapTmpTabs.clear();
            }

            // 所属的服务
            virtual IKCSql& Srv(void) = 0;

            // 控制器数据接口
            virtual ICtrlApiData& CtrlD(void) = 0;

            //  控制器 信息
            virtual std::string ActInfo(void) = 0;

            // 创建参数
            virtual TParmPtr MakePram(std::string = "", unsigned = 0) = 0;

            // 在数据库上解绑所有已绑定的参数
            virtual void UnbindParms(void) {}

            // 参数异常信息
            std::string ParmExceptInfo(TParm &parm, std::string sMsg)
            {
                // string strVal = parm.sVal;
                std::string strVal = BoostAnyToStdString(parm.anyVal);
                return (boost::format("Paramter Error: @%d\t%s='%s'\t(%d/%d)\n%s")
                        % parm.iPos % parm.sName % strVal % static_cast<int>(parm.ePTp) % static_cast<int>(parm.eDTp) % sMsg).str();
            };
            virtual std::string ParmExceptInfo(TExcept&) = 0;

            // 解析sql语句里的参数
            virtual void ParseSqlParms(std::string sSQL, std::string sFmt = R"(:\w+)" /*R"((?<!:):\w+)"*/)
            {
                try
                {
                    // 移除注释和字符串
                    sSQL = CCtrlCommon::RemoveCommentAndStr(sSQL);
                    // 移除多冒号。由于std::regex不支持“零宽度断言（负后顾断言）”或“反向否定预查”。所以需要先移除多冒号，再解析参数。
                    sSQL = CCtrlCommon::RemoveMultiColon(sSQL);
                    // 分析SQL语句中的参数
                    unsigned i = 1;
                    std::regex pattern(sFmt, std::regex::icase);
                    for (std::sregex_iterator it(sSQL.cbegin(), sSQL.cend(), pattern), end_it; end_it != it; ++it)
                    {
                        std::string sParmName = it->str().substr(1, it->str().size() - 1);
                        //boost::algorithm::to_lower(sParm);      // 参数名不区分大小写
                        auto itParm = mapParms.find(sParmName);
                        if (m_multiParm || mapParms.end() == itParm)
                        {
                            std::cout << "\t*[Knewcode] Param: " << sParmName << std::endl;
                            // Srv().WriteLogTrace(("Param: " + sParm).c_str(), __CURR_CODE_PLACE_C__);
                            TParmPtr parmPtr = MakePram(sParmName, i);
                            mapParms.insert(std::make_pair(sParmName, parmPtr));
                            ++i;
                        }
                    }
                    // 设置参数类型
                    SetParmsType();
                }
                catch (std::exception &ex)
                {
                    std::string sErr = (boost::format("ParseSqlParms Error: %s\n%s\n%s") % ex.what() % sFmt % sSQL).str();
                    Srv().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                    throw;
                }
                catch (...)
                {
                    std::string sErr = (boost::format("ParseSqlParms Error.\n%s\n%s") % sFmt % sSQL).str();
                    Srv().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
                    throw;
                }
            }

            // 设置参数类型
            void SetParmsType(void)
            {
                const IKCJson& jsonParmsType = CtrlD().JsonAttach().GetItem(c_RESTful_parmType);
                if (jsonParmsType.IsValid())
                    for (int i = 0, c = jsonParmsType.GetItemCount(); i < c; ++i)
                    {
                        const IKCJson& jsonParm = jsonParmsType.GetItem(i);
                        std::string sName = jsonParm.GetName();
                        std::string sType = jsonParm.GetStr();
                        boost::algorithm::to_lower(sType);
                        std::cout << "Param Type: " << sName << " => " << sType << std::endl;
                        Srv().WriteLogTrace(("Param Type: " + sName + " => " + sType).c_str(), __CURR_CODE_PLACE_C__);
                        // 设置参数类型
                        auto it = mapParms.find(sName);
                        if (mapParms.end() != it && EDataType::edtUnknown == it->second->eDTp)
                            it->second->SetDbTp(sType);
                    }
            }

            // 重置参数
            void ResetParms(void)
            {
                this->UnbindParms();
                for (auto &parm : mapParms) parm.second->Init();
                for (auto &parm : mapOutParms)
                {
                    // parm.second->sVal.clear();
                    parm.second->anyVal.clear();
                    parm.second->bIsNull = true;
                    auto it = mapParms.find(parm.first);
                    if (mapParms.end() != it)
                    {
                        it->second->ePTp = parm.second->ePTp;
                        it->second->SetDbTp(parm.second->eDTp);
                    }
                }
            }

            // 绑定参数
            void BindParmsImpl(const IKCJson& jsonParms, const TRowData* rData = nullptr, const unsigned row = 0, const unsigned count = 1)
            {
                IActionData& act = CtrlD().ActionData();
                for (auto &parm : mapParms)
                {
                    // 参数名
                    const std::string prmName = parm.first;
                    // 参数
                    auto prmPtr = parm.second;
                    // 设置全局参数
                    const char* pGlobal = nullptr;
                    auto fSetGlobalParm = [&](void)
                    {
                        prmPtr->bIsNull = false;
                        prmPtr->anyVal = CUtilFunc::PCharSafeToStr(pGlobal);
                        prmPtr->SetDbTp();
                        // 按数据类型，设置数据库参数值
                        if (IsNeedParmBindInVal() && prmPtr->IsValidSetXX())
                        {
                            if (EDataType::edtInt == prmPtr->eDTp) prmPtr->SetInt(atoi(pGlobal));
                            else if (EDataType::edtNumber == prmPtr->eDTp) prmPtr->SetNumber(atof(pGlobal));
                            else if (EDataType::edtClob == prmPtr->eDTp) prmPtr->SetClob(pGlobal);
                            else if (EDataType::edtString == prmPtr->eDTp || EDataType::edtDatetime == prmPtr->eDTp) prmPtr->SetString(pGlobal);
                            // 未指定全局参数的类型，默认为字符串类型
                            else prmPtr->SetString(pGlobal);
                        }
                        // 设置全局参数值
                        // prmPtr->sVal = pGlobal;
                        auto it = mapSessionParms.find(prmName);
                        if (mapSessionParms.end() != it)
                        {
                            it->second->bIsNull = prmPtr->bIsNull;
                            it->second->anyVal = prmPtr->anyVal;
                        }
                        // 参数类型
                        prmPtr->ePTp = EParmType::eptSessionParm;
                    };
                    // 设置用户参数
                    auto fSetParm = [&](const IKCJson& jsonParm) -> bool
                    {
                        if (!jsonParm.IsValid()) return false;
                        // 是否调用数据库设置参数的函数
                        const bool bCallSetXX = IsNeedParmBindInVal() && prmPtr->IsValidSetXX();
                        // json数据类型
                        auto jsonType = jsonParm.GetType();
                        // 数据为空
                        if (IKCJson::edtNull == jsonType)
                        {
                            prmPtr->bIsNull = true;
                            if (bCallSetXX) prmPtr->SetNull();
                            // prmPtr->sVal.clear();
                            prmPtr->anyVal.clear();
                            // prmPtr->SetDbTp(EDataType::edtString);
                            prmPtr->SetDbTp();
                        }
                        // 按数据类型，设置数据库参数值
                        else
                        {
                            prmPtr->bIsNull = false;
                            //  整型
                            if (EDataType::edtInt == prmPtr->eDTp)
                            {
                                if (IKCJson::edtNumber == jsonType)
                                {
                                    auto fVal = jsonParm.GetVal(0);
                                    if (bCallSetXX) prmPtr->SetInt(static_cast<int>(fVal));
                                    // prmPtr->sVal = std::to_string(static_cast<int>(fVal));
                                    prmPtr->anyVal = static_cast<int>(fVal);
                                }
                                else
                                {
                                    const char* pVal = CUtilFunc::PCharSafeToPChar(jsonParm.GetStr(""));
                                    if (bCallSetXX) prmPtr->SetInt(atoi(pVal));
                                    // prmPtr->sVal = std::to_string(atoi(pVal));
                                    prmPtr->anyVal = atoi(pVal);
                                }
                            }
                            // 浮点型
                            else if (EDataType::edtNumber == prmPtr->eDTp)
                            {
                                if (IKCJson::edtNumber == jsonType)
                                {
                                    auto fVal = jsonParm.GetVal(0);
                                    if (bCallSetXX) prmPtr->SetNumber(fVal);
                                    // prmPtr->sVal = std::to_string(fVal);
                                    prmPtr->anyVal = fVal;
                                }
                                else
                                {
                                    const char* pVal = CUtilFunc::PCharSafeToPChar(jsonParm.GetStr(""));
                                    if (bCallSetXX) prmPtr->SetNumber(atof(pVal));
                                    // prmPtr->sVal = std::to_string(atof(pVal));
                                    prmPtr->anyVal = atof(pVal);
                                }
                            }
                            // 长字符串型
                            else if (EDataType::edtClob == prmPtr->eDTp)
                            {
                                std::string strVal = CUtilFunc::PCharSafeToStr(jsonParm.GetStr(""));
                                if (bCallSetXX) prmPtr->SetClob(strVal);
                                // prmPtr->sVal = strVal;
                                prmPtr->anyVal = strVal;
                            }
                            // 字符型
                            else if (EDataType::edtString == prmPtr->eDTp)
                            {
                                std::string strVal = CUtilFunc::PCharSafeToStr(jsonParm.GetStr(""));
                                if (bCallSetXX) prmPtr->SetString(strVal);
                                // prmPtr->sVal = strVal;
                                prmPtr->anyVal = strVal;
                            }
                            // 日期型
                            else if (EDataType::edtDatetime == prmPtr->eDTp)
                            {
                                if (IKCJson::edtNumber == jsonType)
                                {
                                    auto fVal = jsonParm.GetVal(0);
                                    if (bCallSetXX) prmPtr->SetNumber(fVal);
                                    // prmPtr->sVal = std::to_string(fVal);
                                    prmPtr->anyVal = fVal;
                                }
                                else
                                {
                                    std::string strVal = CUtilFunc::PCharSafeToStr(jsonParm.GetStr(""));
                                    if (bCallSetXX) prmPtr->SetString(strVal);
                                    // prmPtr->sVal = strVal;
                                    prmPtr->anyVal = strVal;
                                }
                            }
                            // 未指定类型，按json的类型
                            else
                            {
                                // 数值类型
                                if (IKCJson::edtNumber == jsonType)
                                {
                                    auto fVal = jsonParm.GetVal(0);
                                    // 按整型
                                    if (!m_isBatchOp && CUtilFunc::isInt(fVal))
                                    {
                                        int iVal = static_cast<int>(fVal);
                                        if (bCallSetXX) prmPtr->SetInt(iVal);
                                        // prmPtr->sVal = std::to_string(iVal);
                                        prmPtr->anyVal = iVal;
                                        // prmPtr->SetDbTp(EDataType::edtInt);
                                        prmPtr->SetDbTp();
                                    }
                                    // 按数值型
                                    else
                                    {
                                        bool isInt = false;
                                        if (bCallSetXX) isInt = prmPtr->SetNumber(fVal);
                                        if (CUtilFunc::isLLong(fVal) || isInt)
                                        {
                                            // prmPtr->sVal = std::to_string(static_cast<long long>(fVal));
                                            prmPtr->anyVal = static_cast<long long>(fVal);
                                            // prmPtr->SetDbTp(EDataType::edtInt);
                                            prmPtr->SetDbTp();
                                        }
                                        else
                                        {
                                            // prmPtr->sVal = std::to_string(fVal);
                                            prmPtr->anyVal = fVal;
                                            // prmPtr->SetDbTp(EDataType::edtNumber);
                                            prmPtr->SetDbTp();
                                        }
                                    }
                                }
                                // 其他类型，按字符串
                                else
                                {
                                    std::string strVal = CUtilFunc::PCharSafeToStr(jsonParm.GetStr(""));
                                    if (bCallSetXX) prmPtr->SetString(strVal);
                                    // prmPtr->sVal = strVal;
                                    prmPtr->anyVal = strVal;
                                    // prmPtr->SetDbTp(EDataType::edtString);
                                    prmPtr->SetDbTp();
                                }
                            }
                        }
                        return true;
                    };
                    // 设置内部数据集
                    auto fSetRowData = [&](const TRowData& rData)
                    {
                        auto it = rData.m_valMap.find(prmName);
                        if (rData.m_valMap.end() == it) return false;
                        // 设置参数值
                        prmPtr->anyVal = it->second->m_val;
                        prmPtr->bIsNull = prmPtr->anyVal.empty();
                        prmPtr->SetDbTp();
                        // 按数据类型，设置数据库参数值
                        if (IsNeedParmBindInVal() && prmPtr->IsValidSetXX())
                        {
                            // 数据为空
                            if (prmPtr->bIsNull) prmPtr->SetNull();
                            //  整型
                            if (EDataType::edtInt == prmPtr->eDTp)
                            {
                                if (const auto* ptr = boost::any_cast<int>(&prmPtr->anyVal)) prmPtr->SetInt(*ptr);
                                else if (const auto* ptr = boost::any_cast<long long>(&prmPtr->anyVal))
                                    prmPtr->SetInt(static_cast<int>(*ptr));
                                else if (const auto* ptr = boost::any_cast<double>(&prmPtr->anyVal))
                                    prmPtr->SetInt(static_cast<int>(*ptr));
                                else
                                {
                                    std::string sVal = BoostAnyToStdString(prmPtr->anyVal);
                                    prmPtr->SetInt(atoi(sVal.c_str()));
                                }
                            }
                            // 浮点型
                            else if (EDataType::edtNumber == prmPtr->eDTp)
                            {
                                if (const auto* ptr = boost::any_cast<double>(&prmPtr->anyVal)) prmPtr->SetNumber(*ptr);
                                else if (const auto* ptr = boost::any_cast<int>(&prmPtr->anyVal)) prmPtr->SetNumber(*ptr);
                                else if (const auto* ptr = boost::any_cast<long long>(&prmPtr->anyVal)) prmPtr->SetNumber(*ptr);
                                else
                                {
                                    std::string sVal = BoostAnyToStdString(prmPtr->anyVal);
                                    prmPtr->SetNumber(atof(sVal.c_str()));
                                }
                            }
                            // 长字符串型
                            else if (EDataType::edtClob == prmPtr->eDTp)
                            {
                                std::string strVal = BoostAnyToStdString(prmPtr->anyVal);
                                prmPtr->SetClob(strVal);
                            }
                            // 字符型
                            else if (EDataType::edtString == prmPtr->eDTp)
                            {
                                std::string strVal = BoostAnyToStdString(prmPtr->anyVal);
                                prmPtr->SetString(strVal);
                            }
                            // 日期型
                            else if (EDataType::edtDatetime == prmPtr->eDTp)
                            {
                                if (const auto* ptr = boost::any_cast<double>(&prmPtr->anyVal)) prmPtr->SetNumber(*ptr);
                                else
                                {
                                    std::string strVal = BoostAnyToStdString(prmPtr->anyVal);
                                    prmPtr->SetString(strVal);
                                }
                            }
                            // 未指定类型，按值的类型
                            else
                            {
                                // 数值类型
                                if (const auto* ptr = boost::any_cast<double>(&prmPtr->anyVal))
                                {
                                    // 按整型
                                    if (!m_isBatchOp && CUtilFunc::isInt(*ptr)) prmPtr->SetInt(static_cast<int>(*ptr));
                                    // 按数值型
                                    else prmPtr->SetNumber(*ptr);
                                }
                                // 整型
                                else if (const auto* ptr = boost::any_cast<int>(&prmPtr->anyVal)) prmPtr->SetInt(*ptr);
                                else if (const auto* ptr = boost::any_cast<long long>(&prmPtr->anyVal)) prmPtr->SetNumber(*ptr);
                                // 其他类型，按字符串
                                else
                                {
                                    std::string strVal = BoostAnyToStdString(prmPtr->anyVal);
                                    prmPtr->SetString(strVal);
                                }
                            }
                        }
                        return true;
                    };
                    // 设置参数
                    try
                    {
                        // 内置参数：批量导入的行号
                        if (prmName == c_RESTful_batchRowID)
                        {
                            prmPtr->ePTp = EParmType::eptInParm;
                            prmPtr->eDTp = EDataType::edtInt;
                            prmPtr->anyVal = row;
                            prmPtr->SetDbTp();
                            if (IsNeedParmBindInVal()) prmPtr->SetNumber(row);
                        }
                        // 内置参数：批量导入的行数
                        else if (prmName == c_RESTful_batchRowCount)
                        {
                            prmPtr->ePTp = EParmType::eptInParm;
                            prmPtr->eDTp = EDataType::edtInt;
                            prmPtr->anyVal = count;
                            prmPtr->SetDbTp();
                            if (IsNeedParmBindInVal()) prmPtr->SetNumber(count);
                        }
                        // 动态参数
                        else
                        {
                            // 绑定参数
                            bool bHasBind =
                                    // 来自内部数据集
                                    (nullptr != rData && fSetRowData(*rData))
                                    // 来自数据集（vals），或其他外部输入参数
                                    || (jsonParms.IsValid() && fSetParm(jsonParms.GetItem(prmName.c_str())))
                                    // 到控制器参数里找
                                    || fSetParm(CtrlD().GetParmJson(prmName.c_str()))
                                    ;
                            if (bHasBind)
                                prmPtr->ePTp = prmPtr->ePTp == EParmType::eptOutParm ? EParmType::eptInOutParm : EParmType::eptInParm;
                            // 绑定全局参数
                            else if ((bHasBind = (pGlobal = act.GetGlobalVal(prmName.c_str())) != nullptr))
                            {
                                fSetGlobalParm();
                                // std::string strVal = parm.second->sVal;
                                std::string strVal = BoostAnyToStdString(parm.second->anyVal);
                                Srv().WriteLogTrace(("Global Param Value: " + prmName + " = " + strVal).c_str(), __CURR_CODE_PLACE_C__);
                            }
                            // 调试
                            if (row % 10000 == 0)
                            {
                                if (bHasBind)
                                {
                                    // std::string strVal = parm.second->sVal;
                                    std::string strVal = BoostAnyToStdString(parm.second->anyVal);
                                    // std::cout << "\t*[Knewcode] " << prmName << "=" << strVal << std::endl;
                                    Srv().WriteLogTrace(("User Param Value: " + prmName + " = " + strVal).c_str(), __CURR_CODE_PLACE_C__);
                                }
                                else if (prmPtr->ePTp != EParmType::eptOutParm)
                                {
                                    cout << "\t*[Knewcode] " << "Not Set Value: " << prmName << endl;
                                    Srv().WriteLogTrace(("User Param Value: " + prmName + " 未设定参数值").c_str(), __CURR_CODE_PLACE_C__);
                                }
                            }
                        }
                    }
                    catch(TExcept& ex)
                    {
                        std::string sErr = ParmExceptInfo(*prmPtr, ParmExceptInfo(ex));
                        act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                        throw std::runtime_error("\"" + prmName + "\" " + ParmExceptInfo(ex));
                    }
                    catch(std::exception& ex)
                    {
                        std::string sErr = ParmExceptInfo(*prmPtr, ex.what());
                        act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                        throw std::runtime_error("\"" + prmName + "\" " + ex.what());
                    }
                    catch(...)
                    {
                        std::string sErr = ParmExceptInfo(*prmPtr, "Unknown error");
                        act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__);
                        throw std::runtime_error("\"" + prmName + "\"");
                    }
                }
            }

            // 判断是否遗漏参数
            void LeakParms(void)
            {
                std::string sLostParm = "";
                for (auto parm : mapParms)
                    if (EParmType::eptUnknown == parm.second->ePTp)
                        sLostParm += parm.first + ",";
                boost::algorithm::trim_right_if(sLostParm, boost::algorithm::is_any_of(","));
                if (!sLostParm.empty()) throw TKCSqlException(203, __CURR_CODE_PLACE_C__, Srv().getHint("Parm_Error_") + sLostParm, Srv());
            }
            // 是否检查遗漏参数（只DML语句检查）
            virtual bool IsCheckLeakParms() {return IsNeedParmBindInVal(); }

            // 绑定参数
            void BindParms(void)
            {
                BindParms(CtrlD().ActionData().InvalidJson());
            }
            void BindParms(const IKCJson& jsonParms, const TRowData* rData = nullptr, const unsigned row = 0, const unsigned count = 1)
            {
                auto fGetInfo = [&](std::string sErr = "")
                {
                    return  Srv().getHint("Parm_Error_") + sErr;
                };
                try
                {
                    // SetParmsType();
                    BindParmsImpl(jsonParms, rData, row, count);
                    if (IsCheckLeakParms()) LeakParms();
                }
                catch (TKCSqlException&)
                {
                    throw;
                }
                catch(TExcept& ex)
                {
                    throw TKCSqlException(200, __CURR_CODE_PLACE_C__, fGetInfo(ParmExceptInfo(ex)), Srv());
                }
                catch (std::exception& ex)
                {
                    std::cout << CBacktraceSymbols::Get() << std::endl;
                    throw TKCSqlException(201, __CURR_CODE_PLACE_C__, fGetInfo(ex.what()), Srv());
                }
                catch (...)
                {
                    throw TKCSqlException(202, __CURR_CODE_PLACE_C__, fGetInfo(), Srv());
                }
            }

            // 解析输出参数
            void ParseOutParms(void)
            {
                // 控制器附加参数里的输出参数
                TParmGrp<TExcept>::ParseOutParms(mapOutParms, CtrlD().ActionData().GetValsName(), CtrlD().JsonAttach().GetItem(c_RESTful_outParm), [&](std::string sName, unsigned) -> TParmPtr{ return this->MakePram(sName); });
            }
            static void ParseOutParms(std::map<std::string, TParmPtr, TLessStr> &mapOutParms, const std::string sValsName, const IKCJson& jsonOutP, std::function<TParmPtr(std::string, unsigned)> fMkPrm, std::vector<std::string> *pPath = nullptr)
            {
                bool bHasFirstRootDataSet = true;
                std::vector<std::string> pth;
                // 根节点
                if (nullptr == pPath)
                    bHasFirstRootDataSet = false;
                // 子节点
                else
                {
                    if (!pPath->empty()) pth.assign(pPath->begin(), pPath->end());
                    pth.push_back(jsonOutP.GetName());
                }
                // 循环本节点
                if (jsonOutP.IsValid())
                    for (int i = 0, c = jsonOutP.GetItemCount(); i < c; ++i)
                    {
                        const IKCJson& jsonParm = jsonOutP.GetItem(i);
                        if (jsonParm.GetType() == IKCJson::edtObject)
                            ParseOutParms(mapOutParms, sValsName, jsonParm, fMkPrm, &pth);
                        else
                        {
                            std::string sName = jsonParm.GetName();
                            std::string sType = jsonParm.GetStr();
                            //boost::algorithm::to_lower(sName);      // 参数名不区分大小写
                            boost::algorithm::to_lower(sType);
                            // 存入输出参数
                            auto it = mapOutParms.find(sName);
                            if (mapOutParms.end() == it)
                            {
                                std::cout << "\t*[Knewcode] Out Param: " << sName << " / " << sType << std::endl;
                                auto parm = fMkPrm(sName, 0);
                                parm->ePTp = EParmType::eptOutParm;
                                parm->SetDbTp(sType);
                                if (!pth.empty()) parm->m_path.assign(pth.begin(), pth.end());
                                mapOutParms.insert(make_pair(sName, parm));
                                // 根节点内出现首个数据集（并且名字为vals）
                                if (!bHasFirstRootDataSet && EDataType::edtDbSet == parm->eDTp && sValsName == sName)
                                    bHasFirstRootDataSet = parm->m_isFirstDataSet = true;
                            }
                            /*
                            it = mapOutParms.find(sName);
                            if (mapOutParms.end() != it)
                            {
                                it->second->ePTp = EParmType::eptOutParm;
                                it->second->SetDbTp(sType);
                            }
                            */
                        }
                    }
            }

            // 注册输出参数
            std::string RegOutParms(void)
            {
                std::string sInfo = "RegOutParms: ";
                IActionData& act = CtrlD().ActionData();
                // 注册输出参数
                for (auto &op : mapOutParms)
                {
                    // 该参数存在于SQL语句中
                    auto it = mapParms.find(op.first);
                    if (mapParms.end() != it)
                        try
                        {
                            op.second->iPos = it->second->iPos;
                            it->second->ePTp = op.second->ePTp;
                            it->second->SetDbTp(op.second->eDTp);
                            op.second->RegOutParam();
                            sInfo += (boost::format("%s @%d $%d \t") % op.first % it->second->iPos % static_cast<int>(it->second->eDTp)).str();
                        }
                        catch(TExcept& ex)
                        {
                            std::string sErr = ParmExceptInfo(*it->second, ParmExceptInfo(ex));
                            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                            throw;
                        }
                        catch (std::exception& ex)
                        {
                            std::string sErr = ParmExceptInfo(*it->second, ex.what());
                            act.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                            throw;
                        }
                        catch (...)
                        {
                            std::string sErr = ParmExceptInfo(*it->second, "Unknown error");
                            act.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
                            throw;
                        }
                }
                sInfo += "\n";
                // 注册Session输出参数
                for (auto &op : mapParms)
                {
                    auto eGloVTp = act.GetGlobalValType(op.first.c_str());
                    if (IActionData::egtSession == eGloVTp || IActionData::egtAction == eGloVTp || IActionData::egtJson == eGloVTp)
                    {
                        try
                        {
                            op.second->ePTp = EParmType::eptSessionParm;
                            if (EDataType::edtUnknown == op.second->eDTp) op.second->SetDbTp(EDataType::edtString);
                            op.second->RegOutParam();
                            auto parmSession = MakePram(op.first, op.second->iPos);
                            parmSession->ePTp = EParmType::eptSessionParm;
                            parmSession->eDTp = op.second->eDTp;
                            mapSessionParms.insert(std::make_pair(op.first, parmSession));
                            sInfo += (boost::format("%s @%d \t") % op.first % op.second->iPos).str();
                        }
                        catch(TExcept& ex)
                        {
                            std::string sErr = ParmExceptInfo(*op.second, ParmExceptInfo(ex));
                            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                            throw;
                        }
                        catch(std::exception& ex)
                        {
                            std::string sErr = ParmExceptInfo(*op.second, ex.what());
                            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                            throw;
                        }
                        catch(...)
                        {
                            std::string sErr = ParmExceptInfo(*op.second, "Unknown error");
                            act.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__);
                            throw;
                        }
                    }
                }
                return sInfo;
            }

            // 设置Session
            void OutParmsToSession(void)
            {
                IActionData& act = CtrlD().ActionData();
                for (auto &op : mapSessionParms)
                {
                    // 为空
                    // if (op.second->IsNull() && !op.second->sVal.empty())
                    if (op.second->IsNull() && !op.second->anyVal.empty())
                    {
                        // op.second->sVal.clear();
                        op.second->anyVal.clear();
                        act.SetGlobalVal(op.first.c_str(), "");
                    }
                    // 整型
                    else if (EDataType::edtInt == op.second->eDTp)
                    {
                        int iVal = op.second->GetInt();
                        // if (atoi(op.second->sVal.c_str()) != iVal)
                        if (op.second->anyVal.type() != typeid(int) || boost::any_cast<int>(op.second->anyVal) != iVal)
                        {
                            // op.second->sVal = std::to_string(iVal);
                            op.second->anyVal = iVal;
                            act.SetGlobalVal(op.first.c_str(), (std::to_string(iVal)).c_str());
                        }
                    }
                    // 浮点型
                    else if (EDataType::edtNumber == op.second->eDTp)
                    {
                        double fVal = op.second->GetNumber();
                        // if (!CUtilFunc::isEqual(atof(op.second->sVal.c_str()), fVal))
                        if (op.second->anyVal.type() != typeid(double) || !CUtilFunc::isEqual(boost::any_cast<double>(op.second->anyVal), fVal))
                        {
                            // op.second->sVal = std::to_string(fVal);
                            op.second->anyVal = fVal;
                            act.SetGlobalVal(op.first.c_str(), (std::to_string(fVal)).c_str());
                        }
                    }
                    // 长字符串、json
                    else if (EDataType::edtClob == op.second->eDTp || EDataType::edtJson == op.second->eDTp)
                    {
                        std::string sVal = op.second->GetClob();
                        // if (op.second->sVal != sVal)
                        if (op.second->anyVal.type() != typeid(std::string) || boost::any_cast<std::string>(op.second->anyVal) != sVal)
                        {
                            // op.second->sVal = sVal;
                            op.second->anyVal = sVal;
                            act.SetGlobalVal(op.first.c_str(), sVal.c_str());
                        }
                    }
                    // 其他类型：字符串、日期等
                    else
                    {
                        std::string sVal = op.second->GetString();
                        // if (op.second->sVal != sVal)
                        if (op.second->anyVal.type() != typeid(std::string) || boost::any_cast<std::string>(op.second->anyVal) != sVal)
                        {
                            // op.second->sVal = sVal;
                            op.second->anyVal = sVal;
                            act.SetGlobalVal(op.first.c_str(), sVal.c_str());
                        }
                    }
                    // 是否空
                    // op.second->bIsNull = op.second->sVal.empty();
                    op.second->bIsNull = op.second->anyVal.empty();
                }
            }

            // 输出参数到json
            int OutParmsToJson(void)
            {
                IActionData& act = CtrlD().ActionData();
                int iRecCount = 0;
                if (!mapOutParms.empty())
                {
                    // json应答
                    IKCJson& jsonRespond = CtrlD().JsonRespond();
                    bool bHasCursor = jsonRespond.GetItem(CtrlD().ActionData().GetValsName()).IsValid();
                    for (auto &op : mapOutParms)
                    {
                        // 返回SQL语句中有位置的参数（或临时表）
                        if (op.second->iPos > 0 || op.second->IsTempTable())
                        {
                            // 输出json的路径
                            IKCJson *jsonPth = &jsonRespond;
                            for (auto pth : op.second->m_path)
                                jsonPth = &jsonPth->AddItem(pth.c_str(), false);
                            // 内部数据集不输出json
                            if (EDataType::edtDbSetInner == op.second->eDTp)
                            {
                                TRecordSetPtr rset = op.second->GetDbSet();
                                if (rset.get() == nullptr)
                                {
                                    // throw TKCSqlException(207, __CURR_CODE_PLACE_C__, Srv().getHint("Don_t_exists_data_") + op.first, typeid(*this).name());
                                    act.WriteLogWarning(("No Data - " + op.first).c_str(), __CURR_CODE_PLACE_C__, typeid(*this).name());
                                }
                                else
                                {
                                    rset->m_name = op.first;
                                    rset->DataSetSaveToInner();
                                }
                            }
                            // 数据集（数组等）
                            else if (EDataType::edtDbSet == op.second->eDTp || EDataType::edtDbSetOne == op.second->eDTp || EDataType::edtDbSetArray == op.second->eDTp)
                            {
                                TRecordSetPtr rset = op.second->GetDbSet();
                                if (rset.get() == nullptr)
                                {
                                    // throw TKCSqlException(207, __CURR_CODE_PLACE_C__, Srv().getHint("Don_t_exists_data_") + op.first, typeid(*this).name());
                                    act.WriteLogWarning(("No Data - " + op.first).c_str(), __CURR_CODE_PLACE_C__, typeid(*this).name());
                                }
                                else
                                {
                                    rset->m_name = op.first;
                                    // 首个根路径数据集
                                    if (!bHasCursor && op.second->m_isFirstDataSet)
                                    {
                                        bHasCursor = true;
                                        if (rset.get() != nullptr)
                                            iRecCount = rset->DataSetToJson(jsonRespond);
                                    }
                                    // 其他数据集（数组等）
                                    else
                                    {
                                        jsonPth->DelItem(op.first.c_str());
                                        if (rset.get() != nullptr)
                                        {
                                            IKCJson& jset = EDataType::edtDbSetArray == op.second->eDTp ? jsonPth->AddArray(op.first.c_str(), false)
                                                                                                        : jsonPth->AddItem(op.first.c_str(), false);
                                            // 按类型添加数据集
                                            if (EDataType::edtDbSetOne == op.second->eDTp)
                                                rset->DataSetToJsonOne(jset);
                                            else if (EDataType::edtDbSetArray == op.second->eDTp)
                                                rset->DataSetToJsonArray(jset);
                                            else
                                                rset->DataSetToJson(jset, false);
                                        }
                                    }
                                }
                            }
                            // 其他类型参数
                            else
                            {
                                IKCJson& jsonVar = *jsonPth;
                                if (op.second->IsNull())
                                    jsonVar.SetNull(op.first.c_str());
                                else if (EDataType::edtInt == op.second->eDTp)
                                    jsonVar.SetVal(op.first.c_str(), op.second->GetInt());
                                else if (EDataType::edtNumber == op.second->eDTp)
                                    jsonVar.SetVal(op.first.c_str(), op.second->GetNumber());
                                else if (EDataType::edtDatetime == op.second->eDTp)
                                    jsonVar.SetStr(op.first.c_str(), op.second->GetDateTime().c_str());
                                else if (EDataType::edtClob == op.second->eDTp)
                                    jsonVar.SetStr(op.first.c_str(), op.second->GetClob().c_str());
                                else if (EDataType::edtJson == op.second->eDTp)
                                    jsonVar.SetJson(op.first.c_str(), op.second->GetString().c_str());
                                else
                                    jsonVar.SetStr(op.first.c_str(), op.second->GetString().c_str());
                            }
                        }
                        // 必须返回的参数
                        else if (c_RESTful_errCode == op.first && op.second->m_path.empty() && EDataType::edtInt == op.second->eDTp)
                        {
                            jsonRespond.SetVal(c_RESTful_errCode, 0);
                        }
                        else if (c_RESTful_errMsg == op.first && op.second->m_path.empty() && EDataType::edtString == op.second->eDTp)
                        {
                            jsonRespond.SetStr(c_RESTful_errMsg, "");
                        }
                    }
                }
                return iRecCount;
            }

            // 解析sql语句里的临时表（目前仅用于odbc的mssql）
            void ParseSqlTmpTab(std::string sSQL, std::string sFmt = std::string("##") + c_RESTful_KCTmpTab + "\\w+")
            {
                try
                {
                    sSQL = CCtrlCommon::RemoveCommentAndStr(sSQL);
                    // 分析SQL语句中的临时表
                    std::regex pattern(sFmt, std::regex::icase);
                    for (std::sregex_iterator it(sSQL.cbegin(), sSQL.cend(), pattern), end_it; end_it != it; ++it)
                    {
                        std::string sTmpTab = it->str();
                        auto itTmpTab = mapTmpTabs.find(sTmpTab);
                        if (mapTmpTabs.end() == itTmpTab)
                            mapTmpTabs.insert(std::make_pair(sTmpTab, sTmpTab));
                    }
                }
                catch (std::exception &ex)
                {
                    std::string sErr = (boost::format("ParseSqlTmpTab Error: %s\n%s\n%s") % ex.what() % sFmt % sSQL).str();
                    Srv().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
                    throw;
                }
                catch (...)
                {
                    std::string sErr = (boost::format("ParseSqlTmpTab Error.\n%s\n%s") % sFmt % sSQL).str();
                    Srv().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
                    throw;
                }
            }
        };

        // 数据库链接
        template<typename TConnObj>
        class TConnObjWork
        {
        public:
            IActionData& m_act;
            std::string m_keepAlive = "off";
            std::string m_actObjName;
            TConnObj* m_actObj = nullptr;

            TConnObjWork(ICtrlApiData& ctrl/*, std::string sCtrlName*/, std::function<TConnObj*(void)> fCtor) : m_act(ctrl.ActionData())
            {
                // 根标识
                std::string sRootSign = m_act.GetGlobalVal((c_RESTful_KCAct + std::string("") + c_RESTful_rootSign).c_str());
                // 数据库链接对象名称
                // m_actObjName = sCtrlName + "~" + sRootSign;
                m_actObjName = CUtilFunc::PCharSafeToStr(ctrl.CtrlName()) + "@" + CUtilFunc::PCharSafeToStr(ctrl.Srv()) + "~" + sRootSign;
                // 是否持久链接
                m_keepAlive = m_act.GetGlobalVal((c_RESTful_KCAct + std::string("") + c_RESTful_keepAlive).c_str());
                // 如果是持久链接，则到进程主控类对象中查找
                if ("on" == m_keepAlive || "first" == m_keepAlive)
                {
                    IActionData::TAliveObj& aliveObj = m_act.GetAliveObj();
                    m_actObj = dynamic_cast<TConnObj*>(aliveObj.GetActObj(m_actObjName.c_str()));
                    if (nullptr == m_actObj)
                    {
                        auto dtBegin = CUtilFunc::GetCurrentStampMS();
                        m_actObj = fCtor();
                        m_act.WriteLogTrace((boost::format("Create Progress Object Connect - %s, Expend %d MS") % m_actObjName % (CUtilFunc::GetCurrentStampMS() - dtBegin)).str().c_str(), __CURR_CODE_PLACE_C__);
                        aliveObj.AddActObj(m_actObjName.c_str(), m_actObj);
                        m_act.SetGlobalVal((c_RESTful_KCAct + std::string("") + c_RESTful_keepAlive).c_str(), "first");
                    }
                }
                // 如果不是持久对象，则到活动页对象中查找
                else
                {
                    m_actObj = dynamic_cast<TConnObj*>(m_act.GetActObj(m_actObjName.c_str()));
                    if (nullptr == m_actObj)
                    {
                        auto dtBegin = CUtilFunc::GetCurrentStampMS();
                        m_actObj = fCtor();
                        m_act.WriteLogTrace((boost::format("Create Action Object Connect - %s, Expend %d MS") % m_actObjName % (CUtilFunc::GetCurrentStampMS() - dtBegin)).str().c_str(), __CURR_CODE_PLACE_C__);
                        m_act.AddActObj(m_actObjName.c_str(), m_actObj);
                    }
                }
            }

            ~TConnObjWork(void)
            {
                // 持久对象
                if ("on" == m_keepAlive || "first" == m_keepAlive)
                {
                    std::string sKeepAliveEnd = m_act.GetGlobalVal((c_RESTful_KCAct + std::string("") + c_RESTful_keepAlive).c_str());
                    // 删除持久对象
                    if ("off" == sKeepAliveEnd)
                        m_act.GetAliveObj().DelActObj(m_actObjName.c_str());
                }
            }

            TConnObj& ActObj(void)
            {
                if (nullptr == m_actObj)
                    throw TKCSqlException(204, __CURR_CODE_PLACE_C__, "NULL Alive Object - " + m_actObjName + " - " + m_act.GetAliveObj().AliveID(), typeid(TConnObj).name());
                return *m_actObj;
            }
        };

        // 数据库执行命令
        template<typename TExcept>
        struct TDBCommand
        {
            // 执行顺序号
            static inline long long s_execSort = 0;
            const long long m_execSort = ++s_execSort;
            const std::string m_sExecSort = std::to_string(m_execSort);

            // 所属的服务
            virtual IKCSql& Srv(void) = 0;

            // 控制器数据接口
            virtual ICtrlApiData& CtrlD(void) = 0;

            //  控制器 信息
            virtual std::string ActInfo(void) = 0;

            // SQL语句
            virtual std::string GetSQL(void) = 0;

            // 参数组
            typedef TParmGrp<TExcept> TParms;
            virtual TParms& GetParms(void) = 0;

            // 检查错误
            void PreCheck(void)
            {
                if (this->GetSQL().empty())
                    throw TKCSqlException(205, __CURR_CODE_PLACE_C__, "Empty SQL", typeid(*this).name());
            }

            // 异常信息
            virtual std::tuple<int, std::string, std::string> ParmExceptInfo(TExcept&) = 0;

            // 事务
            virtual void TranBegin(void) = 0;
            virtual void TranCommit(void) = 0;
            virtual void TranRollback(void) = 0;
            // 以下，只针对postgresql
            virtual void TranSavePoint(std::string) {}
            virtual void TranRollbackToSavePoint(void) {};

            // 预执行
            virtual void PrepareSQL(void) = 0;

            // 批量操作
            virtual std::pair<int, std::string> ExecuteBatch(void) = 0;
            // 批量插入结束
            virtual void BatchInsertEnd(bool) {}

            // 增删改
            virtual unsigned int ExecuteUpdate(void) = 0;

            // 查询
            virtual TRecordSetPtr ExecuteQuery(int &rows_affected) = 0;
            virtual TRecordSetPtr NextResult(void) { return CSqlFunc::TRecordSetPtr(); }

            // 统一获取参数值
            virtual void MakeParmVals(void) {}

            // 执行过程
            virtual int ExecuteProcedures(void) = 0;

            // 执行sql语句
            void RunSQL(void)
            {
                this->PreCheck();
                CtrlD().SetJsonRespond(0, std::to_string(this->ExecuteUpdate()).c_str());
            }

            // 创建临时对象（视图、表、存储过程、函数等，目前用于Oracle、PostgreSQL、SQL Server）
            void CreateDDL(void)
            {
                // 执行日志
                std::string sMsg = "<C" + m_sExecSort + ".> CreateDDL Begin. [" + CtrlD().LocalFile() + " : " + CtrlD().NodeName() + "]";
                Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                std::cout << sMsg << std::endl;
                CAutoRelease _auto([&](){
                    std::string sMsg = "<C" + m_sExecSort + ".> CreateDDL End.";
                    Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                    std::cout << sMsg << std::endl;
                });
                this->PreCheck();
                // 解析参数
                TParms& parms = this->GetParms();
                parms.sSQLWorkType = c_RESTful_sql_DDL;
                parms.ParseSqlParms(this->GetSQL());
                // 预执行前，绑定参数值
                if (parms.m_isPrePrepareBind) parms.BindParms();
                // 预执行
                this->PrepareSQL();
                // 预执行后，绑定参数值
                if (!parms.m_isPrePrepareBind) parms.BindParms();
                // 执行
                CtrlD().SetJsonRespond(0, std::to_string(this->ExecuteUpdate()).c_str());
                Srv().WriteLogTrace(this->GetSQL().c_str(), __CURR_CODE_PLACE_C__, CtrlD().ActionData().GetSingleInfo("UniqueConnID"));
                //boost::this_thread::sleep(boost::posix_time::milliseconds(5));
            }

            // 增删改
            void InsertDeleteUpdate(void)
            {
                // 执行日志
                std::string sMsg = "<U" + m_sExecSort + ".> InsertDeleteUpdate Begin. [" + CtrlD().LocalFile() + " : " + CtrlD().NodeName() + "]";
                Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                std::cout << sMsg << std::endl;
                CAutoRelease _auto([&](){
                    std::string sMsg = "<U" + m_sExecSort + ".> InsertDeleteUpdate End.";
                    Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                    std::cout << sMsg << std::endl;
                });
                this->PreCheck();
                // 解析参数
                TParms& parms = this->GetParms();
                parms.sSQLWorkType = c_RESTful_sql_DML;
                parms.ParseSqlParms(this->GetSQL());
                // 解析输出参数
                parms.ParseOutParms();
                // 预执行前，绑定参数值
                if (parms.m_isPrePrepareBind) parms.BindParms();
                // 预执行
                this->PrepareSQL();
                // 预执行后，绑定参数值
                if (!parms.m_isPrePrepareBind) parms.BindParms();
                // 执行
                CtrlD().SetJsonRespond(0, std::to_string(this->ExecuteUpdate()).c_str());
            }

            // 执行查询，返回数据集json
            void Select(void)
            {
                // 执行日志
                std::string sMsg = "<S" + m_sExecSort + ".> Select Begin. [" + CtrlD().LocalFile() + " : " + CtrlD().NodeName() + "]";
                Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                std::cout << sMsg << std::endl;
                CAutoRelease _auto([&](){
                    std::string sMsg = "<S" + m_sExecSort + ".> Select End.";
                    Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                    std::cout << sMsg << std::endl;
                });
                this->PreCheck();
                // 解析参数
                TParms& parms = this->GetParms();
                parms.sSQLWorkType = c_RESTful_sql_DQL;
                parms.ParseSqlParms(this->GetSQL());
                // 解析输出参数
                parms.ParseOutParms();
                // 执行查询
                auto [iCountRec, __] = SelectQuery(parms);
                // 返回状态
                IKCJson& jsonRespond = CtrlD().JsonRespond();
                if (!jsonRespond.HasItem(c_RESTful_errCode) || jsonRespond.IsNull(c_RESTful_errCode))
                    jsonRespond.SetVal(c_RESTful_errCode, 0);
                if (!jsonRespond.HasItem(c_RESTful_errMsg))
                    jsonRespond.SetStr(c_RESTful_errMsg, std::to_string(iCountRec).c_str());
            }
            // 带返回数据集json的执行（用于odbc、达梦）
            void Query(void)
            {
                // 执行日志
                std::string sMsg = "<Q" + m_sExecSort + ".> Query Begin. [" + CtrlD().LocalFile() + " : " + CtrlD().NodeName() + "]";
                Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                std::cout << sMsg << std::endl;
                CAutoRelease _auto([&](){
                    std::string sMsg = "<Q" + m_sExecSort + ".> Query End.";
                    Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                    std::cout << sMsg << std::endl;
                });
                this->PreCheck();
                // 解析参数
                TParms& parms = this->GetParms();
                parms.sSQLWorkType = c_RESTful_sql_DQL;
                parms.ParseSqlParms(this->GetSQL());
                // 解析输出参数
                parms.ParseOutParms();
                // 注册输出参数
                std::string sInfo = parms.RegOutParms();
                Srv().WriteLogTrace(sInfo.c_str(), __CURR_CODE_PLACE_C__);
                // 执行查询
                auto [iCountRec, rows_affected] = SelectQuery(parms);
                // 从临时表中获取参数值
                this->MakeParmVals();
                // 设置Session
                parms.OutParmsToSession();
                // 返回输出参数
                int iRecCount = parms.OutParmsToJson();
                // 返回
                IKCJson& jsonRespond = CtrlD().JsonRespond();
                if (!jsonRespond.HasItem(c_RESTful_errCode) || jsonRespond.IsNull(c_RESTful_errCode))
                    jsonRespond.SetVal(c_RESTful_errCode, 0);
                if (!jsonRespond.HasItem(c_RESTful_errMsg))
                    jsonRespond.SetStr(c_RESTful_errMsg, std::to_string(std::max({iCountRec, iRecCount, rows_affected})).c_str());
            }

            // 存储过程
            void Procedures(void)
            {
                // 执行日志
                std::string sMsg = "<P" + m_sExecSort + ".> Procedures Begin. [" + CtrlD().LocalFile() + " : " + CtrlD().NodeName() + "]";
                Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                std::cout << sMsg << std::endl;
                CAutoRelease _auto([&](){
                    std::string sMsg = "<P" + m_sExecSort + ".> Procedures End.";
                    Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                    std::cout << sMsg << std::endl;
                });
                this->PreCheck();
                // 解析参数
                TParms& parms = this->GetParms();
                parms.sSQLWorkType = c_RESTful_sql_DML;
                parms.ParseSqlParms(this->GetSQL());
                // 解析输出参数
                parms.ParseOutParms();
                // 注册输出参数
                std::string sInfo = parms.RegOutParms();
                std::cout << "*[Knewcode] " << sInfo << std::endl;
                Srv().WriteLogTrace(sInfo.c_str(), __CURR_CODE_PLACE_C__);
                // 预执行前，绑定参数值
                if (parms.m_isPrePrepareBind) parms.BindParms();
                // 预执行
                this->PrepareSQL();
                // 预执行后，绑定参数值
                if (!parms.m_isPrePrepareBind) parms.BindParms();
                // 执行sql
                {
                    Srv().WriteLogTrace(("<P" + m_sExecSort + ".> Procedures ExecSQL Begin.").c_str(), __CURR_CODE_PLACE_C__);
                    CAutoRelease _auto([&](){ Srv().WriteLogTrace(("<P" + m_sExecSort + ".> Procedures ExecSQL End.").c_str(), __CURR_CODE_PLACE_C__); });
                    this->ExecuteProcedures();
                }
                // 从临时表中获取参数值
                this->MakeParmVals();
                // 设置Session
                parms.OutParmsToSession();
                // 返回输出参数
                int iRecCount = parms.OutParmsToJson();
                // 返回
                IKCJson& jsonRespond = CtrlD().JsonRespond();
                if (!jsonRespond.HasItem(c_RESTful_errMsg))
                {
                    // jsonRespond.SetStr(c_RESTful_errMsg, boost::lexical_cast<std::string>(iRecCount).c_str());
                    std::cout << iRecCount << std::endl;
                }
            }

            // 批量导入
            void BatchInsert(std::function<void(CSqlFunc::TFields&)> fFirstRowInnerDbSet = [](CSqlFunc::TFields&){})
            {
                // 批量输入的json参数（或内部数据集）名称
                std::string sBatchParm = CUtilFunc::PCharSafeToStr(CtrlD().JsonAttach().GetItem(c_RESTful_batchParm).GetStr(c_RESTful_batchValsName, ""));
                std::string sSrcDbSetName = !sBatchParm.empty() ? sBatchParm : CtrlD().ActionData().GetValsName();
                // 出错时的处理方式。 1：出错继续（默认）；2：出错停止；3：出错回滚（只针对带事务的SQL）
                int iBatchMethod = static_cast<int>(CtrlD().JsonAttach().GetItem(c_RESTful_batchParm).GetVal(c_RESTful_batchMethod, 1));
                // 是否设置保存点
                int iSavePoint = atoi(CUtilFunc::PCharSafeToPChar(CtrlD().JsonAttach().GetItem(c_RESTful_batchParm).GetStr("savePoint", "1000")));
                // 出现在日志里的提示行数。默认为0，即不提示。
                int iHintRow = static_cast<int>(CtrlD().JsonAttach().GetItem(c_RESTful_batchParm).GetVal("hintRow", 10000));
                // 结果变量
                unsigned affect = 0, iCount = 0;
                int errCode = 0;
                std::string errMsg = "", strOth = "\n", sDontInsert;
                bool bIgnore = false;
                // 循环变量
                std::atomic_int iLoopInsert = 0, iLoopRead = 0, iGetDbSetUseTime = 0, iGetNextRowUseTime = 0, iGetRowUseTime = 0, iUseTimeWait = 0;
                // 用时统计
                int iUseTimeTotal = 0, iBindParmUseTime = 0, iExecuteBatchUseTime = 0, iTranSavePointUseTime = 0, iUseTimeInsert = 0, iUseTimeInsertRow = 0;
                long long iNowTotal = CUtilFunc::GetCurrentStampMS();
                // 日志
                std::string sMsg = "<B" + m_sExecSort + ".> BatchInsert Begin -  [" + CtrlD().LocalFile() + " : " + CtrlD().NodeName() + "]";
                Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                std::cout << sMsg << std::endl;
                CAutoRelease _auto([&](){
                    iUseTimeTotal = static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowTotal);
                    std::string sMsg = (boost::format("<B%s.> BatchInsert End. Affect = %d, Count = %d, Loop = %d. TotalUseTime = %d, GetDbSetUseTime = %d, GetNextRowUseTime = %d, GetRowUseTime = %d, BindParmUseTime = %d, ExecuteBatchUseTime = %d, TranSavePointUseTime = %d, UseTimeInsert = %d, UseTimeInsertRow = %d, UseTimeWait = %d.")
                                        % m_sExecSort % affect % iCount % iLoopInsert % iUseTimeTotal % iGetDbSetUseTime % iGetNextRowUseTime % iGetRowUseTime % iBindParmUseTime % iExecuteBatchUseTime % iTranSavePointUseTime % iUseTimeInsert % iUseTimeInsertRow % iUseTimeWait).str();
                    Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                    std::cout << sMsg << std::endl;
                });
                // 每隔设定行数（如，一万条），提示一次
                auto fHintAtFixRows = [&](void)
                {
                    // 提示标头
                    if (0 == iLoopInsert && iHintRow > 0)
                    {
                        std::string sMsg = "<B" + m_sExecSort + ".> 0: 0 \t\t GetVal #= GetNext + GetRow. \t InsertRow #= BindParm + ExecuteBatch + TranSavePoint";
                        Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                        std::cout << sMsg << std::endl;
                    }
                    // 定期用时显示
                    if (99 == iLoopInsert || (iHintRow > 0 && iLoopInsert % iHintRow == 0))
                    {
                        std::string sMsg = (boost::format("<B%s.> %d / %d: %d \t\t %d #= %d + %d. \t %d #= %d + %d + %d")
                                            % m_sExecSort % (iLoopInsert + 1) % (iLoopRead + 1) % affect
                                            % iGetDbSetUseTime % iGetNextRowUseTime % iGetRowUseTime
                                            % iUseTimeInsertRow % iBindParmUseTime % iExecuteBatchUseTime % iTranSavePointUseTime).str();
                        Srv().WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                        std::cout << sMsg << std::endl;
                        // 提交一次事务
                        if (0 == errCode)
                        {
                            this->TranCommit();
                            std::this_thread::yield();
                            this->TranBegin();
                        }
                    }
                };
                // 得到记录文本
                std::function<std::string()> fGetValStr = [&]() -> std::string { return string(); };
                // 预先处理
                auto fBeforeExec = [&](void)
                {
                    // 检查错误
                    this->PreCheck();
                    // auto& act = CtrlD().ActionData();
                    // 解析参数
                    TParms& parms = this->GetParms();
                    parms.sSQLWorkType = c_RESTful_sql_DML;
                    const_cast<bool&>(parms.m_isBatchOp) = true;
                    parms.ParseSqlParms(this->GetSQL());
                    // 解析输出参数
                    parms.ParseOutParms();
                };
                // 绑定参数
                auto fBindJsonParms = [&](const IKCJson& jsonVal, const TRowData* rowData)
                {
                    long long iNow = CUtilFunc::GetCurrentStampMS();
                    CAutoRelease _auto([&](){ iBindParmUseTime += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNow); });
                    TParms& parms = this->GetParms();
                    // 行号、行数
                    int iRowID = iLoopInsert, iRowCount = iCount;
                    if (nullptr != rowData) iRowID = iRowCount = rowData->m_row;
                    // parms.ResetParms();
                    // 预执行前，绑定参数值
                    if (parms.m_isPrePrepareBind) parms.BindParms(jsonVal, rowData, iRowID, iRowCount);
                    // 第1次，先预执行（在第1次绑定参数后执行，可以获取参数的数据类型）
                    if (0 == iRowID)
                    {
                        this->PrepareSQL();
                        this->TranSavePoint("s0");
                    }
                    // 预执行后，绑定参数值
                    if (!parms.m_isPrePrepareBind) parms.BindParms(jsonVal, rowData, iRowID, iRowCount);
                };
                // 执行sql
                auto fExecBatch = [&]()
                {
                    std::pair<int, std::string> res;
                    {
                        long long iNow = CUtilFunc::GetCurrentStampMS();
                        CAutoRelease _auto([&](){ iExecuteBatchUseTime += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNow); });
                        // Srv().WriteLogTrace(("BatchInsert Begin: " + std::to_string(iLoopInsert)).c_str(), __CURR_CODE_PLACE_C__);
                        res = this->ExecuteBatch();
                        // Srv().WriteLogTrace(("BatchInsert End: " + std::to_string(iLoopInsert)).c_str(), __CURR_CODE_PLACE_C__);
                    }
                    if (res.second.empty())
                    {
                        affect += res.first;
                        if (0 == res.first)
                        {
                            std::string sParm = fGetValStr();
                            if (sDontInsert.size() < 4000)
                                sDontInsert += "[" + std::to_string(iLoopInsert + 1) + "] " + sParm.substr(0, 66) + "...\n";
                        }
                    }
                    else
                    {
                        if (0 == errCode) errCode = res.first;
                        throw std::runtime_error(res.second);
                    }
                    // 设置保存点
                    if (iSavePoint > 0 && iLoopInsert % iSavePoint == 0)
                    {
                        long long iNow = CUtilFunc::GetCurrentStampMS();
                        CAutoRelease _auto([&](){ iTranSavePointUseTime += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNow); });
                        this->TranSavePoint("s" + std::to_string(iLoopInsert));
                    }
                };
                // 异常处理
                auto fCatchDeal = [&](std::string sErr) -> bool
                {
                    if (0 == errCode) errCode = 207;
                    std::string sParm = fGetValStr();
                    if (strOth.size() < 4000)
                        strOth += "[" + std::to_string(iLoopInsert + 1) + "]" + sParm + " \n";
                    if (errMsg.size() < 4000)
                        errMsg += "[" + std::to_string(iLoopInsert + 1) + "]" + sErr + "  [" + sParm.substr(0, 66) + "...] \n";
                    else if (!bIgnore)
                    {
                        bIgnore = true;
                        errMsg += " \n...";
                    }
                    // 没插入的记录
                    auto fNotInsertMsg = [&](std::string sErr)
                    {
                        std::string sParm = fGetValStr();
                        std::cout << "BatchInsert Don't. " << sErr << std::endl << sParm << std::endl;
                        Srv().WriteLogTrace(("<B" + m_sExecSort + ".> BatchInsert Don't. " + sErr + "\n" + sParm + "\n").c_str(), __CURR_CODE_PLACE_C__);
                    };
                    // fNotInsertMsg(sErr);
                    this->TranRollbackToSavePoint();
                    if (1 != iBatchMethod) return false;
                    this->TranSavePoint("s" + std::to_string(iLoopInsert));
                    return true;
                };
                // 处理事务
                auto fTranDeal = [&]()
                {
                    const bool bRollback = 0 != errCode && 3 == iBatchMethod;
                    if (bRollback) this->TranRollback();
                    else this->TranCommit();
                };
                // 等待（并且挂起线程）
                auto fWaitAndYieldThrd = [&](void)
                {
                    const long long iNowIn = CUtilFunc::GetCurrentStampMS();
                    CAutoRelease _auto([&](){ iUseTimeWait += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
                    std::this_thread::yield();
                };

                // 开始批量插入（从json参数）
                auto fBatchFromJson = [&]()
                {
                    // json批量数组参数
                    const IKCJson& jsonVals = CtrlD().GetBatchArrayJson(sBatchParm.c_str());
                    jsonVals.GC();
                    // 插入数量
                    iCount = jsonVals.GetItemCount();
                    CAutoRelease _autoGC([&](){ jsonVals.GC(); });
                    // 本条数据
                    auto fGetJsonVal = [&]() -> const IKCJson&
                    {
                        long long iNow = CUtilFunc::GetCurrentStampMS();
                        CAutoRelease _auto([&](){ iGetDbSetUseTime += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNow); });
                        return jsonVals.GetItemNext();
                    };

                    // 预先处理
                    fBeforeExec();
                    // 循环插入
                    for (; iLoopInsert < iCount; iLoopRead = ++iLoopInsert)
                    {
                        CAutoRelease _auto(fHintAtFixRows);
                        try
                        {
                            // 统计用时
                            const long long iNowIn = CUtilFunc::GetCurrentStampMS();
                            CAutoRelease _auto([&](){ iUseTimeInsertRow += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
                            // 本条数据
                            const IKCJson& jsonVal = fGetJsonVal();
                            CAutoRelease _autoGC([&](){ jsonVals.GC(jsonVal.GetName()); });
                            if (!jsonVal.IsValid()) break;
                            fGetValStr = [&]() { return jsonVal.ToStr(); };
                            // 绑定参数
                            fBindJsonParms(jsonVal, nullptr);
                            // 执行sql
                            fExecBatch();
                        }
                        catch (std::exception& ex)
                        {
                            if (!fCatchDeal(ex.what())) break;
                        }
                    }
                };

                // 开始批量插入（从内部数据集）
                auto fBatchFromInner = [&](TRecordSetPtr dbSet)
                {
                    // 是否运行，有数据
                    std::atomic_bool bRunning = true;
                    // 行数据队列
                    int iQueSize = std::max(atoi(CtrlD().JsonAttach().GetItem(c_RESTful_batchParm).GetStr("queueSize", CtrlD().JsonAttach().GetItem("copyParm").GetStr("queueSize", "2048"))), 1024);
                    boost::lockfree::spsc_queue<TRowDataPtr> queRowData(iQueSize);
                    CAutoRelease _autoClean([&](){ queRowData.consume_all([](const TRowDataPtr&){}); });
                    // 执行插入的线程
                    auto fBatchInsertQueue = [&]()
                    {
                        for (int iReadSize = 0; bRunning || iReadSize > 0; iReadSize = queRowData.read_available())
                        {
                            try
                            {
                                if (iReadSize > 0)
                                {
                                    std::vector<TRowDataPtr> vctRowData(iReadSize);
                                    int iRealSize = queRowData.pop(vctRowData.data(), vctRowData.size());
                                    for (int i = 0; i < iRealSize; ++i, ++iLoopInsert)
                                    {
                                        CAutoRelease _autoHint(fHintAtFixRows);
                                        try
                                        {
                                            // 统计用时
                                            const long long iNowIn = CUtilFunc::GetCurrentStampMS();
                                            CAutoRelease _auto([&](){ iUseTimeInsertRow += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
                                            // 绑定参数
                                            fBindJsonParms(CtrlD().ActionData().InvalidJson(), vctRowData[i].get());
                                            // 执行sql
                                            fExecBatch();
                                        }
                                        catch (TExcept &ex)
                                        {
                                            auto [iCode, sMsg, sPos] = ParmExceptInfo(ex);
                                            if (!fCatchDeal(sMsg))
                                            {
                                                bRunning = false;
                                                return;
                                            }
                                        }
                                        catch (std::exception& ex)
                                        {
                                            if (!fCatchDeal(ex.what()))
                                            {
                                                bRunning = false;
                                                return;
                                            }
                                        }
                                        catch (...)
                                        {
                                            if (!fCatchDeal("Unknown Error"))
                                            {
                                                bRunning = false;
                                                return;
                                            }
                                        }
                                    }
                                }
                                else fWaitAndYieldThrd();
                            }
                            catch (...) { fWaitAndYieldThrd(); }
                        }
                    };
                    std::thread thrdBatchInsertQue(fBatchInsertQueue);
                    // 请求的字段
                    CSqlFunc::TFields vecFields;
                    // 数据集输出到队列
                    {
                        // 用时统计
                        long long iNow = CUtilFunc::GetCurrentStampMS();
                        CAutoRelease _auto([&](){ iGetDbSetUseTime += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNow); });
                        // 退出时，标记插入队列结束（已无数据）
                        CAutoRelease __insertEnd([&](){ bRunning = false; });
                        // 开始插入队列
                        iCount = dbSet->DataSetToOut(vecFields,
                            [&](){
                                // 首行。表结构
                                fFirstRowInnerDbSet(vecFields);
                                // 预先处理
                                fBeforeExec();
                            }, [&](int r, int useTimeNext) -> bool {
                                iLoopRead = r;
                                iGetNextRowUseTime += useTimeNext;
                                // 用时统计
                                long long iNow = CUtilFunc::GetCurrentStampMS();
                                CAutoRelease _auto([&](){ iGetRowUseTime += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNow); });
                                // 行数据
                                TRowDataPtr pRowData(new TRowData(r));
                                // 添加行记录
                                for (auto &fFeild : vecFields)
                                {
                                    CSqlFunc::TKcValVariant val = dbSet->GetVal(*fFeild);
                                    TFieldValPtr fieldVal(new TFieldVal(fFeild, val));
                                    pRowData->m_valMap.insert(std::make_pair(fFeild->m_name, fieldVal));
                                }
                                // 插入队列
                                while (queRowData.write_available() == 0 || !queRowData.push(pRowData)) fWaitAndYieldThrd();
                                return bRunning;
                            });
                    }
                    // 等待批量插入结束
                    if (thrdBatchInsertQue.joinable()) thrdBatchInsertQue.join();
                };

                // *** 开始批量插入 ***
                {
                    // 事务
                    this->TranBegin();
                    CAutoRelease _auto(fTranDeal);

                    // todo: 从内部数据集里取数据
                    ICtrlApiDataWithDbSet* pCtrl = dynamic_cast<CSqlFunc::ICtrlApiDataWithDbSet*>(&CtrlD());
                    TRecordSetPtr lastInnerDbSet;
                    if (nullptr != pCtrl)
                        lastInnerDbSet = pCtrl->DataSetInnerLast(sSrcDbSetName);
                    if (lastInnerDbSet.get() != nullptr)
                        fBatchFromInner(lastInnerDbSet);
                    // 从json批量数组参数里取数据
                    else fBatchFromJson();
                }

                // 结束批量插入
                if (!sDontInsert.empty())
                    Srv().WriteLogTrace(("BatchInsert, Not Insert Items:\n" + sDontInsert).c_str(), __CURR_CODE_PLACE_C__);
                try
                {
                    this->BatchInsertEnd(0 == errCode || 3 != iBatchMethod);
                }
                catch (TExcept &ex)
                {
                    auto [iCode, sMsg, sPos] = ParmExceptInfo(ex);
                    throw TKCSqlException(iCode, sPos.empty() ? __CURR_CODE_PLACE_C__ : sPos, sMsg + "\n" + errMsg, strOth);
                }
                if (0 != errCode) throw TKCSqlException(errCode, __CURR_CODE_PLACE_C__, errMsg, strOth);
                CtrlD().SetJsonRespond(0, (boost::format("%d / %d") % iCount % affect).str().c_str());
            }

        private:
            // 执行查询，查询结果的数据集到json。返回主数据集行数和执行影响行数。
            std::tuple<int, int> SelectQuery(TParms& parms)
            {
                // 预执行前，绑定参数值
                if (parms.m_isPrePrepareBind) parms.BindParms();
                // 预执行
                this->PrepareSQL();
                // 预执行后，绑定参数值
                if (!parms.m_isPrePrepareBind) parms.BindParms();
                // 执行查询
                auto fExecQuery = [&](int &rows_affected)
                {
                    Srv().WriteLogTrace(("<Q" + m_sExecSort + ".> Query ExecSQL Begin.").c_str(), __CURR_CODE_PLACE_C__);
                    CAutoRelease _auto([&](){ Srv().WriteLogTrace(("<Q" + m_sExecSort + ".> Query ExecSQL End.").c_str(), __CURR_CODE_PLACE_C__); });
                    return this->ExecuteQuery(rows_affected);
                };
                // 数据集参数
                std::vector<std::pair<std::string, std::string>> vctRsetNames;
                const IKCJson& jsonDbSetPrm = CtrlD().JsonAttach().GetItem(c_RESTful_dbsetParms);
                if (jsonDbSetPrm.IsValid())
                    for (int i = 0, c = jsonDbSetPrm.GetItemCount(); i < c; ++i)
                    {
                        const auto &jsonDbSet = jsonDbSetPrm.GetItem(i);
                        std::string sName = CUtilFunc::PCharSafeToStr(jsonDbSet.GetName());
                        std::string sType = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(jsonDbSet.GetStr()));
                        vctRsetNames.push_back(std::make_pair(sName, sType));
                    }
                // 默认数据集名称
                const std::string sDefaultVals = CtrlD().ActionData().GetValsName();
                // 结果
                IKCJson& jsonRespond = CtrlD().JsonRespond();
                int iCountSet = 0, iCountRec = 0;
                if (jsonRespond.GetType() == IKCJson::edtArray) jsonRespond.ResetByStr("{}");
                int rows_affected = -1;
                bool bHasMainSet = false;   // 是否存在主数据集
                for (TRecordSetPtr SetPtr = fExecQuery(rows_affected); SetPtr.get() != nullptr; SetPtr = this->NextResult(), ++iCountSet)
                {
                    // 数据集类型
                    const std::string sDbSetType = vctRsetNames.size() > iCountSet && !vctRsetNames[iCountSet].second.empty() ? vctRsetNames[iCountSet].second : c_RESTful_dbset;
                    // 数据集名称（带路径，用实心点分隔）
                    std::string sDbSetName = SetPtr->m_name = vctRsetNames.size() > iCountSet ? boost::algorithm::trim_copy(vctRsetNames[iCountSet].first) : "";
                    // 是否主数据集
                    bool bMainSet = !bHasMainSet && (c_RESTful_dbset == sDbSetType || c_RESTful_dbset_array == sDbSetType || c_RESTful_dbset_inner == sDbSetType) && (sDbSetName.empty() || sDefaultVals == sDbSetName);
                    bHasMainSet = bHasMainSet || bMainSet;
                    // 设置数据集名称
                    if (sDbSetName.empty())
                        SetPtr->m_name = bMainSet ? sDefaultVals : c_RESTful_rset + std::to_string(iCountSet);
                    else if (sDbSetName.substr(0, sizeof(c_RESTful_rset)) == c_RESTful_rset)
                        sDbSetName = SetPtr->m_name = c_RESTful_rset + std::to_string(iCountSet);
                    // 内部数据集
                    if (c_RESTful_dbset_inner == sDbSetType)
                        SetPtr->DataSetSaveToInner();
                    // 返回主数据集
                    else if (bMainSet)
                    {
                        if (c_RESTful_dbset == sDbSetType)
                            iCountRec = SetPtr->DataSetToJson(jsonRespond);
                        else if (c_RESTful_dbset_array == sDbSetType)
                        {
                            jsonRespond.DelItem(sDefaultVals.c_str());
                            iCountRec = SetPtr->DataSetToJsonArray(jsonRespond.AddArray(sDefaultVals.c_str()));
                        }
                    }
                    // 返回其他数据集
                    else
                    {
                        // 路径
                        auto iLeafPos = sDbSetName.rfind('.');
                        std::string sRsetPath, sRsetName = sDbSetName;
                        if (std::string::npos != iLeafPos)
                        {
                            sRsetPath = sDbSetName.substr(0, iLeafPos);
                            sRsetName = sDbSetName.substr(iLeafPos + 1);
                        }
                        IKCJson& jsonOtherRset = sRsetPath.empty() ? jsonRespond : CCtrlCommon::AddSubItem(jsonRespond, sRsetPath);
                        if (c_RESTful_dbset == sDbSetType)
                            iCountRec = SetPtr->DataSetToJson(jsonOtherRset.AddItem(SetPtr->m_name.c_str()), false);
                        else if (c_RESTful_dbset_one == sDbSetType)
                            iCountRec = SetPtr->DataSetToJsonOne(sRsetName.empty() ? jsonOtherRset : jsonOtherRset.AddItem(sRsetName.c_str()));
                        else if (c_RESTful_dbset_array == sDbSetType)
                            iCountRec = SetPtr->DataSetToJsonArray(jsonOtherRset.AddArray(SetPtr->m_name.c_str()));
                    }
                }
                return std::make_tuple(iCountRec, rows_affected);
            }
        };

        // 请求的字段
        static void RequestFeilds(IActionData& act, TFields& vecFields)
        {
            try
            {
                const IKCJson& json = act.JsonRequest().GetItem(c_RESTful_feilds);
                if (json.IsValid())
                    for (int i = 0, c = json.GetItemCount(); i < c; ++i)
                        vecFields.push_back(std::shared_ptr<TField>(new TField(boost::algorithm::to_lower_copy(std::string(json.GetItem(i).GetStr())))));
            }
            catch (...) {}
        }
    };
}
