#include "ctrl_sqlite.h"

////////////////////////////////////////////////////////////////////////////////
// libpq异常类
CCtrlSqlite::TSqliteException::TSqliteException(int c, string e, string s, string p)
    : std::runtime_error(s.c_str()), m_code(c), m_err(e), m_msg(s), m_pos(p)
{
}

// CCtrlSqlite::TParmOra类
CCtrlSqlite::TParmSqlite::TParmSqlite(TParmGrpSqlite& own, std::string s, unsigned p) : m_own(own)
{
    iPos = p;
    sName = s;
}

// 设置参数值
bool CCtrlSqlite::TParmSqlite::SetNumber(double fVal)
{
    // this->bIsNull = false;
    // this->sVal = std::to_string(fVal);
    return false;
}

void CCtrlSqlite::TParmSqlite::SetInt(int iVal)
{
    // this->bIsNull = false;
    // this->sVal = std::to_string(iVal);
}

void CCtrlSqlite::TParmSqlite::SetString(string strVal)
{
    // this->bIsNull = false;
    // this->sVal = strVal;
}
void CCtrlSqlite::TParmSqlite::SetClob(string strVal)
{
    this->bIsNull = false;
    this->SetString(strVal);
}

void CCtrlSqlite::TParmSqlite::SetNull(void)
{
    // this->bIsNull = true;
    // this->sVal.clear();
}

// 获取参数值
string CCtrlSqlite::TParmSqlite::GetVal(string sDef)
{
    // if (CSqlFunc::EParmType::eptSessionParm == this->ePTp)
    // {
    //     auto it = m_own.mapParms.find(this->sName);
    //     if (m_own.mapParms.end() != it) return it->second->sVal;
    //     else return sDef;
    // }
    // return this->sVal;

    if (CSqlFunc::EParmType::eptSessionParm == this->ePTp)
    {
        auto it = m_own.mapParms.find(this->sName);
        if (m_own.mapParms.end() != it)
            return BoostAnyToStdString(it->second->anyVal, sDef);
        else return sDef;
    }
    return BoostAnyToStdString(this->anyVal, sDef);
}
string CCtrlSqlite::TParmSqlite::GetString(string sDef)
{
    return this->GetVal(sDef);
}
string CCtrlSqlite::TParmSqlite::GetClob(string sDef)
{
    return GetString(sDef);
}

string CCtrlSqlite::TParmSqlite::GetDate(string /*fmt*/)
{
    string val = this->GetVal();
    // todo: 格式化日期类型
    /*
    date d;
    std::string teststring = "2021-06-26";
    boost::gregorian::date_input_facet *fmt = new boost::gregorian::date_input_facet("%Y-%m-%d");
    std::stringstream ss1(teststring);
    ss1.imbue(locale(ss1.getloc(), fmt));
    ss1 >> d;
    */
    //CUtilFunc::TimeToStr()
    return val;
}

string CCtrlSqlite::TParmSqlite::GetDateTime(string /*fmt*/)
{
    string val = this->GetVal();
    // todo: 格式化日期时间类型
    /*
    ptime tp = microsec_clock::local_time();
    time_facet* tfacet = new time_facet("%Y年%m月%d日%H点%M分%S%F秒");
    cout.imbue(locale(cout.getloc(), tfacet));
    cout << tp << endl;
    */
    return val;
}

int CCtrlSqlite::TParmSqlite::GetInt(int iDef)
{
    // string val = this->GetVal(std::to_string(iDef));
    // return atoi(val.c_str());

    if (CSqlFunc::EParmType::eptSessionParm == this->ePTp)
    {
        auto it = m_own.mapParms.find(this->sName);
        if (m_own.mapParms.end() != it)
            return static_cast<int>(CastBoostAnyToInt(it->second->anyVal, iDef));
        else return iDef;
    }
    return static_cast<int>(CastBoostAnyToInt(this->anyVal, iDef));
}

double CCtrlSqlite::TParmSqlite::GetNumber(double fDef)
{
    // string val = this->GetVal(std::to_string(fDef));
    // return atof(val.c_str());

    if (CSqlFunc::EParmType::eptSessionParm == this->ePTp)
    {
        auto it = m_own.mapParms.find(this->sName);
        if (m_own.mapParms.end() != it)
            return CastBoostAnyToFloat(it->second->anyVal, fDef);
        else return fDef;
    }
    return CastBoostAnyToFloat(this->anyVal, fDef);
}

// 获取游标数据集
CSqlFunc::TRecordSetPtr CCtrlSqlite::TParmSqlite::GetDbSet(void)
{
    return CSqlFunc::TRecordSetPtr();
}

void CCtrlSqlite::TParmSqlite::SetDbTp(void)
{
    switch (eDTp)
    {
    case CSqlFunc::EDataType::edtInt:
        sDbTP = "int";
        break;
    case CSqlFunc::EDataType::edtNumber:
        sDbTP = "numeric";
        break;
    case CSqlFunc::EDataType::edtDatetime:
        sDbTP = "timestamp";
        break;
    case CSqlFunc::EDataType::edtJson:
        sDbTP = "json";
        break;
    case CSqlFunc::EDataType::edtClob:
        sDbTP = "text";
        break;
    case CSqlFunc::EDataType::edtUnknown:
        // 未指定类型，通过值类型
        if (anyVal.type() == typeid(int) || anyVal.type() == typeid(long long)) sDbTP = "int";
        else if (anyVal.type() == typeid(double)) sDbTP = "numeric";
        else sDbTP = "varchar";
        break;
    case CSqlFunc::EDataType::edtString:
    default:
        sDbTP = "varchar";
        break;
    }
}

