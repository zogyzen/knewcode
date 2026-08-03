#include "ctrl_postgresql.h"

////////////////////////////////////////////////////////////////////////////////
// libpq异常类
CCtrlPgSQL::TLibPQException::TLibPQException(int c, string e, string s, string p)
    : std::runtime_error(s.c_str()), m_code(c), m_err(e), m_msg(s), m_pos(p)
{
}

// 检查错误
ExecStatusType CCtrlPgSQL::TLibPQException::CheckError(PGresult* res, string pos)
{
    auto resStatus = PQresultStatus(res);
    if(PGRES_EMPTY_QUERY == resStatus)
        throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 3, "empty query string", "empty query string was executed", pos);
    return resStatus;
}

// CCtrlPgSQL::TParmPgSQL 类
CCtrlPgSQL::TParmPgSQL::TParmPgSQL(TParmGrpPgSQL& own, std::string s, unsigned p) : m_own(own)
{
    iPos = p;
    sName = s;
}

// 设置参数值
bool CCtrlPgSQL::TParmPgSQL::SetNumber(double fVal)
{
    // this->bIsNull = false;
    // // this->sVal = std::to_string(fVal);
    // this->anyVal = fVal;
    return false;
}

void CCtrlPgSQL::TParmPgSQL::SetInt(int iVal)
{
    // this->bIsNull = false;
    // // this->sVal = std::to_string(iVal);
    // this->anyVal = iVal;
}

void CCtrlPgSQL::TParmPgSQL::SetString(string strVal)
{
    // this->bIsNull = false;
    // // this->sVal = strVal;
    // this->anyVal = strVal;
}
void CCtrlPgSQL::TParmPgSQL::SetClob(string strVal)
{
    this->SetString(strVal);
}

void CCtrlPgSQL::TParmPgSQL::SetNull(void)
{
    // this->bIsNull = true;
    // // this->sVal.clear();
    // this->anyVal.clear();
}

// 获取参数值
string CCtrlPgSQL::TParmPgSQL::GetVal(string sDef)
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
string CCtrlPgSQL::TParmPgSQL::GetString(string sDef)
{
    return this->GetVal(sDef);
}
string CCtrlPgSQL::TParmPgSQL::GetClob(string sDef)
{
    return this->GetString(sDef);
}

string CCtrlPgSQL::TParmPgSQL::GetDate(string /*fmt*/)
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

string CCtrlPgSQL::TParmPgSQL::GetDateTime(string /*fmt*/)
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

int CCtrlPgSQL::TParmPgSQL::GetInt(int iDef)
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

double CCtrlPgSQL::TParmPgSQL::GetNumber(double fDef)
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

// 获取临时表数据集
CSqlFunc::TRecordSetPtr CCtrlPgSQL::TParmPgSQL::GetDbSet(void)
{
    string sTmpTableName = m_own.m_own.m_dynTmpTableName ? this->sAliasDyn : this->sName;
    try
    {
        // CAutoRelease _auto([&](){ try { PQexec(&m_own.m_con, ("drop table if exists pg_temp." + sTmpTableName).c_str()); } catch (...) {} });
        string sSQL = "select * from pg_temp." + sTmpTableName;
        // cout << sSQL << endl;
        PGresult* res = PQexec(&m_own.m_con, sSQL.c_str());
        CSqlFunc::TRecordSetPtr resPtr(new TRecordSetPgSQL(m_own.m_own, m_own.m_objCtrlD, *res));
        TLibPQException::CheckError<PGRES_TUPLES_OK>(res, sSQL, __CURR_CODE_PLACE_C__);
        return resPtr;
    }
    catch (TLibPQException& ex)
    {
        string sErr = ex.m_msg + "\n" + m_own.m_objCtrlD.ActionData().GetSingleInfo("the_request") + " \t\t" + c_RESTful_outParm + ": " + this->sName + " (" + sTmpTableName + ") ";
        m_own.m_own.m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        return CSqlFunc::TRecordSetPtr();
    }
}

void CCtrlPgSQL::TParmPgSQL::SetDbTp(void)
{
    switch (eDTp)
    {
    case CSqlFunc::EDataType::edtInt:
        sDbTP = "int";
        m_oid = INT4OID;
        break;
    case CSqlFunc::EDataType::edtNumber:
        sDbTP = "numeric";
        m_oid = NUMERICOID;
        break;
    case CSqlFunc::EDataType::edtDatetime:
        sDbTP = "timestamp";
        m_oid = TIMESTAMPOID;
        break;
    case CSqlFunc::EDataType::edtJson:
        sDbTP = "json";
        m_oid = JSONOID;
        break;
    case CSqlFunc::EDataType::edtClob:
        sDbTP = "text";
        m_oid = TEXTOID;
        break;
    case CSqlFunc::EDataType::edtString:
        sDbTP = "varchar";
        m_oid = VARCHAROID;
        break;
    case CSqlFunc::EDataType::edtUnknown:
        // 未指定类型，通过值类型
        if (anyVal.type() == typeid(int))
        {
            sDbTP = "int";
            m_oid = INT4OID;
        }
        else if (anyVal.type() == typeid(long long))
        {
            sDbTP = "int";
            m_oid = INT8OID;
        }
        else if (anyVal.type() == typeid(double))
        {
            sDbTP = "numeric";
            m_oid = NUMERICOID;
        }
        else if (anyVal.type() == typeid(string))
        {
            sDbTP = "varchar";
            m_oid = VARCHAROID;
        }
        else
        {
            sDbTP = "varchar";
            m_oid = 0;
        }
        break;
    default:
        sDbTP = "varchar";
        m_oid = 0;          // 让引擎自动推导
    }
}

// CCtrlPgSQL::TParmGrpPgSQL 类
CCtrlPgSQL::TParmGrpPgSQL::TParmGrpPgSQL(CCtrlPgSQL& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, PGconn &con)
    : TParmGrpPgSQLBase(own.m_parmCaseSensitive), m_own(own), m_objCtrlD(ctrlD), m_kcFile(sKcFile), m_actName(sActName), m_con(con)
{
}

// 所属的服务
IKCSql& CCtrlPgSQL::TParmGrpPgSQL::Srv(void)
{
    return m_own.m_own;
}

//  控制器 信息
string CCtrlPgSQL::TParmGrpPgSQL::ActInfo(void)
{
    return m_kcFile + "?act=" + m_actName;
}

// 创建参数
CSqlFunc::TParmPtr CCtrlPgSQL::TParmGrpPgSQL::MakePram(std::string s, unsigned p)
{
    return CSqlFunc::TParmPtr(new TParmPgSQL(*this, s, p));
}

// 异常信息
string CCtrlPgSQL::TParmGrpPgSQL::ParmExceptInfo(TLibPQException& ex)
{
    return (format("%d: %s") % ex.m_code % ex.m_msg).str();
}

// CCtrlPgSQL::TRecordSetPgSQL 类
CCtrlPgSQL::TRecordSetPgSQL::TRecordSetPgSQL(CCtrlPgSQL& own, ICtrlApiData& ctrlD, PGresult& st)
    : m_own(own), m_objCtrlD(ctrlD), m_dbset(st), m_recCount(PQntuples(&st)), m_fieldCount(PQnfields(&st))
{
}
CCtrlPgSQL::TRecordSetPgSQL::~TRecordSetPgSQL(void)
{
    PQclear(&m_dbset);
}