void CCtrlSqlite::TParmSqlite::RegOutParam(void)
{
}

// CCtrlSqlite::TParmGrpOra类
CCtrlSqlite::TParmGrpSqlite::TParmGrpSqlite(CCtrlSqlite& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, sqlite3 &con)
    : TParmGrpSqliteBase(false), m_own(own), m_objCtrlD(ctrlD), m_kcFile(sKcFile), m_actName(sActName), m_con(con)
{
}

// 所属的服务
IKCSql& CCtrlSqlite::TParmGrpSqlite::Srv(void)
{
    return m_own.m_own;
}

//  控制器 信息
string CCtrlSqlite::TParmGrpSqlite::ActInfo(void)
{
    return m_kcFile + "?act=" + m_actName;
}

// 创建参数
CSqlFunc::TParmPtr CCtrlSqlite::TParmGrpSqlite::MakePram(std::string s, unsigned p)
{
    return CSqlFunc::TParmPtr(new TParmSqlite(*this, s, p));
}

// 异常信息
string CCtrlSqlite::TParmGrpSqlite::ParmExceptInfo(TSqliteException& ex)
{
    return (format("%d: %s") % ex.m_code % ex.m_msg).str();
}

// CCtrlSqlite::TRecordSetSqlite 类
CCtrlSqlite::TRecordSetSqlite::TRecordSetSqlite(TDBCommandSqlite& own, ICtrlApiData& ctrlD, KeepStmtSqlitePtr st)
    : m_own(own), m_objCtrlD(ctrlD), m_keepDbSet(st), m_dbset(st->m_stmt)
    , m_dataCount(sqlite3_data_count(&m_dbset)), m_fieldCount(sqlite3_column_count(&m_dbset))
{
}
CCtrlSqlite::TRecordSetSqlite::~TRecordSetSqlite(void)
{
}

// 获取字段信息
void CCtrlSqlite::TRecordSetSqlite::GetColumnListMetaData(CSqlFunc::TFields &vecFeilds)
{
    map<string, std::shared_ptr<CSqlFunc::TField>> mapFeilds;
    for (int i = 0; i < m_fieldCount; ++i)
    {
        // 列属性值
        int iColType = sqlite3_column_type(&m_dbset, i);
        string sColName = sqlite3_column_name(&m_dbset, i);
        auto sTypeName = CCtrlSqlite::GetTypeName(iColType);
        int iSize = sqlite3_column_bytes(&m_dbset, i);
        sColName = algorithm::to_lower_copy(sColName);
        mapFeilds.insert(make_pair(sColName, std::shared_ptr<CSqlFunc::TField>(
                                                 new CSqlFunc::TField(sColName, i + 1, iColType, sTypeName, iSize))));
    }
    if (vecFeilds.empty())
        for (int i = 0; i < m_fieldCount; ++i)
        {
            string sFieldName = sqlite3_column_name(&m_dbset, i);
            sFieldName = algorithm::to_lower_copy(sFieldName);
            auto it = mapFeilds.find(sFieldName);
            if (mapFeilds.end() != it) vecFeilds.push_back(it->second);
        }
    else
        for (auto &fd : vecFeilds)
        {
            auto it = mapFeilds.find(fd->m_name);
            if (mapFeilds.end() != it) fd = it->second;
        }
}
// 下一条
bool CCtrlSqlite::TRecordSetSqlite::Next(void)
{
    int iErrCode = SQLITE_DONE;
    bool bResult = (iErrCode = sqlite3_step(&m_dbset)) == SQLITE_ROW;
    if (iErrCode != SQLITE_DONE && iErrCode != SQLITE_ROW)
    {
        string sErrMsg = algorithm::trim_copy(CUtilFunc::PCharSafeToStr(sqlite3_errmsg(&m_own.m_con)));
        throw TSqliteException(iErrCode, sErrMsg, sqlite3_expanded_sql(&m_dbset), __CURR_CODE_PLACE_C__);
    }
    return bResult;
}
// 添加值
void CCtrlSqlite::TRecordSetSqlite::AddVal(IKCJson& jsonRow, CSqlFunc::TField& fld)
{
    if (fld.m_pos > 0 && sqlite3_column_type(&m_dbset, fld.m_pos - 1) != SQLITE_NULL)
    {
        switch (fld.m_type)
        {
        case SQLITE_FLOAT:
            jsonRow.AddVal(fld.m_name.c_str(), sqlite3_column_double(&m_dbset, fld.m_pos - 1));
            break;
        case SQLITE_INTEGER:
            jsonRow.AddVal(fld.m_name.c_str(), sqlite3_column_int64(&m_dbset, fld.m_pos - 1));
            break;
        case SQLITE_BLOB:
        case SQLITE_TEXT:
        default:
            jsonRow.AddStr(fld.m_name.c_str(), reinterpret_cast<const char*>(sqlite3_column_text(&m_dbset, fld.m_pos - 1)));
            break;
        }
    }
    else jsonRow.AddNull(fld.m_name.c_str());
}
// 得到值
CSqlFunc::TKcValVariant CCtrlSqlite::TRecordSetSqlite::GetVal(CSqlFunc::TField& fld) const
{
    CSqlFunc::TKcValVariant result;
    if (fld.m_pos > 0 && sqlite3_column_type(&m_dbset, fld.m_pos - 1) != SQLITE_NULL)
    {
        switch (fld.m_type)
        {
        case SQLITE_FLOAT:
            result = sqlite3_column_double(&m_dbset, fld.m_pos - 1);
            break;
        case SQLITE_INTEGER:
            result = sqlite3_column_int64(&m_dbset, fld.m_pos - 1);
            break;
        case SQLITE_BLOB:
        case SQLITE_TEXT:
        default:
            result = CUtilFunc::PCharSafeToStr(reinterpret_cast<const char*>(sqlite3_column_text(&m_dbset, fld.m_pos - 1)));
            break;
        }
    }
    return result;
}

// CCtrlSqlite::TDBCommandSqlite 类
CCtrlSqlite::TDBCommandSqlite::TDBCommandSqlite(CCtrlSqlite& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, sqlite3 &con, string sSQL, string sMethod)
    : m_own(own), m_objCtrlD(ctrlD), m_kcFile(sKcFile), m_actName(sActName), m_stmtName("__" + sActName + std::to_string(m_execSort))
    , m_method(sMethod), m_con(con), m_sql(sSQL), m_parms(own, m_objCtrlD, sKcFile, sActName, con)
{
}
CCtrlSqlite::TDBCommandSqlite::~TDBCommandSqlite(void)
{
}

// 所属的服务
IKCSql& CCtrlSqlite::TDBCommandSqlite::Srv(void)
{
    return m_own.m_own;
}

// 控制器信息
string CCtrlSqlite::TDBCommandSqlite::ActInfo(void)
{
    return m_kcFile + "?act=" + m_actName;
}

// 事务
void CCtrlSqlite::TDBCommandSqlite::TranBegin(void)
{
    sqlite3_exec(&m_con, "BEGIN", nullptr, nullptr, nullptr);
}
void CCtrlSqlite::TDBCommandSqlite::TranCommit(void)
{
    sqlite3_exec(&m_con, "COMMIT", nullptr, nullptr, nullptr);
}
void CCtrlSqlite::TDBCommandSqlite::TranRollback(void)
{
    sqlite3_exec(&m_con, "ROLLBACK", nullptr, nullptr, nullptr);
}