// 获取字段信息
void CCtrlPgSQL::TRecordSetPgSQL::GetColumnListMetaData(CSqlFunc::TFields &vecFeilds)
{
    map<string, std::shared_ptr<CSqlFunc::TField>> mapFeilds;
    for (int i = 0; i < m_fieldCount; ++i)
    {
        // 列属性值
        int iColType = PQftype(&m_dbset, i);
        string sColName = PQfname(&m_dbset, i);
        auto sTypeName = CCtrlPgSQL::GetTypeName(iColType);
        int iSize = PQfsize(&m_dbset, i);
        if (!m_own.m_parmCaseSensitive) sColName = algorithm::to_lower_copy(sColName);
        mapFeilds.insert(make_pair(sColName, std::shared_ptr<CSqlFunc::TField>(
                        new CSqlFunc::TField(sColName, i + 1, iColType, sTypeName, iSize))));
    }
    if (vecFeilds.empty())
        for (int i = 0; i < m_fieldCount; ++i)
        {
            string sFieldName = PQfname(&m_dbset, i);
            if (!m_own.m_parmCaseSensitive) sFieldName = algorithm::to_lower_copy(sFieldName);
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
bool CCtrlPgSQL::TRecordSetPgSQL::Next(void)
{
    bool bResult = m_resStep + 1 < m_recCount;
    if (bResult) ++m_resStep;
    return bResult;
}
// 添加值
void CCtrlPgSQL::TRecordSetPgSQL::AddVal(IKCJson& jsonRow, CSqlFunc::TField& fld)
{
    if (fld.m_pos > 0 && PQgetisnull(&m_dbset, m_resStep, fld.m_pos - 1) != 1)
    {
        auto strVal = PQgetvalue(&m_dbset, m_resStep, fld.m_pos - 1);
        switch (fld.m_type)
        {
            case NUMERICOID:
            case FLOAT8OID:
            case FLOAT4OID:
                jsonRow.AddVal(fld.m_name.c_str(), atof(strVal));
                break;
            case INT8OID:
                jsonRow.AddVal(fld.m_name.c_str(), static_cast<double>(atoll(strVal)));
                break;
            case INT4OID:
            case INT2OID:
                jsonRow.AddVal(fld.m_name.c_str(), atoi(strVal));
                break;
            case DATEOID:
            case TIMESTAMPOID:
            case TIMESTAMPTZOID:
                jsonRow.AddStr(fld.m_name.c_str(), strVal);
                break;
            case JSONOID:
            case JSONBOID:
                jsonRow.AddJson(fld.m_name.c_str(), strVal);
                break;
            case VARCHAROID:
            default:
                jsonRow.AddStr(fld.m_name.c_str(), strVal);
                break;
        }
    }
    else jsonRow.AddNull(fld.m_name.c_str());
}
// 得到值
CSqlFunc::TKcValVariant CCtrlPgSQL::TRecordSetPgSQL::GetVal(CSqlFunc::TField& fld) const
{
    CSqlFunc::TKcValVariant result;
    if (fld.m_pos > 0 && PQgetisnull(&m_dbset, m_resStep, fld.m_pos - 1) != 1)
    {
        auto strVal = PQgetvalue(&m_dbset, m_resStep, fld.m_pos - 1);
        if (nullptr != strVal)
            switch (fld.m_type)
            {
                case NUMERICOID:
                case FLOAT8OID:
                case FLOAT4OID:
                    result = atof(strVal);
                    break;
                case INT8OID:
                    result = atoll(strVal);
                    break;
                case INT4OID:
                case INT2OID:
                    result = atoi(strVal);
                    break;
                case DATEOID:
                case TIMESTAMPOID:
                case TIMESTAMPTZOID:
                    result = string(strVal);
                    break;
                case JSONOID:
                case JSONBOID:
                    result = string(strVal);
                    break;
                case VARCHAROID:
                default:
                    result = string(strVal);
                    break;
            }
    }
    return result;
}

// CCtrlPgSQL::TDBCommandPgSQL 类
CCtrlPgSQL::TDBCommandPgSQL::TDBCommandPgSQL(CCtrlPgSQL& own, ICtrlApiData& ctrlD, string sKcFile, string sActName, PGconn &con, string sSQL, string sMethod)
    : m_own(own), m_objCtrlD(ctrlD), m_kcFile(sKcFile), m_actName(sActName), m_stmtName("__" + sActName + std::to_string(m_execSort))
    , m_method(sMethod), m_con(con), m_sql(sSQL), m_parms(own, m_objCtrlD, sKcFile, sActName, con)
    , m_batchIsProc(c_RESTful_batch == sMethod && CCtrlCommon::IsSQLProcedures(sSQL))
{
}

// 所属的服务
IKCSql& CCtrlPgSQL::TDBCommandPgSQL::Srv(void)
{
    return m_own.m_own;
}

// 控制器信息
string CCtrlPgSQL::TDBCommandPgSQL::ActInfo(void)
{
    return m_kcFile + "?act=" + m_actName;
}

// 事务
void CCtrlPgSQL::TDBCommandPgSQL::TranBegin(void)
{
    PQexec(&m_con, "BEGIN TRANSACTION");
}
void CCtrlPgSQL::TDBCommandPgSQL::TranCommit(void)
{
    PQexec(&m_con, "COMMIT");
}
void CCtrlPgSQL::TDBCommandPgSQL::TranRollback(void)
{
    PQexec(&m_con, "ROLLBACK");
}
void CCtrlPgSQL::TDBCommandPgSQL::TranSavePoint(string)
{
    string sStmtName = m_stmtName + "_pnt";
    if (!m_tranSavePnt.empty())
        // PQexec(&m_con, ("release savepoint " + m_tranSavePnt).c_str());
        PQexecPrepared(&m_con, (sStmtName + "_free").c_str(), 0, nullptr, nullptr, nullptr, 0);
    m_tranSavePnt = sStmtName;
    // if (!m_tranSavePnt.empty())
    //     PQexec(&m_con, ("savepoint " + m_tranSavePnt).c_str());
    PQexecPrepared(&m_con, (sStmtName + "_save").c_str(), 0, nullptr, nullptr, nullptr, 0);
}
void CCtrlPgSQL::TDBCommandPgSQL::TranRollbackToSavePoint(void)
{
    string sStmtName = m_stmtName + "_pnt";
    // if (!m_tranSavePnt.empty())
        // PQexec(&m_con, ("rollback to savepoint " + m_tranSavePnt).c_str());
    PQexecPrepared(&m_con, (sStmtName + "_roll").c_str(), 0, nullptr, nullptr, nullptr, 0);
    m_tranSavePnt.clear();
}

// 预备sql语句
void CCtrlPgSQL::TDBCommandPgSQL::PrepareSQL(void)
{
    // SQL语句不能为空
    if (m_sql.empty())
        throw TKCSqlPgSQLException(ecd_ErrCode_KCSqlPostgresql + 5, __CURR_CODE_PLACE_C__, string(m_own.m_own.getHint("SQL_Empty_")) + m_objCtrlD.ActionData().GetSingleInfo("the_request") + " - " + m_actName, m_own.m_own);
    // DDL语句（创建函数、存储过程等），将参数设置到全局
    if (c_RESTful_sql_DDL == this->GetParms().sSQLWorkType)
        PrepareByDDLSQL();
    // DML、DQL（增、删、改、查、执行过程等）语句
    else if (c_RESTful_sql_DML == this->GetParms().sSQLWorkType || c_RESTful_sql_DQL == this->GetParms().sSQLWorkType)
    {
        // 过程，创建临时函数
        if (c_RESTful_procedures == m_method || m_batchIsProc)
            PrepareByProcedures();
        // 单语句，替换sql语句里的参数格式（从“:XXX”替换为“$xxx”）
        else PrepareByOneSQL();
    }
    cout << "[PrepareSQL] " << m_sql << endl;
}
// 预执行DDL语句
void CCtrlPgSQL::TDBCommandPgSQL::PrepareByDDLSQL(void)
{
    // 有参数，预先设置全局配置参数
    if (!m_parms.mapParms.empty())
    {
        // 参数
        vector<unsigned int> vctParmOID(m_parms.mapParms.size());
        for (auto &oid : vctParmOID) oid = VARCHAROID;
        vector<string> vctParmStr(m_parms.mapParms.size());
        vector<const char*> vctParm(m_parms.mapParms.size());
        // 循环添加全局配置参数
        int i = 0;
        string sValInfo;
        string sCfgSql = "select \n";
        for (auto &parm : m_parms.mapParms)
        {
            ++i;
            // 参数值
            bool isNull = parm.second->IsNull() || ((CSqlFunc::EDataType::edtInt == parm.second->eDTp || CSqlFunc::EDataType::edtNumber == parm.second->eDTp) && parm.second->anyVal.empty());
            if (!isNull) vctParmStr[i - 1] = BoostAnyToStdString(parm.second->anyVal);
            vctParm[i - 1] = isNull ? nullptr : vctParmStr[i - 1].c_str();
            sValInfo += isNull ? (format("$%d is null\n") % i).str() : (format("$%d = '%s'\n") % i % vctParmStr[i - 1]).str();
            // 全局配置参数名
            string sCfgName = (format("_$kc_.%s.%s") % m_actName % parm.second->sName).str();
            // 设置全局配置参数语句
            sCfgSql += (format("\tset_config('%s', $%d, false) v%d,\n") % sCfgName % i % i).str();
            // 替换sql语句中的参数名
            algorithm::ireplace_all(sCfgName, "$", "\\$");
            string sValFmt = " current_setting('" + sCfgName + "')";
            if ("jna" == m_own.m_loadType)
            {
                string sPattern = ":" + parm.second->sName + "\\b";
                if (m_own.m_parmCaseSensitive)
                {
                    std::regex pattern(sPattern);
                    m_sql = std::regex_replace(m_sql, pattern, isNull ? " nullif(" + sValFmt + ", '')" : sValFmt);
                }
                else
                {
                    std::regex pattern(sPattern, std::regex::icase);
                    m_sql = std::regex_replace(m_sql, pattern, isNull ? " nullif(" + sValFmt + ", '')" : sValFmt);
                }
            }
            else
            {
                string sPattern = "(?<!:):" + parm.second->sName + "\\b";
                boost::regex pattern(sPattern, m_own.m_parmCaseSensitive ? boost::regex::nocollate : boost::regex::icase);
                m_sql = boost::regex_replace(m_sql, pattern, isNull ? " nullif(" + sValFmt + ", '')" : sValFmt);
            }
        }
        boost::trim(sCfgSql);
        boost::trim_if(sCfgSql, boost::is_any_of(","));
        sCfgSql += "\n";
        // 执行添加全局配置参数
        PGresult* res = PQexecParams(&m_con, sCfgSql.c_str(), static_cast<int>(vctParm.size()), &vctParmOID[0], &vctParm[0], nullptr, nullptr, 0);
        CAutoRelease _auto([&](){ PQclear(res); });
        // 错误信息
        auto resStatus = TLibPQException::CheckError(res, __CURR_CODE_PLACE_C__);
        if (PGRES_COMMAND_OK != resStatus && PGRES_TUPLES_OK != resStatus)
            m_own.m_own.WriteLogError((sCfgSql + sValInfo).c_str(), __CURR_CODE_PLACE_C__, (boost::format("%s - %s: %s") % m_objCtrlD.ActionData().GetSingleInfo("UniqueConnID") % CUtilFunc::PCharSafeToStr(PQcmdStatus(res)) % PQresultErrorMessage(res)).str().c_str());
        else
            m_own.m_own.WriteLogDebug((sCfgSql + sValInfo).c_str(), __CURR_CODE_PLACE_C__, m_objCtrlD.ActionData().GetSingleInfo("UniqueConnID"));
        // 清除参数
        m_parms.mapParms.clear();
    }
}
// 预执行过程
void CCtrlPgSQL::TDBCommandPgSQL::PrepareByProcedures(void)
{
    // string sLogInfo = "<R" + m_sExecSort + ".> Prepare By Procedures" + c_strLineShow + m_sql.substr(0, 1024) + c_strLineShow;
    string sLogInfo = "<R" + m_sExecSort + ".> Prepare By Procedures" + c_strLineShow;
    // 当前时间标识
    //string sNowTime = posix_time::to_iso_string(posix_time::second_clock::local_time());
    // 时间戳的base62编码
    string sNowTime = m_objCtrlD.ActionData().GetNowTimeFlag();
    // 文件名
    string sFilename = boost::filesystem::path(m_kcFile).filename().string();
    sFilename = algorithm::replace_all_copy(sFilename, ".", "_");
    // 临时函数名
    string sProcName = (format("_%s__%s_$%s") % m_actName % sFilename % sNowTime).str();
    // 将返回数据集的临时表名，替换为动态名称
    if (m_own.m_dynTmpTableName)
    {
        for (auto &outPrm : m_parms.mapOutParms)
            if (CSqlFunc::EDataType::edtDbSet == outPrm.second->eDTp || CSqlFunc::EDataType::edtDbSetOne == outPrm.second->eDTp || CSqlFunc::EDataType::edtDbSetArray == outPrm.second->eDTp || CSqlFunc::EDataType::edtDbSetInner == outPrm.second->eDTp)
            {
                TParmPgSQL *pOutPgPrm = dynamic_cast<TParmPgSQL*>(outPrm.second.get());
                if (nullptr != pOutPgPrm)
                {
                    // 临时表的动态名
                    pOutPgPrm->sAliasDyn = "_" + pOutPgPrm->sName + "__" + sNowTime;
                    // 替换sql语句中的临时表名
                    m_sql = std::regex_replace(m_sql, std::regex("##" + pOutPgPrm->sName + "\\b", m_own.m_parmCaseSensitive ? std::regex::ECMAScript : std::regex::icase), pOutPgPrm->sAliasDyn);
                    string sSrc = "CREATE\\s+TEMP(ORARY)?\\s+TABLE\\s+(IF\\s+NOT\\s+EXISTS\\s+)?(pg_temp\\s*.\\s*)?" + pOutPgPrm->sName + "\\b";
                    string sDst = "CREATE TEMP TABLE " + pOutPgPrm->sAliasDyn;
                    m_sql = std::regex_replace(m_sql, std::regex(sSrc, std::regex::icase), sDst);
                }
            }
    }
    // 替换动态临时表名
    auto &jsnDynTmpTableNames = m_objCtrlD.JsonAttach().GetItem("dynTmpTableName");
    for (int i = 0, c = jsnDynTmpTableNames.GetItemCount(); i < c; ++i)
    {
        string sTmpTableName = jsnDynTmpTableNames.GetItem(i).GetStr();
        string sAliasDyn = "_" + sTmpTableName + "__" + sNowTime;
        // 替换sql语句中的临时表名
        m_sql = std::regex_replace(m_sql, std::regex("##" + sTmpTableName + "\\b", m_own.m_parmCaseSensitive ? std::regex::ECMAScript : std::regex::icase), sAliasDyn);
    }
    // 创建临时函数的sql语句
    // string sFuncType = !m_parms.mapOutParms.empty() ? "function" : "procedure";
    string sFuncType = "function";
    string sFuncCreateSQL = "create or replace " + sFuncType + " pg_temp." + sProcName + "(";
    // 返回值数量和类型
    m_resultParmCount = 0;
    string sResultParmType = "void";
    // 循环拼加形参
    for (auto &parm : m_parms.mapParms)
    //for (auto iter = m_parms.mapParms.rbegin(); m_parms.mapParms.rend() != iter; ++iter)
    {
        // 是否输出参数
        const bool bIsOutParm = m_parms.mapOutParms.end() != m_parms.mapOutParms.find(parm.second->sName) || CSqlFunc::EParmType::eptSessionParm == parm.second->ePTp;
        if (bIsOutParm)
        {
            ++m_resultParmCount;
            sResultParmType = parm.second->Get<TParmPgSQL>().sDbTP;
        }
        //auto &parm = *iter;
        // 参数名
        string sParmName = (format("%s___%d__%s") % parm.second->sName % parm.second->iPos % sNowTime).str();
        // 参数输入输出类型
        string sParmInOutType = bIsOutParm ? "INOUT" : "IN";
        // 拼加
        sFuncCreateSQL += sParmInOutType + " " + sParmName + " " + parm.second->Get<TParmPgSQL>().sDbTP + ",";

        // 替换sql语句中的参数名
        //        if (m_own.m_parmCaseSensitive)
        //            algorithm::replace_all(m_sql, ":" + parm.second->sName, sParmName);
        //        else
        //            algorithm::ireplace_all(m_sql, ":" + parm.second->sName, sParmName);
        algorithm::ireplace_all(sParmName, "$", "\\$");

        if ("jna" == m_own.m_loadType)
        {
            // std::regex不支持“零宽度断言”（负后顾断言）
            string sPattern = ":" + parm.second->sName + "\\b";
            m_sql = std::regex_replace(m_sql, m_own.m_parmCaseSensitive ? std::regex(sPattern) : std::regex(sPattern, std::regex::icase), sParmName);
        }
        else
        {
            // “零宽度断言”（负后顾断言）
            string sPattern = "(?<!:):" + parm.second->sName + "\\b";
            boost::regex pattern(sPattern, m_own.m_parmCaseSensitive ? boost::regex::nocollate : boost::regex::icase);
            m_sql = boost::regex_replace(m_sql, pattern, sParmName);
        }
    }
    algorithm::trim_right_if(sFuncCreateSQL, algorithm::is_any_of(","));
    // 无参数时，加入占位参数
    // if (m_parms.mapParms.size() == 0) sFuncCreateSQL += "INOUT __0_" + sNowTime + "__ varchar";
    // 返回值类型
    // string sResultType = m_parms.mapOutParms.size() == 1 ? m_parms.mapOutParms.begin()->second->Get<TParmPgSQL>().sDbTP : (m_parms.mapOutParms.size() == 0 ? "void" : "record");
    string sResultType = m_resultParmCount > 1 ? "record" : sResultParmType;
    // 添加sql语句体
    //sFunc += ") returns " + sResultType + " as $" + sProcName + "$ begin " + m_sql + "\nend $" + sProcName + "$ language plpgsql;\n";
    string sFlagName = "$" + algorithm::replace_all_copy(sProcName, "$", "") + "$";
    sFuncCreateSQL += ") returns " + sResultType + " as " + sFlagName;
    sLogInfo += sFuncCreateSQL + "\n......" + c_strLineShow;
    sFuncCreateSQL += " \t\t" + m_sql + "\n \t" + sFlagName + " language plpgsql;\n";
    m_funcCreateSQL = sFuncCreateSQL;
    // 创建临时函数
    {
        PGresult* res = PQexec(&m_con, sFuncCreateSQL.c_str());
        CAutoRelease _auto([&](){ PQclear(res); });
        TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    }
    // 预执行临时函数的sql语句
    string sPerform = m_resultParmCount > 0 ? "select * from" : "call";
    string sExecSQL = sPerform + " pg_temp." + sProcName + "(";
    // 循环拼加实参（以$符号开头，+参数在sql语句中的位置）
    for (auto &parm : m_parms.mapParms)
        sExecSQL += "$" + std::to_string(parm.second->iPos) + ",";
    algorithm::trim_right_if(sExecSQL, algorithm::is_any_of(","));
    // 无参数时，加入占位形参
    // if (m_parms.mapParms.size() == 0) sExecSQL += "$1";
    sExecSQL += ");\n";
    m_execSQL = sExecSQL;
    sLogInfo += sExecSQL + c_strLineShow;
    // libpq的参数数组
    vector<unsigned int> vctParmOID(m_parms.mapParms.size());
    if (vctParmOID.size() > 0) vctParmOID[0] = VARCHAROID;
    // 循环按位置插入参数类型
    string sParmInfo = "Formal-Parameter:\n";
    for (auto &parm : m_parms.mapParms)
    {
        // 参数类型
        auto &ptrParmTp = vctParmOID[parm.second->iPos - 1];
        ptrParmTp = parm.second->Get<TParmPgSQL>().m_oid;
        sParmInfo += (format("$%d/%d \t") % parm.second->iPos % ptrParmTp).str();
    }
    sLogInfo += sParmInfo + c_strLineShow;
    // 预执行
    PGresult *res = PQprepare(&m_con, m_stmtName.c_str(), sExecSQL.c_str(), static_cast<int>(vctParmOID.size()), vctParmOID.data());
    TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    // 预执行保存点SQL
    PrepareBySavePoint();
    // 日志
    m_own.m_own.WriteLogTrace(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
}
// 预执行单语句
void CCtrlPgSQL::TDBCommandPgSQL::PrepareByOneSQL(void)
{
    string sLogInfo = "<R" + m_sExecSort + ".> Prepare By A SQL" + c_strLineShow + m_sql.substr(0, 1024) + c_strLineShow;
    // 参数类型
    vector<Oid> vctParmOID;
    if (!m_parms.mapParms.empty())
    {
        vctParmOID.resize(m_parms.mapParms.size());
        string sParmInfo = "Formal-Parameter:\n";
        // 循环每个参数
        for (auto &parm : m_parms.mapParms)
        {
            // c_RESTful_batch == m_method;
            // 参数类型
            auto &ptrParmTp = vctParmOID[parm.second->iPos - 1];
            ptrParmTp = parm.second->Get<TParmPgSQL>().m_oid;
            sParmInfo += (format("$%d/%d \t") % parm.second->iPos % ptrParmTp).str();
            // 替换sql语句里的参数
            if ("jna" == m_own.m_loadType)
            {
                string sPattern = ":" + parm.second->sName + "\\b";
                string sPrefix = "~@!#$%^&*<>a~";   // 将冒号替换成特殊前缀，然后再字符串替换。由于std::regex不支持“零宽度断言（负后顾断言）”或“反向否定预查”。

                m_sql = std::regex_replace(m_sql,
                                           m_own.m_parmCaseSensitive ? std::regex(sPattern) : std::regex(sPattern, std::regex::icase),
                                           sPrefix + std::to_string(parm.second->iPos));
                algorithm::replace_all(m_sql, sPrefix + std::to_string(parm.second->iPos), "$" + std::to_string(parm.second->iPos));
            }
            else
            {
                // boost::regex支持“零宽度断言（负后顾断言）”或“反向否定预查”。
                string sPattern = "(?<!:):" + parm.second->sName + "\\b";
                boost::regex pattern(sPattern, m_own.m_parmCaseSensitive ? boost::regex::nocollate : boost::regex::icase);
                m_sql = boost::regex_replace(m_sql, pattern, "\\$" + std::to_string(parm.second->iPos));
            }
        }
        m_execSQL = m_sql;
        sLogInfo += sParmInfo + "\r\n";
    }
    // 预执行
    PGresult *res = PQprepare(&m_con, m_stmtName.c_str(), m_sql.c_str(), static_cast<int>(vctParmOID.size()), vctParmOID.data());
    TLibPQException::CheckError<PGRES_COMMAND_OK>(res, m_sql, __CURR_CODE_PLACE_C__);
    // 获取参数类型（自动推导后）
    res = PQdescribePrepared(&m_con, m_stmtName.c_str());
    TLibPQException::CheckError<PGRES_COMMAND_OK>(res, m_sql, __CURR_CODE_PLACE_C__);
    string sParmInfo;
    for (int i = 0, c = PQnparams(res); i < c; ++i)
        sParmInfo += (format("$%d/%d \t") % (i + 1) % PQparamtype(res, i)).str();
    sLogInfo += sParmInfo + c_strLineShow;
    // 预执行保存点SQL
    PrepareBySavePoint();
    // 日志
    m_own.m_own.WriteLogTrace(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
}
// 预执行保存点SQL
void CCtrlPgSQL::TDBCommandPgSQL::PrepareBySavePoint(void)
{
    string sStmtName = m_stmtName + "_pnt";
    // 保存
    {
        string sSQL = "savepoint " + sStmtName;
        PGresult *res = PQprepare(&m_con, (sStmtName + "_save").c_str(), sSQL.c_str(), 0, nullptr);
        TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sSQL, __CURR_CODE_PLACE_C__);
    }
    // 释放
    {
        string sSQL = "release savepoint " + sStmtName;
        PGresult *res = PQprepare(&m_con, (sStmtName + "_free").c_str(), sSQL.c_str(), 0, nullptr);
        TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sSQL, __CURR_CODE_PLACE_C__);
    }
    // 回滚
    {
        string sSQL = "rollback to savepoint " + sStmtName;
        PGresult *res = PQprepare(&m_con, (sStmtName + "_roll").c_str(), sSQL.c_str(), 0, nullptr);
        TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sSQL, __CURR_CODE_PLACE_C__);
    }
}

// 批量操作
pair<int, string> CCtrlPgSQL::TDBCommandPgSQL::ExecuteBatch(void)
{
    try
    {
        // m_own.m_own.WriteLogTrace(("Insert Begin: " + std::to_string(m_batchIsProc)).c_str(), __CURR_CODE_PLACE_C__);
        unsigned iAffect = m_batchIsProc ? ExecuteProcedures(false) : ExecuteUpdate(false);
        // m_own.m_own.WriteLogTrace(("Insert End: " + std::to_string(m_batchIsProc)).c_str(), __CURR_CODE_PLACE_C__);
        // 获取返回参数
        int iErrCode = 0;
        string sErrMsg, sLogMsg;
        if (m_batchIsProc)
        {
            auto fGetParm = [&](string sName, std::function<void(boost::any)> fRes)
            {
                // 设置输出参数
                auto itOutParm = m_parms.mapOutParms.find(sName);
                if (m_parms.mapOutParms.end() != itOutParm)
                    fRes(itOutParm->second->anyVal);
            };
            fGetParm("errCode", [&](boost::any anyVal){ iErrCode = atoi(BoostAnyToStdString(anyVal).c_str()); });
            fGetParm("errMsg", [&](boost::any anyVal){ sErrMsg = BoostAnyToStdString(anyVal); });
            fGetParm("logMsg", [&](boost::any anyVal){ sLogMsg = BoostAnyToStdString(anyVal); });
            if (!sLogMsg.empty())
                m_own.m_own.WriteLogInfo((boost::format("ExecuteBatch: %s - %s\n%s") % m_kcFile % m_actName % sLogMsg).str().c_str(), __CURR_CODE_PLACE_C__);
        }
        if (0 != iErrCode) return make_pair(iErrCode, sErrMsg.empty() ? "批量插入错误" : sErrMsg);
        else return make_pair(iAffect, "");
    }
    catch (TLibPQException& ex)
    {
        return make_pair(ex.m_code, ex.m_msg);
    }
}

// 执行
unsigned int CCtrlPgSQL::TDBCommandPgSQL::ExecuteUpdate(void)
{
    return ExecuteUpdate(true);
}
unsigned int CCtrlPgSQL::TDBCommandPgSQL::ExecuteUpdate(bool outLog)
{
    string sLogInfo = "<U" + m_sExecSort + ".>";
    PGresult* res = UpdateOrQuery(sLogInfo);
    CAutoRelease _auto([&](){ try { PQclear(res); res = nullptr; } catch (...) {} });
    TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    if (outLog) m_own.m_own.WriteLogTrace(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
    return atoi(PQcmdTuples(res));
}

// 查询
CSqlFunc::TRecordSetPtr CCtrlPgSQL::TDBCommandPgSQL::ExecuteQuery(int &rows_affected)
{
    string sLogInfo = "<S" + m_sExecSort + ".>";
    PGresult* res = UpdateOrQuery(sLogInfo);
    CSqlFunc::TRecordSetPtr resPtr(new TRecordSetPgSQL(m_own, m_objCtrlD, *res));
    TLibPQException::CheckError<PGRES_TUPLES_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    //m_own.m_own.WriteLogDebug(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
    rows_affected = -1;
    return resPtr;
}

// 执行过程
int CCtrlPgSQL::TDBCommandPgSQL::ExecuteProcedures(void)
{
    return ExecuteProcedures(true);
}
int CCtrlPgSQL::TDBCommandPgSQL::ExecuteProcedures(bool outLog)
{
    /*
    string sLogInfo = c_strLineShow + m_sql + c_strLineShow;
    // 当前时间标识
    //string sNowTime = posix_time::to_iso_string(posix_time::second_clock::local_time());
    // 时间戳的base62编码
    string sNowTime = m_objCtrlD.ActionData().GetNowTimeFlag();
    // 文件名
    string sFilename = boost::filesystem::path(m_kcFile).filename().string();
    sFilename = algorithm::replace_all_copy(sFilename, ".", "_");
    // 临时函数名
    string sProcName = (format("_%s__%s_$%s") % m_actName % sFilename % sNowTime).str();
    // 创建临时函数的sql语句
    string sFunc = "create or replace function pg_temp." + sProcName + "(";
    // 循环拼加形参
    for (auto &parm : m_parms.mapParms)
    //for (auto iter = m_parms.mapParms.rbegin(); m_parms.mapParms.rend() != iter; ++iter)
    {
        //auto &parm = *iter;
        // 参数名
        string sParmName = (format("%s___%s_%d") % parm.second->sName % sNowTime % parm.second->iPos).str();
        // 拼加
        sFunc += "INOUT " + sParmName + " " + parm.second->Get<TParmPgSQL>().sDbTP + ",";

        // 替换sql语句中的参数名
//        if (m_own.m_parmCaseSensitive)
//            algorithm::replace_all(m_sql, ":" + parm.second->sName, sParmName);
//        else
//            algorithm::ireplace_all(m_sql, ":" + parm.second->sName, sParmName);
        algorithm::ireplace_all(sParmName, "$", "\\$");

        if ("jna" == m_own.m_loadType)
        {
            // std::regex不支持“零宽度断言”（负后顾断言）
            string sPattern = ":" + parm.second->sName + "\\b";
            if (m_own.m_parmCaseSensitive)
            {
                std::regex pattern(sPattern);
                m_sql = std::regex_replace(m_sql, pattern, sParmName);
            }
            else
            {
                std::regex pattern(sPattern, std::regex::icase);
                m_sql = std::regex_replace(m_sql, pattern, sParmName);
            }
        }
        else
        {
            string sPattern = "(?<!:):" + parm.second->sName + "\\b";
            boost::regex pattern(sPattern, m_own.m_parmCaseSensitive ? boost::regex::nocollate : boost::regex::icase);
            m_sql = boost::regex_replace(m_sql, pattern, sParmName);
        }
    }
    algorithm::trim_right_if(sFunc, algorithm::is_any_of(","));
    // 无参数时，加入占位参数
    if (m_parms.mapParms.size() == 0)
        sFunc += "INOUT __0_" + sNowTime + "__ varchar";
    // 返回值类型
    string sResultType = m_parms.mapParms.size() == 1 ? m_parms.mapParms.begin()->second->Get<TParmPgSQL>().sDbTP : (m_parms.mapParms.size() == 0 ? "varchar" : "record");
    // 添加sql语句体
    //sFunc += ") returns " + sResultType + " as $" + sProcName + "$ begin " + m_sql + "\nend $" + sProcName + "$ language plpgsql;\n";
    string sFlagName = "$" + algorithm::replace_all_copy(sProcName, "$", "") + "$";
    sFunc += ") returns " + sResultType + " as " + sFlagName + " begin " + m_sql + "\nend " + sFlagName + " language plpgsql;\n";
    sLogInfo += sFunc + c_strLineShow;
    // 创建临时函数
    {
        PGresult* res = PQexec(&m_con, sFunc.c_str());
        CAutoRelease _auto([&](){ PQclear(res); });
        TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    }
    // 执行临时函数的sql语句
    string sExecSQL = "select * from pg_temp." + sProcName + "(";
    // 循环拼加实参（以$符号开头，+参数在sql语句中的位置）
    for (auto &parm : m_parms.mapParms)
        sExecSQL += "$" + lexical_cast<string>(parm.second->iPos) + ",";
    algorithm::trim_right_if(sExecSQL, algorithm::is_any_of(","));
    // 无参数时，加入占位形参
    if (m_parms.mapParms.size() == 0)
        sExecSQL += "$1";
    sExecSQL += ");\n";
    sLogInfo += sExecSQL + c_strLineShow;
    // 执行临时函数
    int iResult = 0;
    {
        // libpq的参数数组
        vector<unsigned int> vctParmOID(max(static_cast<int>(m_parms.mapParms.size()), 1));
        vctParmOID[0] = VARCHAROID;
        vector<const char*> vctParm(max(static_cast<int>(m_parms.mapParms.size()), 1));
        vctParm[0] = "";
        // 循环按位置插入参数值
        string sParmInfo = "argv:\n";
        for (auto &parm : m_parms.mapParms)
        {
            // 参数类型
            auto &ptrParmTp = vctParmOID[parm.second->iPos - 1];
            ptrParmTp = parm.second->Get<TParmPgSQL>().m_oid;
            // 设置参数值
            auto &ptrParm = vctParm[parm.second->iPos - 1];
            if (parm.second->IsNull() || ((CSqlFunc::EDataType::edtInt == parm.second->eDTp || CSqlFunc::EDataType::edtNumber == parm.second->eDTp) && parm.second->sVal.empty()))
                ptrParm = nullptr;
            else
                ptrParm = parm.second->sVal.c_str();
            // 提示
            if (nullptr == ptrParm)
                sParmInfo += (format("$%d = null\n") % parm.second->iPos).str();
            else
                sParmInfo += (format("$%d = '%s'\n") % parm.second->iPos % ptrParm).str();
            // algorithm::replace_all(m_sql, ":" + parm.second->sName, "$" + lexical_cast<string>(parm.second->iPos));
        }
        sLogInfo += sParmInfo + c_strLineShow;
        // 带入参数执行
        PGresult* res = PQexecParams(&m_con, sExecSQL.c_str(), static_cast<int>(vctParm.size()), &vctParmOID[0], &vctParm[0], nullptr, nullptr, 0);
        CAutoRelease _auto([&](){ PQclear(res); });
        // 检查错误
        TLibPQException::CheckError<PGRES_TUPLES_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
        iResult = atoi(PQcmdTuples(res));
        // 获取返回参数值，循环更新到参数映射里
        int iSort = 0;
        for (auto &parm : m_parms.mapParms)
        {
            parm.second->bIsNull = PQgetisnull(res, 0, iSort);
            if (parm.second->bIsNull) parm.second->sVal.clear();
            else
            {
                auto pVal = PQgetvalue(res, 0, iSort);
                parm.second->sVal = nullptr != pVal ? pVal : "";
            }
            // 设置输出参数
            auto itOutParm = m_parms.mapOutParms.find(parm.second->sName);
            if (m_parms.mapOutParms.end() != itOutParm)
            {
                itOutParm->second->bIsNull = parm.second->bIsNull;
                itOutParm->second->sVal = parm.second->sVal;
            }
            // 下一个序号
            ++iSort;
        }
    }
    //m_own.m_own.WriteLogDebug(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
    return iResult;
    */

    string sLogInfo = "<P" + m_sExecSort + ".>";
    PGresult* res = UpdateOrQuery(sLogInfo);
    string sSqlCmdType = CUtilFunc::PCharSafeToStr(PQcmdStatus(res));
    sSqlCmdType = boost::algorithm::to_lower_copy(sSqlCmdType.substr(0, strlen(c_RESTful_select)));
    CAutoRelease _auto([&](){ try { PQclear(res); res = nullptr; } catch (...) {} });
    if (c_RESTful_select != sSqlCmdType)
        TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    else
        TLibPQException::CheckError<PGRES_TUPLES_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    // 获取返回参数值，循环更新到参数映射里
    int iSort = 0;
    for (auto &parm : m_parms.mapParms)
    {
        // 是否输出参数
        const bool bIsOutParm = m_parms.mapOutParms.end() != m_parms.mapOutParms.find(parm.second->sName) || CSqlFunc::EParmType::eptSessionParm == parm.second->ePTp;
        if (bIsOutParm)
        {
            // const auto iPgParmCnt = PQnparams(res);
            // if (iSort >= iPgParmCnt)
            // {
            //     string sErrMsg = (boost::format("输出参数[%s]越界：%d <= %d") % parm.second->sName % iPgParmCnt % iSort).str();
            //     cout << sErrMsg << endl;
            //     m_own.m_own.WriteLogTrace(sErrMsg.c_str(), __CURR_CODE_PLACE_C__);
            //     break;
            // }
            parm.second->bIsNull = PQgetisnull(res, 0, iSort);
            if (parm.second->bIsNull) parm.second->anyVal.clear();
            else
            {
                auto pVal = PQgetvalue(res, 0, iSort);
                // parm.second->sVal = nullptr != pVal ? pVal : "";
                // 按类型赋值
                if (CSqlFunc::EDataType::edtInt == parm.second->eDTp) parm.second->anyVal = atoi(pVal);
                else if (CSqlFunc::EDataType::edtNumber == parm.second->eDTp) parm.second->anyVal = atof(pVal);
                else parm.second->anyVal = CUtilFunc::PCharSafeToStr(pVal);
            }
            // 设置输出参数
            auto itOutParm = m_parms.mapOutParms.find(parm.second->sName);
            if (m_parms.mapOutParms.end() != itOutParm)
            {
                itOutParm->second->bIsNull = parm.second->bIsNull;
                itOutParm->second->anyVal = parm.second->anyVal;
            }
            // 下一个序号
            ++iSort;
        }
    }
    if (outLog) m_own.m_own.WriteLogTrace(sLogInfo.c_str(), __CURR_CODE_PLACE_C__);
    int iAffect = atoi(PQcmdTuples(res));
    if (c_RESTful_batch == m_method && 0 == iAffect) iAffect = 1;
    return iAffect;
}

// 执行或查询
PGresult* CCtrlPgSQL::TDBCommandPgSQL::UpdateOrQuery(string& sLogInfo)
{
    PGresult* res = nullptr;
    // // 无参数
    // if (m_parms.mapParms.empty())
    // {
    //     res = PQexec(&m_con, m_sql.c_str());
    //     // cout << m_sql << endl;
    // }
    // // 有参数

    // DDL语句（创建函数、存储过程等）
    if (c_RESTful_sql_DDL == this->GetParms().sSQLWorkType)
        res = PQexec(&m_con, m_sql.c_str());
    // DML、DQL（增、删、改、查、执行过程等）语句
    else
    {
        string sParmInfo = "argv:\n";
        // 参数
        // vector<unsigned int> vctParmOID;
        vector<string> vctParmStr;
        vector<const char*> vctParm;
        if (!m_parms.mapParms.empty())
        {
            // vctParmOID.resize(m_parms.mapParms.size());
            const auto iSize = m_parms.mapParms.size();
            vctParm.resize(iSize);
            vctParmStr.resize(iSize);
            // 循环每个参数
            for (auto &parm : m_parms.mapParms)
            {
                // 参数类型
                // auto &ptrParmTp = vctParmOID[parm.second->iPos - 1];
                // ptrParmTp = parm.second->Get<TParmPgSQL>().m_oid;

                // 设置参数值
                auto &ptrParm = vctParm[parm.second->iPos - 1];
                auto &ptrParmStr = vctParmStr[parm.second->iPos - 1];
                if (parm.second->IsNull() || parm.second->anyVal.empty())
                    ptrParm = nullptr;
                else
                {
                    ptrParmStr = BoostAnyToStdString(parm.second->anyVal);
                    ptrParm = ptrParmStr.c_str();
                }
                sParmInfo += (format("$%d = '%s'\n") % parm.second->iPos % CUtilFunc::PCharSafeToStr(vctParm[parm.second->iPos - 1])).str();

                /*
                // 替换sql语句里的参数
                if (m_own.m_parmCaseSensitive)
                   algorithm::replace_all(m_sql, ":" + parm.second->sName, "$" + lexical_cast<string>(parm.second->iPos));
                else
                   algorithm::ireplace_all(m_sql, ":" + parm.second->sName, "$" + lexical_cast<string>(parm.second->iPos));

                // 替换sql语句里的参数格式
                if ("jna" == m_own.m_loadType)
                {
                    string sPattern = ":" + parm.second->sName + "\\b";
                    string sPrefix = "~@!#$%^&*<>a~";   // 将冒号替换成特殊前缀，然后再字符串替换。由于std::regex不支持“零宽度断言（负后顾断言）”或“反向否定预查”。
                    if (m_own.m_parmCaseSensitive)
                    {
                        std::regex pattern(sPattern);
                        m_sql = std::regex_replace(m_sql, pattern, sPrefix + std::to_string(parm.second->iPos));
                    }
                    else
                    {
                        std::regex pattern(sPattern, std::regex::icase);
                        m_sql = std::regex_replace(m_sql, pattern, sPrefix + std::to_string(parm.second->iPos));
                    }
                    algorithm::replace_all(m_sql, sPrefix + std::to_string(parm.second->iPos), "$" + std::to_string(parm.second->iPos));
                }
                else
                {
                    // boost::regex支持“零宽度断言（负后顾断言）”或“反向否定预查”。
                    string sPattern = "(?<!:):" + parm.second->sName + "\\b";
                    boost::regex pattern(sPattern, m_own.m_parmCaseSensitive ? boost::regex::nocollate : boost::regex::icase);
                    m_sql = boost::regex_replace(m_sql, pattern, "\\$" + lexical_cast<string>(parm.second->iPos));
                }
                */
            }
        }
        // 执行
        sLogInfo += c_strLineShow + sParmInfo + c_strLineShow;
        // m_own.m_own.WriteLogTrace("UpdateOrQuery Begin.", __CURR_CODE_PLACE_C__);
        // res = PQexecParams(&m_con, m_sql.c_str(), static_cast<int>(vctParm.size()), &vctParmOID[0], &vctParm[0], nullptr, nullptr, 0);
        res = PQexecPrepared(&m_con, m_stmtName.c_str(), static_cast<int>(vctParm.size()), vctParm.data(), nullptr, nullptr, 0);
        sLogInfo += c_strLineShow + CUtilFunc::PCharSafeToStr(PQresStatus(PQresultStatus(res))) + ": " + CUtilFunc::PCharSafeToStr(PQcmdStatus(res)) + c_strLineShow;
        // m_own.m_own.WriteLogTrace("UpdateOrQuery End.", __CURR_CODE_PLACE_C__);
        // cout << m_sql << endl << sParmInfo << endl;
    }
    return res;
}

// 调用临时函数
void CCtrlPgSQL::TDBCommandPgSQL::PerformFunc(void)
{
    // 创建
    CreateDDL();
    //cout << "perform：select pg_temp.subMod(v_mod=>1, i=>0);" << endl << m_sql << endl;
    // 解析函数头
    string sFuncName, sParms, sRetType;
    {
        string sFunc = CCtrlCommon::RemoveCommentAndStr(m_sql);
        string sFmt = R"(\bcreate\s+(?:or\s+replace\s+)?function\s+pg_temp\s*\.\s*(\w+)\s*\(([\w|\s|,]*)\)\s*returns\s+(?!void\b)(\w+)\s*(?:\([\w|\s|,]*\))?\s*\bas\b)";
        std::regex pattern(sFmt, std::regex::icase);
        std::smatch what;
        if(!std::regex_search(sFunc, what, pattern))
            throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 10, string(m_own.m_own.getHint("Request_Failed_")) + m_own.m_own.getHint("Need_Func_Return_Value"), "", __CURR_CODE_PLACE_C__);
        //for(unsigned i=1;i<what.size();i++) cout<<"str: "<<what[i].str()<<endl;
        if(what.size() != 4 || what[1].str().empty() || what[3].str().empty())
        {
            string sMsg;
            for(unsigned i = 1; i < what.size(); ++i) sMsg += what[i].str() + c_strLineShow;
            throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 11, string(m_own.m_own.getHint("Request_Failed_")) + m_own.m_own.getHint("Need_Func_Return_Value"), sMsg, __CURR_CODE_PLACE_C__);
        }
        sFuncName = what[1].str();
        sParms = what[2].str();
        sRetType = algorithm::to_lower_copy(what[3].str());
        cout << sFuncName << "(" << sParms << ")" << sRetType << endl;
    }
    // 解析形参
    std::vector<std::pair<string, string>> vctParms;
    {
        string sFmt = R"(\w+\s+\w+\s*,?)";
        std::regex pattern(sFmt, std::regex::icase);
        // 循环所有参数
        for (std::sregex_iterator it(sParms.cbegin(), sParms.cend(), pattern), end_it; end_it != it; ++it)
        {
            std::string sParm = algorithm::trim_copy(it->str());
            algorithm::trim_right_if(sParm, algorithm::is_any_of(","));
            algorithm::trim_right(sParm);
            //cout << "parm: " << sParm << ";" << endl;
            // 分割参数名和类型
            vector<string> vctParm;
            algorithm::split_regex(vctParm, sParm, boost::regex(R"(\s+)"));
            if (vctParm.size() != 2)
            {
                string sMsg = sFuncName + "(" + sParms + ")" + sRetType + c_strLineShow + sParm + c_strLineShow;
                throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 12, string(m_own.m_own.getHint("Request_Failed_")) + m_own.m_own.getHint("Need_Func_Return_Value"), sMsg, __CURR_CODE_PLACE_C__);
            }
            vctParms.push_back(make_pair(vctParm[0], algorithm::to_lower_copy(vctParm[1])));
        }
    }
    // 执行函数
    string sSQL = (boost::format("select%s pg_temp.%s(") % ("table" == sRetType ? " * from" : "") % sFuncName).str();
    for (size_t i = 0, c = vctParms.size(); i < c; ++i)
        sSQL += "$" + std::to_string(i + 1) + ",";
    algorithm::trim_right_if(sSQL, algorithm::is_any_of(","));
    sSQL += string(")") + ("table" == sRetType ? "" : " c");
    cout << sSQL << endl;
    string sLogInfo = string(66, '-') + "\n" + sSQL + "\n" + string(66, '-');
    PGresult* res = nullptr;
    // 无参数
    if (vctParms.empty())
        res = PQexec(&m_con, sSQL.c_str());
    // 有参数
    else
    {
        // 参数
        vector<unsigned int> pgVctParmOID(vctParms.size());
        vector<string> pgVctParmStr(vctParms.size());
        vector<const char*> pgVctParm(vctParms.size());
        // 循环每个参数
        for (size_t i = 0, c = vctParms.size(); i < c; ++i)
        {
            auto &prm = vctParms[i];
            // 参数类型
            pgVctParmOID[i] = CCtrlPgSQL::GetOIDByTp(prm.second);
            // 参数值
            auto &itmVal = m_objCtrlD.JsonRequest().GetItem(prm.first.c_str());
            if (!itmVal.IsValid())
                throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 13, string(m_own.m_own.getHint("Parm_Error_")) + prm.first, sLogInfo, __CURR_CODE_PLACE_C__);
            pgVctParmStr[i] = itmVal.IsNull() ? nullptr : itmVal.GetStr();
            pgVctParm[i] = pgVctParmStr[i].c_str();
            string sStrTp = (VARCHAROID == pgVctParmOID[i] || DATEOID == pgVctParmOID[i] || TIMESTAMPOID == pgVctParmOID[i] || TIMESTAMPTZOID == pgVctParmOID[i]) ? "'" : "";
            sLogInfo += "\n" + prm.first + " = " + sStrTp + (itmVal.IsNull() ? "null" : itmVal.GetStr()) + sStrTp;
        }
        sLogInfo += "\n" + string(66, '-') + "\n";
        res = PQexecParams(&m_con, sSQL.c_str(), static_cast<int>(pgVctParm.size()), &pgVctParmOID[0], &pgVctParm[0], nullptr, nullptr, 0);
    }
    // 结果集
    CSqlFunc::TRecordSetPtr resPtr(new TRecordSetPgSQL(m_own, m_objCtrlD, *res));
    // 检查错误
    TLibPQException::CheckError<PGRES_TUPLES_OK>(res, sLogInfo, __CURR_CODE_PLACE_C__);
    // 字段列表
    CSqlFunc::TFields vecFields;
    resPtr->GetColumnListMetaData(vecFields);
    /// 子控制器调用时，返回结果可能影响主控制器
    // 返回字符串
    auto fOutStr = [&](string str = "")
    {
        auto &parmRespond = m_objCtrlD.JsonRespond().ParmInOut();
        parmRespond.SetContentType(c_DownTxtFileResponseContentType);
        parmRespond.SetContent(str.c_str(), static_cast<unsigned>(str.size()));
    };
    // 返回json格式
    if ("json" == sRetType || "jsonb" == sRetType)
    {
        if (vecFields.size() != 1)
            throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 14, string(m_own.m_own.getHint("Request_Failed_")) + m_own.m_own.getHint("Need_Func_Return_Value"), "", __CURR_CODE_PLACE_C__);
        CSqlFunc::TField& fld = *vecFields[0];
        // 返回结果
        if (fld.m_pos > 0 && PQgetisnull(res, 0, fld.m_pos - 1) != 1)
        {
            auto strVal = PQgetvalue(res, 0, fld.m_pos - 1);
            m_objCtrlD.JsonRespond().ResetByStr(strVal);
        }
        else fOutStr();
    }
    // 返回table格式
    else if ("table" == sRetType)
    {
        m_objCtrlD.JsonRespond().ResetByStr("[]");
        resPtr->DataSetToJsonArray(m_objCtrlD.JsonRespond());
    }
    // 返回字符串格式
    else
    {
        if (vecFields.size() != 1)
            throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 14, string(m_own.m_own.getHint("Request_Failed_")) + m_own.m_own.getHint("Need_Func_Return_Value"), "", __CURR_CODE_PLACE_C__);
        CSqlFunc::TField& fld = *vecFields[0];
        // 返回结果
        if (fld.m_pos > 0 && PQgetisnull(res, 0, fld.m_pos - 1) != 1)
            fOutStr(PQgetvalue(res, 0, fld.m_pos - 1));
        else fOutStr();
    }
}