// 预备sql语句
void CCtrlSqlite::TDBCommandSqlite::PrepareSQL(void)
{
    // SQL语句不能为空
    if (m_sql.empty())
        throw TKCSqliteException(ecd_ErrCode_KCSqlSqlite + 5, __CURR_CODE_PLACE_C__, string(m_own.m_own.getHint("SQL_Empty_")) + m_objCtrlD.ActionData().GetSingleInfo("the_request") + " - " + m_actName, m_own.m_own);
    // DDL语句（创建表等）
    if (c_RESTful_sql_DDL == this->GetParms().sSQLWorkType)
        PrepareByDDLSQL();
    // DML、DQL（增、删、改、查等）语句
    else if (c_RESTful_sql_DML == this->GetParms().sSQLWorkType || c_RESTful_sql_DQL == this->GetParms().sSQLWorkType)
        PrepareByOneSQL();
    cout << "[PrepareSQL] " << m_sql << endl;
}
// 预执行DDL语句
void CCtrlSqlite::TDBCommandSqlite::PrepareByDDLSQL(void)
{
}
// 预执行单语句
void CCtrlSqlite::TDBCommandSqlite::PrepareByOneSQL(void)
{
    string sLogInfo = "<R" + m_sExecSort + ".> Prepare By A SQL" + c_strLineShow + m_sql + c_strLineShow;
    // 预执行
    const char *pNearBy = nullptr;
    const auto iErrCode = sqlite3_prepare_v2(&m_con, m_sql.c_str(), static_cast<int>(m_sql.size()), &m_stmt, &pNearBy);
    if (nullptr != m_stmt) m_keepStmt.reset(new KeepStmtSqlite(*m_stmt));
    TSqliteException::CheckError<SQLITE_OK>(m_con, iErrCode, m_sql + "\r\n" + CUtilFunc::PCharSafeToStr(pNearBy), __CURR_CODE_PLACE_C__);
    // 日志
    m_own.m_own.WriteLogTrace(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
}

// 批量操作
pair<int, string> CCtrlSqlite::TDBCommandSqlite::ExecuteBatch(void)
{
    try
    {
        // m_own.m_own.WriteLogTrace("Insert Begin: ", __CURR_CODE_PLACE_C__);
        // CAutoRelease _auto([&](){ m_own.m_own.WriteLogTrace("Insert End: ", __CURR_CODE_PLACE_C__); });
        return make_pair(ExecuteUpdate(false), "");
    }
    catch (TSqliteException& ex)
    {
        return make_pair(ex.m_code, ex.m_msg);
    }
}

// 执行
unsigned int CCtrlSqlite::TDBCommandSqlite::ExecuteUpdate(void)
{
    return ExecuteUpdate(true);
}
unsigned int CCtrlSqlite::TDBCommandSqlite::ExecuteUpdate(bool outLog)
{
    string sLogInfo = "<U" + m_sExecSort + ".>";
    int res = UpdateOrQuery(sLogInfo);
    // TSqliteException::CheckError<SQLITE_OK>(m_con, res, sLogInfo, __CURR_CODE_PLACE_C__);
    if (outLog) m_own.m_own.WriteLogTrace(sLogInfo.c_str(), __CURR_CODE_PLACE_C__, sqlite3_errstr(res));
    // return sqlite3_total_changes(&m_con);
    return sqlite3_changes(&m_con);
}

// 查询
CSqlFunc::TRecordSetPtr CCtrlSqlite::TDBCommandSqlite::ExecuteQuery(int &rows_affected)
{
    string sLogInfo = "<S" + m_sExecSort + ".>";
    int res = UpdateOrQuery(sLogInfo);
    CSqlFunc::TRecordSetPtr resPtr(new TRecordSetSqlite(*this, m_objCtrlD, m_keepStmt));
    TSqliteException::CheckError<SQLITE_OK>(m_con, res, sLogInfo, __CURR_CODE_PLACE_C__);
    //m_own.m_own.WriteLogDebug(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
    rows_affected = -1;
    return resPtr;
}

// 执行过程
int CCtrlSqlite::TDBCommandSqlite::ExecuteProcedures(void)
{
    return ExecuteProcedures(true);
}
int CCtrlSqlite::TDBCommandSqlite::ExecuteProcedures([[maybe_unused]] bool outLog)
{
    // return static_cast<int>(ExecuteUpdate());
    throw TKCSqliteException(ecd_ErrCode_KCSqlSqlite + 5, __CURR_CODE_PLACE_C__, string(m_own.m_own.getHint("Method_Not_Allowed")) + " - " + c_RESTful_procedures + "\n" + m_objCtrlD.ActionData().GetSingleInfo("the_request") + " - " + m_actName, m_own.m_own);
}

// 绑定参数
string CCtrlSqlite::TDBCommandSqlite::bindParm(void)
{
    if (nullptr == m_stmt) return "";
    string sParmInfo = "argv:\n";
    sqlite3_clear_bindings(m_stmt);
    sqlite3_reset(m_stmt);
    for (auto &parm : m_parms.mapParms)
    {
        int idxPrm = parm.first.empty() ? parm.second->iPos : sqlite3_bind_parameter_index(m_stmt, (":" + parm.first).c_str());
        string strVal;
        // 参数的持久字符串值，绑定为sqlite文本参数
        auto fBindPrmStr = [&](std::function<void(const char* zData, int nData)> fBindText)
        {
            string *pVal = boost::any_cast<string>(&parm.second->anyVal);
            if (nullptr == pVal)
            {
                auto &prmSqlt = parm.second->Get<TParmSqlite>();
                prmSqlt.sOther = strVal = BoostAnyToStdString(prmSqlt.anyVal);
                fBindText(prmSqlt.sOther.c_str(), prmSqlt.sOther.size());
            }
            else
            {
                strVal = *pVal;
                fBindText(pVal->c_str(), pVal->size());
            }
        };
        if (parm.second->IsNull() || parm.second->anyVal.empty())
            sqlite3_bind_null(m_stmt, idxPrm);
        else
        {
            auto eDTp = parm.second->eDTp;
            // 未指定类型，通过值类型
            if (CSqlFunc::EDataType::edtUnknown == eDTp)
            {
                if (parm.second->anyVal.type() == typeid(int) || parm.second->anyVal.type() == typeid(long long))
                    eDTp = CSqlFunc::EDataType::edtInt;
                else if (parm.second->anyVal.type() == typeid(double))
                    eDTp = CSqlFunc::EDataType::edtNumber;
                else eDTp = CSqlFunc::EDataType::edtString;
            }
            // 绑定参数
            switch(eDTp)
            {
            case CSqlFunc::EDataType::edtInt:
                // sqlite3_bind_int64(m_stmt, idxPrm, atoll(parm.second->sVal.c_str()));
                sqlite3_bind_int64(m_stmt, idxPrm, CastBoostAnyToInt(parm.second->anyVal));
                strVal = BoostAnyToStdString(parm.second->anyVal);
                break;
            case CSqlFunc::EDataType::edtNumber:
                // sqlite3_bind_double(m_stmt, idxPrm, atof(parm.second->sVal.c_str()));
                sqlite3_bind_double(m_stmt, idxPrm, CastBoostAnyToFloat(parm.second->anyVal));
                strVal = BoostAnyToStdString(parm.second->anyVal);
                break;
            case CSqlFunc::EDataType::edtClob:
                // sqlite3_bind_blob(m_stmt, idxPrm, parm.second->sVal.c_str(), parm.second->sVal.size(), nullptr);
                fBindPrmStr([&](const char* zData, int nData){
                    sqlite3_bind_blob(m_stmt, idxPrm, zData, nData, nullptr);
                });
                break;
            case CSqlFunc::EDataType::edtString:
            default:
                // sqlite3_bind_text(m_stmt, idxPrm, parm.second->sVal.c_str(), parm.second->sVal.size(), nullptr);
                fBindPrmStr([&](const char* zData, int nData){
                    sqlite3_bind_text(m_stmt, idxPrm, zData, nData, nullptr);
                });
            }
        }
        sParmInfo += (format("$%d = '%s'\n") % parm.second->iPos % strVal).str();
    }
    return sParmInfo;
}

// 执行或查询
int CCtrlSqlite::TDBCommandSqlite::UpdateOrQuery(string& sLogInfo)
{
    int res = SQLITE_OK;
    // DDL语句（创建表等）
    if (c_RESTful_sql_DDL == this->GetParms().sSQLWorkType)
    {
        char *pzErrMsg = nullptr;
        res = sqlite3_exec(&m_con, m_sql.c_str(), nullptr, nullptr, &pzErrMsg);
        TSqliteException::CheckError<SQLITE_OK>(m_con, res, sLogInfo + "\r\n" + CUtilFunc::PCharSafeToStr(pzErrMsg), __CURR_CODE_PLACE_C__);
    }
    // DQL语句（查询）
    else if (c_RESTful_sql_DQL == this->GetParms().sSQLWorkType)
    {
        // 参数
        string sParmInfo = bindParm();
        sLogInfo += c_strLineShow + sParmInfo + c_strLineShow;
    }
    // DML语句（增、删、改）
    else
    {
        // 参数
        string sParmInfo = bindParm();
        sLogInfo += c_strLineShow + sParmInfo + c_strLineShow;
        // 执行
        res = sqlite3_step(m_stmt);
        TSqliteException::CheckError<SQLITE_DONE>(m_con, res, sLogInfo, __CURR_CODE_PLACE_C__);
    }
    return res;
}

// 仿postgresql的copy
void CCtrlSqlite::TDBCommandSqlite::CopyBatch(void)
{
    // 附加参数
    string sCopyFeildsParm = CUtilFunc::PCharSafeToPChar(m_objCtrlD.JsonAttach().GetItem("copyParm").GetStr("copyFeildsParm", ""));
    string sCopyValsParm = CUtilFunc::PCharSafeToPChar(m_objCtrlD.JsonAttach().GetItem("copyParm").GetStr("copyValsParm", ""));
    string sToTable = CUtilFunc::PCharSafeToPChar(m_objCtrlD.JsonAttach().GetItem("copyParm").GetStr("toTable", ""));
    auto &jsnPKey = m_objCtrlD.JsonAttach().GetItem("copyParm").GetItem("primaryKey");
    // 源数据集名称
    string sSrcDbSetName = sCopyValsParm.empty() ? m_objCtrlD.ActionData().GetValsName() : sCopyValsParm;
    // 目标数据集名称（插入的表或临时表名称）
    string sTableName = sToTable.empty() ? sSrcDbSetName : CUtilFunc::PCharSafeToStr(m_objCtrlD.GetParmVal(sToTable.c_str(), sToTable.c_str()));
    // 主键
    vector<string> vctPKey;
    if (!jsnPKey.IsNull())
    {
        for (int i = 0, c = jsnPKey.GetItemCount(); i < c; ++i)
        {
            string sKeyName = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(jsnPKey.GetItem(i).GetStr()));
            if (!sKeyName.empty()) vctPKey.push_back(sKeyName);
        }
    }

    // 字段和参数列表
    string sFields, sArgs;

    // 创建表
    auto fCtreateTable = [&](std::function<void(string&)> fDynMakeFeilds)
    {
        // 创建表的sql语句
        string sCreateTabSQL = (boost::format("CREATE %s TABLE IF NOT EXISTS %s (") % (sToTable.empty() ? "temp" : "") % sTableName).str();
        // 指定表结构
        if (!sCopyFeildsParm.empty())
        {
            const IKCJson& jsonFeilds = m_objCtrlD.GetParmJson(sCopyFeildsParm.c_str());
            if (!jsonFeilds.IsValid())
                throw TKCSqliteException(ecd_ErrCode_KCSqlSqlite + 14, __CURR_CODE_PLACE_C__, string(m_own.m_own.getHint("Request_Failed_")) + m_own.m_own.getHint("Don_t_exists_field_") + sCopyFeildsParm, m_own.m_own);
            jsonFeilds.InitItemNextStep();
            for (unsigned i = 0, c = jsonFeilds.GetItemCount(); i < c; ++i)
            {
                auto &jsnFeild = jsonFeilds.GetItemNext();
                string sFeildName = jsnFeild.GetName();
                string sFeildType = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(jsnFeild.GetStr()));
                sCreateTabSQL += "\n" + sFeildName + " " + sFeildType + ",";
                sFields += " " + sFeildName + ",";
                sArgs += " :" + sFeildName + ",";
            }
        }
        // 未指定表结构
        else
            fDynMakeFeilds(sCreateTabSQL);
        // 添加主键
        if (!vctPKey.empty())
        {
            sCreateTabSQL += "\nPRIMARY KEY(";
            for (string &sKey : vctPKey) sCreateTabSQL += sKey + ",";
            boost::algorithm::trim_right_if(sCreateTabSQL, boost::is_any_of(","));
            sCreateTabSQL += ")";
        }
        boost::algorithm::trim_right_if(sCreateTabSQL, boost::is_any_of(","));
        boost::algorithm::trim_right_if(sFields, boost::is_any_of(","));
        boost::algorithm::trim_right_if(sArgs, boost::is_any_of(","));
        sCreateTabSQL += "\n)";
        // 执行创建表的sql
        char *pzErrMsg = nullptr;
        int res = sqlite3_exec(&m_con, sCreateTabSQL.c_str(), nullptr, nullptr, &pzErrMsg);
        TSqliteException::CheckError<SQLITE_OK>(m_con, res, CUtilFunc::PCharSafeToStr(pzErrMsg) + "\n" + sCreateTabSQL, __CURR_CODE_PLACE_C__);
        // 日志
        m_own.m_own.WriteLogTrace(("<C" + m_sExecSort + ".> 创建表：" + sCreateTabSQL).c_str(), __CURR_CODE_PLACE_C__);
    };

    // 创建批量插入语句
    auto fNewInsertSQL = [&]()
    {
        // m_sql = "  INSERT INTO " + sTableName + " ( " + sFields + " )\n";
        // if (vctPKey.empty())
        //     m_sql += "  values ( " + sArgs + " )\n";
        // else
        // {
        //     m_sql += "  select " + sArgs + " \n";
        //     m_sql += "  where not exists (select 1 from " + sTableName + " where 1 = 1";
        //     for (string &sKey : vctPKey)
        //         m_sql += " and " + sKey + " = :" + sKey;
        //     m_sql += ")\n";
        // }
        m_sql = (boost::format("  INSERT %s INTO %s (%s)\n  values(%s)\n")
                    % (/*vctPKey.empty() ? "" : */"OR IGNORE")
                    % sTableName % sFields % sArgs).str();
    };

    // 从json批量数组参数里取
    auto fFromJson = [&](void)
    {
        // 批量参数json数组
        const IKCJson& jsonVals = m_objCtrlD.GetBatchArrayJson(sCopyValsParm.c_str());
        jsonVals.GC();
        // 有数据
        if (jsonVals.GetItemCount() > 0)
        {
            // 创建表
            fCtreateTable([&](string& sCreateTabSQL){
                // 未指定表结构，取第1条记录的结构
                auto &jsnFirst = jsonVals.GetItem(static_cast<unsigned>(0));
                jsnFirst.InitItemNextStep();
                // 循环每个字段
                for (unsigned i = 0, c = jsnFirst.GetItemCount(); i < c; ++i)
                {
                    // auto &jsnFeild = jsnFirst.GetItem(i);
                    auto &jsnFeild = jsnFirst.GetItemNext();
                    string sFeildName = jsnFeild.GetName();
                    string sFeildType = jsnFeild.GetType() == IKCJson::edtNumber ? "numeric" : "varchar";
                    sCreateTabSQL += sFeildName + " " + sFeildType + ",";
                    sFields += " " + sFeildName + ",";
                    sArgs += " :" + sFeildName + ",";
                }
            });
            // 创建批量插入语句
            fNewInsertSQL();
            // 调用批量插入
            this->BatchInsert();
        }
    };

    // 从内部数据集里取
    auto fFromInner = [&](CSqlFunc::TRecordSetPtr dbSet)
    {
        // 调用批量插入
        this->BatchInsert([&](CSqlFunc::TFields& vecFields){
            // 创建表
            fCtreateTable([&](string& sCreateTabSQL){
                // 循环每个字段
                for (auto fFeild : vecFields)
                {
                    string sFeildName = fFeild->m_name;
                    string sFeildType = fFeild->GetTypeName();
                    sCreateTabSQL += sFeildName + " " + sFeildType + ",";
                    sFields += " " + sFeildName + ",";
                    sArgs += " :" + sFeildName + ",";
                }
            });
            // 创建批量插入语句
            fNewInsertSQL();
        });
    };

    // 获取上一个控制器的内部数据集
    CSqlFunc::ICtrlApiDataWithDbSet* pCtrl = dynamic_cast<CSqlFunc::ICtrlApiDataWithDbSet*>(&m_objCtrlD);
    CSqlFunc::TRecordSetPtr lastInnerDbSet;
    if (nullptr != pCtrl)
        lastInnerDbSet = pCtrl->DataSetInnerLast(sSrcDbSetName);
    // 从json批量数组参数里拷贝数据
    if (lastInnerDbSet.get() == nullptr) fFromJson();
    // 从内部数据集里拷贝数据
    else fFromInner(lastInnerDbSet);
}