// postgresql特有的批量数据导入
void CCtrlPgSQL::TDBCommandPgSQL::CopyFromStdinByCsv(void)
{
    // 用时统计
    const long long iNowTotal = CUtilFunc::GetCurrentStampMS();
    int iUseTimeTotal = 0, iUseTimeGetDbSet = 0, iUseTimeGetLineVal = 0, iUseTimeCopyData = 0, iUseTimeCopyLine = 0, iUseTimeCommitCopy = 0, iUseTimeCopyWait = 0;
    int iSum = 0, iCopyStatus = 0;
    string sCopyError;
    unsigned iCount = 0;
    // 执行日志
    string sMsg = "<C" + m_sExecSort + ".> Copy Begin. [" + m_objCtrlD.LocalFile() + " : " + m_objCtrlD.NodeName() + "]";
    m_own.m_own.WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
    cout << sMsg << endl;
    CAutoRelease _auto([&](){
        iUseTimeTotal = static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowTotal);
        string sMsg = (boost::format("<C%d.> Copy End. Count = %d, Sum = %d, Affect = %d. UseTimeTotal = %d, UseTimeGetDbSet = %d, UseTimeGetLineVal = %d, UseTimeCopyData = %d, UseTimeCopyLine = %d, UseTimeCommitCopy = %d, UseTimeCopyWait = %d.\n%s")
                       % m_execSort % iCount % iSum % iCopyStatus % iUseTimeTotal % iUseTimeGetDbSet % iUseTimeGetLineVal % iUseTimeCopyData % iUseTimeCopyLine % iUseTimeCommitCopy % iUseTimeCopyWait % sCopyError).str();
        m_own.m_own.WriteLogTrace(sMsg.c_str(), __CURR_CODE_PLACE_C__);
        cout << sMsg << endl;
        // 返回
        m_objCtrlD.JsonRespond().SetVal(c_RESTful_errCode, iCount > 0 ? 0 : ecd_ErrCode_KCSqlPostgresql + 5);
        if (iCount > 0)
            m_objCtrlD.JsonRespond().SetStr(c_RESTful_errMsg, (boost::format("[Controllers.%s] %d/%d|%d [%d]") % m_actName % iCount % iSum % iCopyStatus % iUseTimeTotal).str().c_str());
        else
            m_objCtrlD.JsonRespond().SetStr(c_RESTful_errMsg, (boost::format("[Controllers.%s] No Data") % m_actName).str().c_str());
    });
    // 附加参数
    string sCopyFeildsParm = CUtilFunc::PCharSafeToPChar(m_objCtrlD.JsonAttach().GetItem("copyParm").GetStr("copyFeildsParm", ""));
    string sCopyValsParm = CUtilFunc::PCharSafeToPChar(m_objCtrlD.JsonAttach().GetItem("copyParm").GetStr("copyValsParm", ""));
    string sSrcDbSetName = !sCopyValsParm.empty() ? sCopyValsParm : m_objCtrlD.ActionData().GetValsName();
    string sToTable = CUtilFunc::PCharSafeToPChar(m_objCtrlD.JsonAttach().GetItem("copyParm").GetStr("toTable", ""));
    string sTableName = !sToTable.empty() ? sToTable : sSrcDbSetName;
    string sNowTime = m_objCtrlD.ActionData().GetNowTimeFlag();
    string sTableAlias = m_own.m_dynTmpTableName && sToTable.empty() ? "_" + sTableName + "__" + sNowTime : sTableName;
    int iQueSize = std::max(atoi(m_objCtrlD.JsonAttach().GetItem("copyParm").GetStr("queueSize", "2048")), 1024);
    // 删除临时表
    auto fDropTmpTable = [&]()
    {
        // string sDropSQL = (boost::format(R"(
        //     DO $$ begin
        //         IF NOT to_regclass('%1%') IS NULL THEN
        //             DROP TABLE %1%;
        //         END IF;
        //     END; $$
        // )") % sTableAlias).str();
        string sDropSQL = (boost::format(R"(DROP TABLE IF EXISTS %s)") % sTableAlias).str();
        PGresult* res = PQexec(&m_con, sDropSQL.c_str());
        CAutoRelease _auto([&](){ PQclear(res); });
        TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sDropSQL, __CURR_CODE_PLACE_C__);
    };
    fDropTmpTable();
    // 未指定表，创建临时表
    auto fCtreateTmpTable = [&](std::function<void(string&)> fDynMakeFeilds)
    {
        if (sToTable.empty())
        {
            // 创建临时表的sql语句
            string sCreateTmpTabSQL = "CREATE TEMP TABLE " + sTableAlias + " (";
            // 指定表结构
            if (!sCopyFeildsParm.empty())
            {
                const IKCJson& jsonFeilds = m_objCtrlD.GetParmJson(sCopyFeildsParm.c_str());
                if (!jsonFeilds.IsValid())
                    throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 14, string(m_own.m_own.getHint("Request_Failed_")) + m_own.m_own.getHint("Don_t_exists_field_") + sCopyFeildsParm, "", __CURR_CODE_PLACE_C__);
                jsonFeilds.InitItemNextStep();
                for (unsigned i = 0, c = jsonFeilds.GetItemCount(); i < c; ++i)
                {
                    auto &jsnFeild = jsonFeilds.GetItemNext();
                    string sFeildName = jsnFeild.GetName();
                    string sFeildType = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(jsnFeild.GetStr()));
                    sCreateTmpTabSQL += sFeildName + " " + sFeildType + ",";
                }
            }
            // 未指定表结构
            else
                fDynMakeFeilds(sCreateTmpTabSQL);
            boost::algorithm::trim_right_if(sCreateTmpTabSQL, boost::is_any_of(","));
            sCreateTmpTabSQL += ")";
            // 执行创建表的sql
            PGresult* res = PQexec(&m_con, sCreateTmpTabSQL.c_str());
            CAutoRelease _auto([&](){ PQclear(res); });
            TLibPQException::CheckError<PGRES_COMMAND_OK>(res, sCreateTmpTabSQL, __CURR_CODE_PLACE_C__);
            // 日志
            m_own.m_own.WriteLogTrace(("<C" + m_sExecSort + ".> 创建临时表：" + sCreateTmpTabSQL).c_str(), __CURR_CODE_PLACE_C__);
        }
    };
    // 执行从输入流拷贝数据
    auto fExecCopy = [&](void)
    {
        string sSQL = "COPY " + sTableAlias + " FROM STDIN WITH (FORMAT csv)";
        PGresult* res = PQexec(&m_con, sSQL.c_str());
        CAutoRelease _auto([&](){ PQclear(res); });
        TLibPQException::CheckError<PGRES_COPY_IN>(res, sSQL, __CURR_CODE_PLACE_C__);
    };
    // 拷贝CSV一行数据
    auto fExecCopyCSVLineData = [&](string sCsvLine)
    {
        const long long iNowIn = CUtilFunc::GetCurrentStampMS();
        CAutoRelease _auto([&](){ iUseTimeCopyLine += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
        // sCsvLine = boost::algorithm::trim_right_copy_if(sCsvLine, boost::is_any_of(",")) + "\n";
        sCsvLine = sCsvLine.substr(0, sCsvLine.size() - 1) + "\n";
        iSum += PQputCopyData(&m_con, sCsvLine.data(), static_cast<int>(sCsvLine.size()));
    };
    // 提交拷贝
    PGresult* resCommit = nullptr;
    CAutoRelease _autoClsResCommit([&](){ if (nullptr != resCommit) PQclear(resCommit); });
    auto fExecCopyCommit = [&](void)
    {
        const long long iNowIn = CUtilFunc::GetCurrentStampMS();
        CAutoRelease _auto([&](){ iUseTimeCommitCopy += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
        // 提交
        iCopyStatus = PQputCopyEnd(&m_con, nullptr);
        if (-1 == iCopyStatus) sCopyError = CUtilFunc::PCharSafeToStr(PQerrorMessage(&m_con));
        resCommit = PQgetResult(&m_con);
    };
    // 将字符串插入CSV行
    auto fAddStrToLine = [&](string& sCsvLine, string strVal)
    {
        if (strVal.find("\"") != string::npos) boost::algorithm::replace_all(strVal, "\"", "\"\"");
        if (strVal.find(",") != string::npos || strVal.find("\"") != string::npos || strVal.find("\n") != string::npos || strVal.find("\r") != string::npos)
            strVal = "\"" + strVal + "\"";
        sCsvLine += strVal + ",";
    };
    // 等待（并且挂起线程）
    auto fWaitAndYieldThrd = [&](void)
    {
        const long long iNowIn = CUtilFunc::GetCurrentStampMS();
        CAutoRelease _auto([&](){ iUseTimeCopyWait += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
        std::this_thread::yield();
    };

    // 从json批量数组参数里取
    auto fFromJson = [&](void)
    {
        // 批量参数json数组
        const IKCJson& jsonVals = m_objCtrlD.GetBatchArrayJson(sCopyValsParm.c_str());
        jsonVals.GC();
        // 数据数量
        iCount = jsonVals.GetItemCount();
        if (iCount > 0)
        {
            // 未指定表，创建临时表
            fCtreateTmpTable([&](string& sCreateTmpTabSQL){
                // 取第1条记录的结构
                auto &jsnFirst = jsonVals.GetItem(static_cast<unsigned>(0));
                jsnFirst.InitItemNextStep();
                // 循环每个字段
                for (unsigned i = 0, c = jsnFirst.GetItemCount(); i < c; ++i)
                {
                    // auto &jsnFeild = jsnFirst.GetItem(i);
                    auto &jsnFeild = jsnFirst.GetItemNext();
                    string sFeildName = jsnFeild.GetName();
                    string sFeildType = jsnFeild.GetType() == IKCJson::edtNumber ? "numeric" : "varchar";
                    sCreateTmpTabSQL += sFeildName + " " + sFeildType + ",";
                }
            });
            // 执行从输入流拷贝数据
            fExecCopy();
            // 循环向输入流插入每行数据，csv格式
            {
                const long long iNowIn = CUtilFunc::GetCurrentStampMS();
                CAutoRelease _autoTM([&](){ iUseTimeGetDbSet += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
                jsonVals.InitItemNextStep();
                // 提交拷贝
                CAutoRelease _auto([&](){ fExecCopyCommit(); });
                // 循环拷贝每行
                for (unsigned iLoop = 0; iLoop < iCount; ++iLoop)
                {
                    string sCsvLine;
                    {
                        const long long iNowIn = CUtilFunc::GetCurrentStampMS();
                        CAutoRelease _auto([&](){ iUseTimeGetLineVal += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
                        const IKCJson& jsonVal = jsonVals.GetItemNext();
                        jsonVal.InitItemNextStep();
                        // 循环每个字段
                        for (unsigned i = 0, c = jsonVal.GetItemCount(); i < c; ++i)
                        {
                            // auto &jsnFeildVal = jsonVal.GetItem(i);
                            auto &jsnFeildVal = jsonVal.GetItemNext();
                            // 数值类型
                            if (jsnFeildVal.GetType() == IKCJson::edtNumber)
                            {
                                auto fVal = jsnFeildVal.GetVal(0);
                                sCsvLine += (CUtilFunc::isLLong(fVal) ? std::to_string(static_cast<long long>(fVal)) : std::to_string(fVal)) + ",";
                            }
                            // 字符串类型
                            else
                            {
                                string strVal = CUtilFunc::PCharSafeToStr(jsnFeildVal.GetStr(""));
                                fAddStrToLine(sCsvLine, strVal);
                            }
                        }
                    }
                    fExecCopyCSVLineData(sCsvLine);
                }
            }
        }
    };
    // 从内部数据集里取
    auto fFromInner = [&](CSqlFunc::TRecordSetPtr dbSet)
    {
        // 标记有数据
        std::atomic_bool bHaveData = true;
        // 行数据队列
        boost::lockfree::spsc_queue<string> queLineCopy(iQueSize);
        CAutoRelease _autoClean([&](){ queLineCopy.consume_all([](const string&){}); });
        // 拷贝行队列的线程
        auto fCopyLineQueue = [&]()
        {
            const long long iNowIn = CUtilFunc::GetCurrentStampMS();
            CAutoRelease _auto([&](){ iUseTimeCopyData += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
            for (int iReadSize = 0; bHaveData || iReadSize > 0; iReadSize = queLineCopy.read_available())
            {
                try
                {
                    if (iReadSize > 0)
                    {
                        std::vector<string> strLinsList(iReadSize);
                        int iRealSize = queLineCopy.pop(strLinsList.data(), strLinsList.size());
                        for (int i = 0; i < iRealSize; ++i)
                            fExecCopyCSVLineData(strLinsList[i]);
                    }
                    else fWaitAndYieldThrd();
                }
                catch (...) { fWaitAndYieldThrd(); }
            }
        };
        std::thread thrdCopyLneQue(fCopyLineQueue);
        // 请求的字段
        CSqlFunc::TFields vecFields;
        // 最终提交拷贝
        CAutoRelease _auto([&](){ fExecCopyCommit(); });
        // 数据集输出到队列
        {
            const long long iNowIn = CUtilFunc::GetCurrentStampMS();
            CAutoRelease _auto([&](){ iUseTimeGetDbSet += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
            // 退出时，标记插入队列结束（已无数据）
            CAutoRelease __insertEnd([&](){ bHaveData = false; });
            // 开始插入队列
            iCount = dbSet->DataSetToOut(vecFields,
                [&](){
                    // 未指定表，创建临时表
                    fCtreateTmpTable([&](string& sCreateTmpTabSQL){
                        // 循环每个字段
                        for (auto fFeild : vecFields)
                        {
                            string sFeildName = fFeild->m_name;
                            string sFeildType = fFeild->GetTypeName();
                            sCreateTmpTabSQL += sFeildName + " " + sFeildType + ",";
                        }
                    });
                    // 执行从输入流拷贝数据
                    fExecCopy();
                }, [&](int, int){
                    const long long iNowIn = CUtilFunc::GetCurrentStampMS();
                    CAutoRelease _auto([&](){ iUseTimeGetLineVal += static_cast<int>(CUtilFunc::GetCurrentStampMS() - iNowIn); });
                    // 添加行记录
                    string sCsvLine;
                    for (auto &fFeild : vecFields)
                    {
                        CSqlFunc::TKcValVariant val = dbSet->GetVal(*fFeild);
                        if (val.empty()) sCsvLine += ",";
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<int>(val))
                            sCsvLine += std::to_string(*ptr) + ",";
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<long long>(val))
                            sCsvLine += std::to_string(*ptr) + ",";
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<double>(val))
                            sCsvLine += std::to_string(*ptr) + ",";
                        else if (const auto* ptr = CSqlFunc::KCVariantAnyCastPtr<string>(val))
                            fAddStrToLine(sCsvLine, *ptr);
                        else fAddStrToLine(sCsvLine, BoostAnyToStdString(val));
                    }
                    // 插入队列
                    while (queLineCopy.write_available() == 0 || !queLineCopy.push(sCsvLine))
                        std::this_thread::yield();
                    return true;
                });
        }
        // 等待拷贝结束
        if (thrdCopyLneQue.joinable()) thrdCopyLneQue.join();
    };

    // 在上一个控制器的内部数据集里找
    CSqlFunc::ICtrlApiDataWithDbSet* pCtrl = dynamic_cast<CSqlFunc::ICtrlApiDataWithDbSet*>(&m_objCtrlD);
    CSqlFunc::TRecordSetPtr lastInnerDbSet;
    if (nullptr != pCtrl)
        lastInnerDbSet = pCtrl->DataSetInnerLast(sSrcDbSetName);
    // 从json批量数组参数里取
    if (lastInnerDbSet.get() == nullptr) fFromJson();
    // 从内部数据集里取
    else fFromInner(lastInnerDbSet);
    // 检查提交失败
    TLibPQException::CheckError<PGRES_COMMAND_OK>(resCommit, sCopyError, __CURR_CODE_PLACE_C__);
}

// CCtrlPgSQL::TPgConnObj 类
CCtrlPgSQL::TPgConnObj::TPgConnObj(CCtrlPgSQL& own) : m_conn(PQconnectdb(own.m_connStr.c_str()))
{
    if (PQstatus(m_conn) == CONNECTION_BAD)
    {
        string sPgErr = algorithm::trim_copy(CUtilFunc::PCharSafeToStr(PQerrorMessage(m_conn)));
        if (!sPgErr.empty())
            sPgErr += "   [ToUtf8::   " + CUtilFunc::GbkToUtf8(sPgErr) + "].";
        else sPgErr = "postgresql连接错误 - CONNECTION_BAD";
        own.m_own.WriteLogError(sPgErr.c_str(), __CURR_CODE_PLACE_C__, own.m_connStr.c_str());
    }
}
void CCtrlPgSQL::TPgConnObj::Release(void)
{
    PQfinish(m_conn);
    delete this;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlPgSQL类
CCtrlPgSQL::CCtrlPgSQL(IKCSqlPgSQL& own, string sName, property_tree::ptree& pt)
    : m_own(own), m_name(sName), m_loadType(CUtilFunc::StrToLower(CUtilFunc::PCharSafeToStr(m_own.getContext().GetSysFlag("loadType"))))
{
    // 记录函数地址
    m_own.WriteLogDebug((m_name + ": "
                            + "\nCCtrlPgSQL::Exce: \t\t" + CUtilFunc::GetFuncAddr(&CCtrlPgSQL::Perform)
                       ).c_str());
    // 连接串
    if (pt.get_child_optional("<xmlattr>.str"))
        m_connStr = pt.get<string>("<xmlattr>.str");
    // 是否加密连接串
    if (pt.get_child_optional("<xmlattr>.encrypt"))
    {
        m_encryptConnStr = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.encrypt")) == "true";
        if (m_encryptConnStr)
            m_connStr = CCtrlCommon::DecryptConnStrDB(m_own.getContext(), m_connStr);
    }
    // 参数名称是否区分大小写
    if (pt.get_child_optional("<xmlattr>.parmCaseSensitive"))
        m_parmCaseSensitive = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.parmCaseSensitive")) == "true";
    // 过程返回的临时表数据集，临时表名称是否动态可变，默认不变（针对金仓数据库，建议设置为可变）
    if (pt.get_child_optional("<xmlattr>.dynTmpTableName"))
        m_dynTmpTableName = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.dynTmpTableName")) == "true";
}
CCtrlPgSQL::~CCtrlPgSQL()
{
}

// 获取类型名
CSqlFunc::EDataType CCtrlPgSQL::GetTypeName(unsigned int oid)
{
    switch (oid)
    {
    case NUMERICOID:
    case FLOAT8OID:
    case FLOAT4OID:
    case INT8OID:
        return CSqlFunc::EDataType::edtNumber;
        break;
    case INT4OID:
    case INT2OID:
        return CSqlFunc::EDataType::edtInt;
        break;
    case DATEOID:
    case TIMESTAMPOID:
    case TIMESTAMPTZOID:
        return CSqlFunc::EDataType::edtDatetime;
        break;
    case JSONOID:
    case JSONBOID:
        return CSqlFunc::EDataType::edtJson;
        break;
    case RECORDOID:
        return CSqlFunc::EDataType::edtDbSet;
        break;
    case VARCHAROID:
    default:
        return CSqlFunc::EDataType::edtString;
        break;
    }
}
unsigned int CCtrlPgSQL::GetOIDByTp(string tp)
{
    unsigned int oid = VARCHAROID;
    algorithm::to_lower(tp);
    if ("int" == tp) oid = INT4OID;
    else if ("numeric" == tp) oid = NUMERICOID;
    else if ("timestamp" == tp) oid = TIMESTAMPOID;
    else if ("date" == tp) oid = DATEOID;
    else if ("timestamptz" == tp) oid = TIMESTAMPTZOID;
    else if ("jsonb" == tp) oid = JSONBOID;
    else if ("json" == tp) oid = JSONOID;
    else if ("table" == tp) oid = RECORDOID;
    else oid = VARCHAROID;
    return oid;
}

// 执行控制器
void CCtrlPgSQL::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm& /*attPrm*/)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pSQL = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sSQL, sFuncCreateSQL, sExecSQL, sJsonParm, sAct = CUtilFunc::PCharSafeToStr(pAct);
    string sSrvFlag = m_name + "@" + CUtilFunc::PCharSafeToStr(m_own.getAlias(), "kc_sql_postgresql");
    // 异常信息
    int iExecSort = 0;
    auto fExceptInfo = [&](void)
    {
        return (format("\n%d.%s%s%s%s%s%s%s%s%s%s%s\n%s?act=%s") % iExecSort % act.GetSingleInfo("UniqueConnID") % c_strLineShow
                % sSQL % c_strLineShow % sFuncCreateSQL % c_strLineShow % sExecSQL % c_strLineShow
                % sJsonParm % c_strLineShow % act.GetSingleInfo("the_request") % pLocalFile % sAct).str();
    };
    try
    {
        // 方法类型
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, c_RESTful_exec));
        algorithm::to_lower(sMethod);
        // SQL语句
        // if ("copy" != sMethod && (nullptr == pSQL || strlen(pSQL) == 0))
        //     throw TKCSqlPgSQLException(ecd_ErrCode_KCSqlPostgresql + 5, __CURR_CODE_PLACE_C__, string(m_own.getHint("SQL_Empty_")) + act.GetSingleInfo("the_request") + " - " + pAct, m_own);
        sSQL = CUtilFunc::PCharSafeToStr(pSQL);
        // 参数
        //sJsonParm = objCtrlD.GetPostStr();
        sJsonParm = objCtrlD.JsonRequest().ToStr();
        cout << "\t" << CUtilFunc::Utf8ToGbk(sJsonParm).substr(0, 500) << endl;
        const char* pKCCLNID = act.GetGlobalVal("KC__SESSION__KCCLNID");
        m_own.WriteLogTrace(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (sSQL.substr(0, 256) + "\r\n" + sJsonParm.substr(0, 256) + "\r\n" + pLocalFile + "\t-\t" + sAct + "\npKCCLNID=" + (nullptr != pKCCLNID ? pKCCLNID : "")).c_str());
        // 数据库链接
        CSqlFunc::TConnObjWork<TPgConnObj> actObjWork(objCtrlD, /*sSrvFlag,*/ [&](){ return new TPgConnObj(*this); });
        try
        {
            PGconn *conn = actObjWork.ActObj().m_conn;
            ConnStatusType eConnST = CONNECTION_BAD;
            if(nullptr == conn || (eConnST = PQstatus(conn)) != CONNECTION_OK)
            {
                string sConnErr;
                if (nullptr != conn)
                {
                    string sPgErr = algorithm::trim_copy(CUtilFunc::PCharSafeToStr(PQerrorMessage(conn)));
                    if (!sPgErr.empty())
                        sConnErr = sPgErr + "   [ToUtf8::   " + CUtilFunc::GbkToUtf8(sPgErr) + "].";
                }
                throw TKCSqlPgSQLException(eConnST, __CURR_CODE_PLACE_C__, string(m_own.getHint("Database_Connection_Error_")) + sAct + ". " + sConnErr, m_own);
            }
            // 控制器执行类型
            // string sActGoTo = CUtilFunc::PCharSafeToStr(attPrm.GetStr("goto"));
            string sActGoTo = CUtilFunc::PCharSafeToStr(objCtrlD.GetGetArg(c_RESTful_goTo));
            // 创建数据库执行实例
            TDBCommandPgSQL comm(*this, objCtrlD, pLocalFile, sAct, *conn, sSQL, sMethod);
            CAutoRelease _auto([&](){
                sFuncCreateSQL = comm.m_funcCreateSQL;
                sExecSQL = comm.m_execSQL;
            });
            iExecSort = static_cast<int>(comm.m_execSort);
            // 查询（method="select"）
            if (c_RESTful_select == sMethod || c_RESTful_query == sMethod)
                comm.Select();
            // 存储过程（method="procedures"）
            else if (c_RESTful_procedures == sMethod)
                comm.Procedures();
            // 批量导入（method="batch"）
            else if (c_RESTful_batch == sMethod)
                comm.BatchInsert();
            // postgresql特有的批量数据导入（method="copy"）
            else if ("copy" == sMethod)
                comm.CopyFromStdinByCsv();
            // 调用临时函数（method="perform"）
            else if (c_RESTful_perform == sMethod || (c_RESTful_function == sMethod && c_RESTful_perform == sActGoTo))
                comm.PerformFunc();
            // 创建临时函数（method="create"）
            else if (c_RESTful_function == sMethod || c_RESTful_create == sMethod)
                comm.CreateDDL();
            // 增删改（method="update", "insert", "delete"）
            else if (sMethod.empty() || c_RESTful_exec == sMethod || c_RESTful_insert == sMethod || c_RESTful_delete == sMethod || c_RESTful_update == sMethod)
                comm.InsertDeleteUpdate();
            // 方法类型错误
            else throw TLibPQException(ecd_ErrCode_KCSqlPostgresql + 15, string(m_own.getHint("Request_Failed_")) + m_own.getHint("Method_Not_Allowed") + " [" + sMethod + "]", "", __CURR_CODE_PLACE_C__);
        }
        catch (...)
        {
            // 异常时，关闭持久连接
            if ("on" == actObjWork.m_keepAlive || "first" == actObjWork.m_keepAlive)
                act.SetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_keepAlive).c_str(), "off");
            throw;
        }
    }
    catch(TLibPQException& ex)
    {
        string sErr = (format("[%s]%d: %s\n%s") % ex.m_pos % ex.m_code % ex.m_msg % fExceptInfo()).str();
        m_own.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iErrCode = ex.m_code;
        objCtrlD.SetJsonRespond(0 == iErrCode ? ecd_ErrCode_KCSqlPostgresql + 6 : iErrCode, ("[Controllers." + sAct + "] " + ex.m_err).c_str());
    }
    catch (TKCSqlException& ex)
    {
        string sEx = ex.error_info();
        string sErr = "[" + std::to_string(ex.error_id()) + "]" + ex.error_place() + sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iCode ? ecd_ErrCode_KCSqlPostgresql + 7 : iCode, ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        ex.OtherInfo() += fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.BackTrace() = CBacktraceSymbols::Get();
        m_own.WriteLog(ex);
        int iErrCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iErrCode ? ecd_ErrCode_KCSqlPostgresql + 8 : iErrCode, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlPostgresql + 9, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlPostgresql + 10, ("[Controllers." + sAct + "] " + m_own.getHint("Unknown_exception") + " (" + sSrvFlag + ")").c_str());
    }
}