// CCtrlSqlite::TSqliteConnObj 类
CCtrlSqlite::TSqliteConnObj::TSqliteConnObj(CCtrlSqlite& own, ICtrlApiData& objCtrlD)
    : m_con([&]() -> sqlite3&{
        // 数据库文件
        string sDbFile = own.m_dbFile;
        // 动态数据库文件
        if (sDbFile.empty())
        {
            sDbFile = objCtrlD.JsonAttach().GetItem("sqliteParms").GetStr("DB", "");
            if (sDbFile.empty()) objCtrlD.JsonAttachByKcFile().GetItem("sqliteParms").GetStr("DB", "");
            if (sDbFile.empty())
                throw TKCSqliteException(ecd_ErrCode_KCSqlSqlite + 5, __CURR_CODE_PLACE_C__, "Unspecified Database File", own.m_own);
            if (":memory:" != sDbFile)
            {
                string sFile = CUtilFunc::PCharSafeToStr(objCtrlD.GetParmVal(sDbFile.c_str()));
                if (!sFile.empty()) sDbFile = sFile;
            }
        }
        // 数据库绝对路径
        string dbFileFull = sDbFile;
        if (":memory:" != dbFileFull)
        {
            // 数据库绝对路径
            dbFileFull = own.m_own.getContext().transCfgPathToFullPath(sDbFile.c_str());
            // 创建目录
            auto dbDir = boost::filesystem::path(dbFileFull).parent_path();
            if (!boost::filesystem::exists(dbDir)) boost::filesystem::create_directories(dbDir);
        }
        // 是否新创
        bool bNewDB = !boost::filesystem::exists(dbFileFull);
        // 创建（打开）数据库
        sqlite3 *conn = nullptr;
        int iErrCode = SQLITE_OK;
        if ((iErrCode = sqlite3_open(dbFileFull.c_str(), &conn)) != SQLITE_OK || nullptr == conn)
        {
            string sErr = "Unable To Open Sqlite File - " + sDbFile;
            own.m_own.WriteLogError((sErr + "\n" + dbFileFull).c_str(), __CURR_CODE_PLACE_C__, sqlite3_errstr(iErrCode));
            throw TKCSqliteException(ecd_ErrCode_KCSqlSqlite + 5, __CURR_CODE_PLACE_C__, sErr, own.m_own);
        }
        // 执行预设的配置。例如，启用WAL模式，实现高并发
        if (bNewDB && !own.m_PRAGMA.empty())
        {
            try
            {
                char *zErrMsg = nullptr;
                int rc = sqlite3_exec(conn, own.m_PRAGMA.c_str(), nullptr, nullptr, &zErrMsg);
                CAutoRelease _auto([&](){ if (nullptr != zErrMsg) sqlite3_free(zErrMsg); });
                if (rc != SQLITE_OK)
                {
                    string sErr = CUtilFunc::PCharSafeToStr(zErrMsg) + "\n" + own.m_PRAGMA;
                    std::cerr << "SQLite error: " << sErr << std::endl;
                    own.m_own.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, sqlite3_errstr(rc));
                }
            }
            catch (...)
            {
                own.m_own.WriteLogError(("Config Connect SQLite Error: " + own.m_PRAGMA).c_str(), __CURR_CODE_PLACE_C__);
            }
        }
        return *conn;
    }())
{
}
void CCtrlSqlite::TSqliteConnObj::Release(void)
{
    sqlite3_close(&m_con);
    delete this;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlSqlite 类
CCtrlSqlite::CCtrlSqlite(IKCSqlite& own, string sName, property_tree::ptree& pt)
    : m_own(own), m_name(sName)
    , m_dbFile([&](){
        string sDbFile;
        // 静态数据库文件
        if (pt.get_child_optional("<xmlattr>.str"))
            sDbFile = pt.get<string>("<xmlattr>.str");
        return sDbFile;
    }())
    , m_PRAGMA([&](){
        string sPRAGMA;
        // 静态数据库文件
        if (pt.get_child_optional("<xmlattr>.m_PRAGMA"))
            sPRAGMA = pt.get<string>("<xmlattr>.m_PRAGMA");
        return sPRAGMA;
    }())
{
    // 记录函数地址
    m_own.WriteLogDebug((m_name + ": "
                            + "\nCCtrlMssql::Exce: \t\t" + CUtilFunc::GetFuncAddr(&CCtrlSqlite::Perform)
                       ).c_str());
}
CCtrlSqlite::~CCtrlSqlite()
{
}

// 获取类型名
CSqlFunc::EDataType CCtrlSqlite::GetTypeName(unsigned int oid)
{
    switch (oid)
    {
    case SQLITE_FLOAT:
        return CSqlFunc::EDataType::edtNumber;
        break;
    case SQLITE_INTEGER:
        return CSqlFunc::EDataType::edtInt;
        break;
    case SQLITE_BLOB:
    case SQLITE_TEXT:
    default:
        return CSqlFunc::EDataType::edtString;
        break;
    }
}

// 获取数据库文件的绝对路径
string CCtrlSqlite::GetDBFile(ICtrlApiData& objCtrlD)
{
    string sDbFile = this->m_dbFile;
    if (":memory:" == sDbFile) return sDbFile;
    // 动态数据库文件
    if (sDbFile.empty())
    {
        sDbFile = objCtrlD.JsonAttach().GetItem("sqliteParms").GetStr("DB", "");
        if (sDbFile.empty()) objCtrlD.JsonAttachByKcFile().GetItem("sqliteParms").GetStr("DB", "");
        if (sDbFile.empty())
            throw TKCSqliteException(ecd_ErrCode_KCSqlSqlite + 5, __CURR_CODE_PLACE_C__, "Unspecified Database File", m_own);
        if (":memory:" == sDbFile) return sDbFile;
        string sFile = CUtilFunc::PCharSafeToStr(objCtrlD.GetParmVal(sDbFile.c_str()));
        if (!sFile.empty()) sDbFile = sFile;
        if (":memory:" == sDbFile) return sDbFile;
    }
    // 数据库绝对路径
    string dbFileFull = m_own.getContext().transCfgPathToFullPath(sDbFile.c_str());
    // 创建目录
    auto dbDir = boost::filesystem::path(dbFileFull).parent_path();
    if (!boost::filesystem::exists(dbDir)) boost::filesystem::create_directories(dbDir);
    return sDbFile;
}

// 执行控制器
void CCtrlSqlite::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm& attachP)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pSQL = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sSQL, sJsonParm, sAct = CUtilFunc::PCharSafeToStr(pAct);
    string sSrvFlag = m_name + "@" + CUtilFunc::PCharSafeToStr(m_own.getAlias(), "kc_sql_sqlite");
    // 异常信息
    int iExecSort = 0;
    auto fExceptInfo = [&](void)
    {
        return (format("\n%d.%s%s%s%s%s%s%s\n%s?act=%s") % iExecSort % act.GetSingleInfo("UniqueConnID") % c_strLineShow % sSQL % c_strLineShow % sJsonParm % c_strLineShow % act.GetSingleInfo("the_request") % pLocalFile % sAct).str();
    };
    try
    {
        // 方法类型
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, c_RESTful_exec));
        algorithm::to_lower(sMethod);
        // SQL语句
        sSQL = CUtilFunc::PCharSafeToStr(pSQL);
        // 参数
        sJsonParm = objCtrlD.JsonRequest().ToStr();
        cout << "\t" << CUtilFunc::Utf8ToGbk(sJsonParm).substr(0, 500) << endl;
        const char* pKCCLNID = act.GetGlobalVal("KC__SESSION__KCCLNID");
        m_own.WriteLogTrace(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (sSQL.substr(0, 256) + "\r\n" + sJsonParm.substr(0, 256) + "\r\n" + pLocalFile + "\t-\t" + sAct + "\npKCCLNID=" + (nullptr != pKCCLNID ? pKCCLNID : "")).c_str());
        // 数据库链接
        CSqlFunc::TConnObjWork<TSqliteConnObj> actObjWork(objCtrlD, /*sSrvFlag,*/ [&]() { return new TSqliteConnObj(*this, objCtrlD); });
        try
        {
            auto &conn = actObjWork.ActObj().m_con;
            // 控制器执行类型
            string sActGoTo = CUtilFunc::PCharSafeToStr(objCtrlD.GetGetArg(c_RESTful_goTo));
            // 创建数据库执行实例
            TDBCommandSqlite comm(*this, objCtrlD, pLocalFile, sAct, conn, sSQL, sMethod);
            CAutoRelease _auto([&](){ sSQL = comm.GetSQL(); });
            iExecSort = static_cast<int>(comm.m_execSort);
            // 查询（method="select"）
            if (c_RESTful_select == sMethod || c_RESTful_query == sMethod)
                comm.Select();
            // 批量导入（method="batch"）
            else if (c_RESTful_batch == sMethod)
                comm.BatchInsert();
            // 仿postgresql的copy
            else if ("copy" == sMethod)
                comm.CopyBatch();
            // 创建表（method="create"）
            else if (c_RESTful_create == sMethod)
                comm.CreateDDL();
            // 增删改（method="update", "insert", "delete"）
            else if (sMethod.empty() || c_RESTful_exec == sMethod || c_RESTful_insert == sMethod || c_RESTful_delete == sMethod || c_RESTful_update == sMethod)
                comm.InsertDeleteUpdate();
            // 方法类型错误
            else throw TSqliteException(ecd_ErrCode_KCSqlSqlite + 15, string(m_own.getHint("Request_Failed_")) + m_own.getHint("Method_Not_Allowed") + " [" + sMethod + "]", "", __CURR_CODE_PLACE_C__);
        }
        catch (...)
        {
            // 异常时，关闭持久连接
            if ("on" == actObjWork.m_keepAlive || "first" == actObjWork.m_keepAlive)
                act.SetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_keepAlive).c_str(), "off");
            throw;
        }
    }
    catch(TSqliteException& ex)
    {
        string sErr = (format("[%s]%d: %s\n%s") % ex.m_pos % ex.m_code % ex.m_msg % fExceptInfo()).str();
        m_own.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iErrCode = ex.m_code;
        objCtrlD.SetJsonRespond(0 == iErrCode ? ecd_ErrCode_KCSqlSqlite + 6 : iErrCode, ("[Controllers." + sAct + "] " + ex.m_err).c_str());
    }
    catch (TKCSqlException& ex)
    {
        string sEx = ex.error_info();
        string sErr = "[" + std::to_string(ex.error_id()) + "]" + ex.error_place() + sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iCode ? ecd_ErrCode_KCSqlSqlite + 7 : iCode, ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        ex.OtherInfo() += fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.BackTrace() = CBacktraceSymbols::Get();
        m_own.WriteLog(ex);
        int iErrCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iErrCode ? ecd_ErrCode_KCSqlSqlite + 8 : iErrCode, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlSqlite + 9, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlSqlite + 10, ("[Controllers." + sAct + "] " + m_own.getHint("Unknown_exception") + " (" + sSrvFlag + ")").c_str());
    }
}
