#include "ctrl_dm.h"

////////////////////////////////////////////////////////////////////////////////
// 达梦DPI异常类
CCtrlDM::TDmdpiException::TDmdpiException(int c, string s, string p)
    : std::runtime_error(s.c_str()), m_code(c), m_msg(s), m_pos(p)
{
}

// 检查错误
void CCtrlDM::TDmdpiException::CheckError(CCtrlDM& own, DPIRETURN rt, sdint2 hndl_type, dhandle hndl, string sPos, string sKey)
{
    if(!DSQL_SUCCEEDED(rt))
    {
        sdint4 err_code = 0;
        sdint2 msg_len = 0;
        sdbyte err_msg[SDBYTE_MAX + 1] = { 0 };
        /* 获取错误信息集合*/
        DPIRETURN rtDiag = dpi_get_diag_rec(hndl_type, hndl, 1, &err_code, err_msg, SDBYTE_MAX, &msg_len);
        if (err_code != 0)
        {
            // 字符集
            string sErrUtf8 = CUtilFunc::GbkToUtf8((char*)err_msg);
            string sErr = (boost::format("errCode = %d \nerrMsg = [%s] %s \n\t (toUtf8:  %s )") % err_code % sKey % sErrUtf8 % err_msg).str();
            own.m_own.WriteLogFatal((sErr + " \n" + own.m_srv).c_str(), sPos.c_str(), "TDmdpiException");
            throw TDmdpiException(err_code, sErr, sPos);
        }
        else if (DSQL_NO_DATA == rtDiag)
        {
            string sErr = "[" + sKey + "] Fail <<DSQL_NO_DATA>> ";
            own.m_own.WriteLogFatal((sErr + " \n" + own.m_srv).c_str(), sPos.c_str(), "TDmdpiException");
            throw TDmdpiException(rt, sErr, sPos);
        }
    }
}

// CCtrlDM::TParmDM类
CCtrlDM::TParmDM::TParmDM(TParmGrpDM& own, std::string s, unsigned p) : m_own(own)
{
    iPos = p;
    sName = s;
}

// 设置参数值
bool CCtrlDM::TParmDM::SetNumber(double fVal)
{
    GetParmDesc();
    bool bResult = 0 == scale;
    DPIRETURN rt = DSQL_SUCCESS;
    if (bResult)
    {
        m_nVal.i = static_cast<int>(fVal);
        rt = dpi_bind_param(m_own.stmt, iPos, ePTp == CSqlFunc::EParmType::eptInOutParm ? DSQL_PARAM_INPUT_OUTPUT : DSQL_PARAM_INPUT,
                                      DSQL_C_SLONG, DSQL_INT, prec, 0, &m_nVal.i, sizeof(m_nVal.i), &c_ind);
    }
    else
    {
        m_nVal.d = fVal;
        rt = dpi_bind_param(m_own.stmt, iPos, ePTp == CSqlFunc::EParmType::eptInOutParm ? DSQL_PARAM_INPUT_OUTPUT : DSQL_PARAM_INPUT,
                                      DSQL_C_DOUBLE, DSQL_DEC, prec, scale, &m_nVal.d, sizeof(m_nVal.d), &c_ind);
    }
    TDmdpiException::CheckError(m_own.m_own, rt, DSQL_HANDLE_STMT, m_own.stmt, __CURR_CODE_PLACE_C__, this->sName);
    return bResult;
}
void CCtrlDM::TParmDM::SetInt(int iVal)
{
    GetParmDesc();
    m_nVal.i = iVal;
    DPIRETURN rt = dpi_bind_param(m_own.stmt, iPos, ePTp == CSqlFunc::EParmType::eptInOutParm ? DSQL_PARAM_INPUT_OUTPUT : DSQL_PARAM_INPUT,
                                  DSQL_C_SLONG, DSQL_INT, prec, 0, &m_nVal.i, sizeof(m_nVal.i), &c_ind);
    TDmdpiException::CheckError(m_own.m_own, rt, DSQL_HANDLE_STMT, m_own.stmt, __CURR_CODE_PLACE_C__, this->sName);
}

void CCtrlDM::TParmDM::SetString(string strVal)
{
    GetParmDesc();
    if (c_RESTful_GBK == m_own.m_own.GetCharset()) strVal = CUtilFunc::Utf8ToGbk(strVal);
    // 是否输入输出参数
    bool bIsOutIn = CSqlFunc::EParmType::eptInOutParm == ePTp || CSqlFunc::EParmType::eptSessionParm == ePTp;
    // 宽度
    unsigned iLen = 10;
    // 输出参数
    if (bIsOutIn)
        iLen = m_own.m_dmParmDesc.size() >= iPos ? max(static_cast<unsigned>(m_own.m_dmParmDesc[iPos - 1].prec), 10u) : c_KCMaxParmBufSize;
    // 输入参数
    else
        iLen = static_cast<unsigned>(strVal.size());
    if (0 == iLen) iLen = 10;
    // 缓冲区
    m_sVal.resize(iLen + 1);
    memset(m_sVal.data(), 0, iLen + 1);
    memcpy(m_sVal.data(), strVal.c_str(), std::min(static_cast<unsigned>(strVal.size()), iLen));
    c_ind = strVal.size();
    DPIRETURN rt = dpi_bind_param(m_own.stmt, iPos, bIsOutIn ? DSQL_PARAM_INPUT_OUTPUT : DSQL_PARAM_INPUT,
                                  DSQL_C_CHAR, DSQL_VARCHAR, std::max(iLen, 1u), 0, m_sVal.data(), iLen, &c_ind);
    TDmdpiException::CheckError(m_own.m_own, rt, DSQL_HANDLE_STMT, m_own.stmt, __CURR_CODE_PLACE_C__, this->sName);
}
void CCtrlDM::TParmDM::SetClob(string strVal)
{
    SetString(strVal);
}

void CCtrlDM::TParmDM::SetNull(void)
{
    GetParmDesc();
    DPIRETURN rt = DSQL_SUCCESS;
    // static const char sDateInit1[256] = { 0 };
    if (DSQL_TIMESTAMP == sql_type)
    {
        // dpi_timestamp_struct dt;
        // memset(&dt, 0, sizeof(dpi_timestamp_struct));
        // dt.year = 1900;
        // dt.month = dt.day = 1;
        // rt = dpi_bind_param(m_own.stmt, iPos, DSQL_PARAM_INPUT, DSQL_C_TIMESTAMP, DSQL_TIMESTAMP, prec, 0, (void*)&dt, sizeof(dpi_timestamp_struct), &c_ind);
        // static const char* sDateInit = "1000-01-01 00:00:00";
        // rt = dpi_bind_param(m_own.stmt, iPos, DSQL_PARAM_INPUT, DSQL_C_CHAR, DSQL_TIMESTAMP, prec, 0, (void*)sDateInit, strlen(sDateInit), &c_ind);
        c_ind = DSQL_NULL_DATA;
        rt = dpi_bind_param(m_own.stmt, iPos, DSQL_PARAM_INPUT, DSQL_C_BIT, sql_type, prec, 0, (void*)"", 0, &c_ind);
    }
    else
    {
        c_ind = DSQL_NULL_DATA;
        rt = dpi_bind_param(m_own.stmt, iPos, DSQL_PARAM_INPUT, DSQL_C_CHAR, sql_type, 1, 0, (void*)"", 0, &c_ind);
    }
    TDmdpiException::CheckError(m_own.m_own, rt, DSQL_HANDLE_STMT, m_own.stmt, __CURR_CODE_PLACE_C__, this->sName + " is null");
}

// 得到dm参数信息
void CCtrlDM::TParmDM::GetParmDesc(void)
{
    // sdint2         sql_type     = DSQL_VARCHAR;
    // ulength        prec         = 0;
    // sdint2         scale        = 0;
    // sdint2         nullable     = 0;
    // DPIRETURN rt = dpi_desc_param(m_own.stmt, iPos, &sql_type, &prec, &scale, &nullable);
    // try
    // {
    //     TDmdpiException::CheckError(m_own.m_own, rt, DSQL_HANDLE_STMT, m_own.stmt, __CURR_CODE_PLACE_C__, this->sName);
    // }
    // catch (TDmdpiException &ex)
    // {
    //     m_own.m_own.m_own.WriteLogWarning(ex.m_msg.c_str(), ex.m_pos.c_str(), "TDmdpiException");
    // }
    // return std::make_tuple(sql_type, prec, scale, nullable);

    if (m_own.m_dmParmDesc.size() >= iPos)
    {
        auto &parmDesc = m_own.m_dmParmDesc[iPos - 1];
        sql_type = parmDesc.sql_type;
        prec = parmDesc.prec;
        scale = parmDesc.scale;
        nullable = parmDesc.nullable;
        eDTp = CCtrlDM::GetTypeName(sql_type);
    }
}

// 获取参数值
bool CCtrlDM::TParmDM::IsNull(void)
{
    bIsNull = c_ind < 0;
    return bIsNull;
}

string CCtrlDM::TParmDM::GetString(string sDef)
{
    string sRes = IsNull() ? sDef : &m_sVal[0];
    if (c_RESTful_GBK == m_own.m_own.GetCharset()) sRes = CUtilFunc::GbkToUtf8(sRes);
    return sRes;
}

string CCtrlDM::TParmDM::GetClob(string sDef)
{
    return GetString(sDef);
}

string CCtrlDM::TParmDM::GetDate(string sDef)
{
    return GetString(sDef);
}

string CCtrlDM::TParmDM::GetDateTime(string sDef)
{
    return GetString(sDef);
}

int CCtrlDM::TParmDM::GetInt(int iDef)
{
    return IsNull() ? iDef : m_nVal.i;
}

double CCtrlDM::TParmDM::GetNumber(double fDef)
{
    return IsNull() ? fDef : m_nVal.d;
}

// 获取游标数据集
CSqlFunc::TRecordSetPtr CCtrlDM::TParmDM::GetDbSet(void)
{
    //throw TKCSqlDMException(ecd_ErrCode_KCSqlDM + 3, __CURR_CODE_PLACE_C__, "Not Support", m_own.m_own.m_own);
    return CSqlFunc::TRecordSetPtr();
}

void CCtrlDM::TParmDM::SetDbTp(void)
{
}

void CCtrlDM::TParmDM::RegOutParam(void)
{
     DPIRETURN rt = DSQL_SUCCESS;
    if (eDTp == CSqlFunc::EDataType::edtInt)
    {
        m_nVal.i = 0;
        rt = dpi_bind_param(m_own.stmt, iPos, DSQL_PARAM_INPUT_OUTPUT, DSQL_C_SLONG, DSQL_INT, sizeof(m_nVal.i), 0, &m_nVal.i, sizeof(m_nVal.i), &c_ind);
    }
    else if (eDTp == CSqlFunc::EDataType::edtNumber)
    {
        m_nVal.d = 0;
        rt = dpi_bind_param(m_own.stmt, iPos, DSQL_PARAM_INPUT_OUTPUT, DSQL_C_NUMERIC, DSQL_DEC, sizeof(m_nVal.d), 0, &m_nVal.d, sizeof(m_nVal.d), &c_ind);
    }
    else
    {
        // 宽度
        unsigned iLen = m_own.m_dmParmDesc.size() >= iPos ? max(static_cast<unsigned>(m_own.m_dmParmDesc[iPos - 1].prec), 10u) : c_KCMaxParmBufSize;
        m_sVal.resize(iLen + 1);
        memset(m_sVal.data(), 0, iLen + 1);
        c_ind = iLen;
        rt = dpi_bind_param(m_own.stmt, iPos, DSQL_PARAM_INPUT_OUTPUT, DSQL_C_NCHAR, DSQL_VARCHAR, iLen, 0, m_sVal.data(), iLen, &c_ind);
    }
    TDmdpiException::CheckError(m_own.m_own, rt, DSQL_HANDLE_STMT, m_own.stmt, __CURR_CODE_PLACE_C__, this->sName);
}

// CCtrlDM::TParmGrpDM类
CCtrlDM::TParmGrpDM::TParmGrpDM(CCtrlDM& ctrl, dhcon& cn, KeepStmtDMPtr st, ICtrlApiData& ctrlD, string sAct)
    : TParmGrpDMBase(false, false, false), m_own(ctrl), conn(cn), keepStmt(st), stmt(st->stmt), m_objCtrlD(ctrlD), m_act(sAct)
{
}


// 所属的服务
IKCSql& CCtrlDM::TParmGrpDM::Srv(void)
{
    return m_own.m_own;
}

//  控制器 信息
string CCtrlDM::TParmGrpDM::ActInfo(void)
{
    return m_act;
}

// 创建参数
CSqlFunc::TParmPtr CCtrlDM::TParmGrpDM::MakePram(std::string s, unsigned p)
{
    CSqlFunc::TParmPtr ptrPrm;
    auto itrPrm = mapDmParms.find(s);
    if (mapDmParms.end() == itrPrm)
    {
        ptrPrm.reset(new TParmDM(*this, s, p));
        mapDmParms.insert(std::make_pair(s, ptrPrm));
    }
    else ptrPrm = itrPrm->second;
    return ptrPrm;
}

// 在数据库上解绑所有已绑定的参数
void CCtrlDM::TParmGrpDM::UnbindParms(void)
{
    DPIRETURN rt = dpi_unbind_params(stmt);
    try
    {
        TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "dpi_unbind_params");
    }
    catch (TDmdpiException &ex)
    {
        cout << ex.m_msg << endl;
    }
    // dpointer ptr;
    // dpi_param_data(stmt, &ptr);
}

// 异常信息
string CCtrlDM::TParmGrpDM::ParmExceptInfo(TDmdpiException& ex)
{
    return ex.m_msg;
}

// CCtrlDM::TFieldDM 类
CCtrlDM::TFieldDM::TFieldDM(TRecordSetDM& own, std::string n, int p, int t, unsigned sz, unsigned dc, bool nl)
    : CSqlFunc::TField(n, p, t, CCtrlDM::GetTypeName(t), sz, dc, nl)
{
    DPIRETURN rt = DSQL_SUCCESS;
    switch (t)
    {
    case DSQL_INT:
        rt = dpi_bind_col(own.stmt, p, DSQL_C_SLONG, &m_nVal.i, sizeof(m_nVal.i), &c_ind);
        break;
    case DSQL_DEC:
        if (0 == dc)
            rt = dpi_bind_col(own.stmt, p, DSQL_C_SLONG, &m_nVal.i, sizeof(m_nVal.i), &c_ind);
        else
            //rt = dpi_bind_col(own.stmt, p, DSQL_C_NUMERIC, &dpi_numeric_struct(), sizeof(dpi_numeric_struct), &c_ind);
            rt = dpi_bind_col(own.stmt, p, DSQL_C_DOUBLE, &m_nVal.d, sizeof(m_nVal.d), &c_ind);
        break;
    default:
        m_sVal.resize(min(sz, c_KCMaxParmBufSize));
        rt = dpi_bind_col(own.stmt, p, DSQL_C_NCHAR, m_sVal.data(), m_sVal.size(), &c_ind);
        break;
    }
    TDmdpiException::CheckError(own.m_own, rt, DSQL_HANDLE_STMT, own.stmt, __CURR_CODE_PLACE_C__, n);
}

// CCtrlDM::TRecordSetDM 类
CCtrlDM::TRecordSetDM::TRecordSetDM(CCtrlDM& own, KeepStmtDMPtr st, ICtrlApiData& ctrlD)
    : m_own(own), m_objCtrlD(ctrlD), keepStmt(st), stmt(st->stmt)
{
}
CCtrlDM::TRecordSetDM::~TRecordSetDM(void)
{
}

// 获取字段信息
bool CCtrlDM::TRecordSetDM::GetColumnListMetaDataBeforeFetch(CSqlFunc::TFields &vecFeilds)
{
    map<string, std::shared_ptr<CSqlFunc::TField>> mapFeilds;
    vector<string> vecColNames;
    // 字段数量
    sdint2 iFieldCount = 0;
    dpi_number_columns(stmt, &iFieldCount);
    //cout << iFieldCount << endl;
    // 字段信息
    for (int i = 1; i <= iFieldCount; ++i)
    {
        sdbyte         name[200];
        sdint2         name_len;
        sdint2         sqltype;
        ulength        col_sz;
        sdint2         dec_digits;
        sdint2         nullable;
        dpi_desc_column(stmt, i, name, 200, &name_len, &sqltype, &col_sz, &dec_digits, &nullable);
        //cout << name << "\t" << name_len << "\tsqltype=" << sqltype << "\tcol_sz=" << col_sz << "\tdec_digits=" << dec_digits << "\tnullable=" << nullable << endl;
        string sColName = (char*)(name);
        algorithm::to_lower(sColName);
        vecColNames.push_back(sColName);
        mapFeilds.insert(make_pair(sColName, std::shared_ptr<CSqlFunc::TField>(new TFieldDM(*this, sColName, i, (int)sqltype, static_cast<unsigned>(col_sz), dec_digits, 1 == nullable))));
    }
    if (vecFeilds.empty())
        for (int i = 0; i < iFieldCount; ++i)
        {
            string sFieldName = vecColNames[i];
            auto it = mapFeilds.find(sFieldName);
            if (mapFeilds.end() != it) vecFeilds.push_back(it->second);
        }
    else
        for (auto &fd : vecFeilds)
        {
            auto it = mapFeilds.find(algorithm::to_lower_copy(fd->m_name));
            if (mapFeilds.end() != it) fd = it->second;
        }
    return true;
}
// 下一条
bool CCtrlDM::TRecordSetDM::Next(void)
{
    return dpi_fetch(stmt, &row_num) != DSQL_NO_DATA;
}
// 添加值
void CCtrlDM::TRecordSetDM::AddVal(IKCJson& jsonRow, CSqlFunc::TField& fld)
{
    TFieldDM &flddm = dynamic_cast<TFieldDM&>(fld);
    if (fld.m_pos > 0 && flddm.c_ind >= 0)
        switch (fld.m_type)
        {
        case DSQL_INT:
            jsonRow.AddVal(fld.m_name.c_str(), flddm.m_nVal.i);
            break;
        case DSQL_DEC:
            if (0 == flddm.m_decimals)
                jsonRow.AddVal(fld.m_name.c_str(), flddm.m_nVal.i);
            else
                jsonRow.AddVal(fld.m_name.c_str(), flddm.m_nVal.d);
            break;
        default:
            {
                string strVal = &flddm.m_sVal[0];
                if (c_RESTful_GBK == m_own.GetCharset()) strVal = CUtilFunc::GbkToUtf8(strVal);
                jsonRow.AddStr(fld.m_name.c_str(), strVal.c_str());
            }
            break;
        }
    else jsonRow.AddNull(fld.m_name.c_str());
}
// 得到值
CSqlFunc::TKcValVariant CCtrlDM::TRecordSetDM::GetVal(CSqlFunc::TField& fld) const
{
    CSqlFunc::TKcValVariant result;
    TFieldDM &flddm = dynamic_cast<TFieldDM&>(fld);
    if (fld.m_pos > 0 && flddm.c_ind >= 0)
        switch (fld.m_type)
        {
        case DSQL_INT:
            result = flddm.m_nVal.i;
            break;
        case DSQL_DEC:
            if (0 == flddm.m_decimals)
                result =flddm.m_nVal.i;
            else
                result = flddm.m_nVal.d;
            break;
        default:
            {
                string strVal(&flddm.m_sVal[0], flddm.m_sVal.size());
                if (c_RESTful_GBK == m_own.GetCharset()) strVal = CUtilFunc::GbkToUtf8(strVal);
                result = strVal;
            }
            break;
        }
    return result;
}

// CCtrlDM::TDBCommandDM类
CCtrlDM::TDBCommandDM::TDBCommandDM(CCtrlDM& own, dhcon& cn, KeepStmtDMPtr st, ICtrlApiData& ctrlD, string sAct, string sSQL, string sMethod, string &sBack)
    : m_own(own), conn(cn), keepStmt(st), stmt(st->stmt), m_objCtrlD(ctrlD), m_parms(own, cn, st, m_objCtrlD, sAct), m_act(sAct), m_sql(sSQL), m_method(sMethod), m_back(sBack)
    , m_nowTime(posix_time::to_iso_string(posix_time::second_clock::local_time()))
    , m_parmTmpTab((format("##__%d_%d_%s_%d__") % boost::this_process::get_id() % boost::this_thread::get_id() % m_nowTime % (rand() % 1000)).str())
{
    // 移除注释
    m_sql = CCtrlCommon::RemoveComment(m_sql);
    // 字符集编码
    if (c_RESTful_GBK == m_own.GetCharset()) m_sql = CUtilFunc::Utf8ToGbk(m_sql);
    // 拼成1条语句
    string sTmp = algorithm::trim_copy(m_sql);
    sTmp = algorithm::trim_copy(algorithm::to_lower_copy(sTmp.substr(sTmp.size() - 4)));
    //cout << sTmp << endl;
    if ("end" != sTmp && "end;" != sTmp)
    {
        if (sTmp[sTmp.size() - 1] != ';') m_sql += ';';
        m_sql = "begin " + m_sql + " end;";
    }
    // if ("end" != sTmp && "end;" != sTmp && sTmp[sTmp.size() - 1] == ';')
    //     m_sql = "begin " + m_sql + " end;";
}
CCtrlDM::TDBCommandDM::~TDBCommandDM(void)
{
    m_back = m_sql;
}

// 所属的服务
IKCSql& CCtrlDM::TDBCommandDM::Srv(void)
{
    return m_own.m_own;
}

//  控制器 信息
string CCtrlDM::TDBCommandDM::ActInfo(void)
{
    return m_act;
}

// 事务
void CCtrlDM::TDBCommandDM::TranBegin(void)
{
    ExecSQL("START TRANSACTION;");
}
void CCtrlDM::TDBCommandDM::TranCommit(void)
{
    // dpi_commit(conn);
    ExecSQL("COMMIT;");
}
void CCtrlDM::TDBCommandDM::TranRollback(void)
{
    // dpi_rollback(conn);
    ExecSQL("ROLLBACK;");
}

// 预执行
void CCtrlDM::TDBCommandDM::PrepareSQL(void)
{
    // SQL语句不能为空
    if (m_sql.empty())
        throw TKCSqlDMException(ecd_ErrCode_KCSqlDM + 5, __CURR_CODE_PLACE_C__, string(m_own.m_own.getHint("SQL_Empty_")) + m_objCtrlD.ActionData().GetSingleInfo("the_request") + " - " + m_act, m_own.m_own);
    // 准备sql语句
    DPIRETURN rt = dpi_prepare(stmt, (sdbyte*)(m_sql.c_str()));
    TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "dpi_prepare");
    // 参数数量
    udint2 iPrmCount = 0;
    dpi_number_params(stmt, &iPrmCount);
    //cout << iPrmCount << endl;
    m_parms.m_dmParmDesc.resize(iPrmCount);
    // 参数信息
    for (int i = 1; i <= iPrmCount; ++i)
    {
        TParmGrpDM::TDmParmDesc &prmDesc = m_parms.m_dmParmDesc[i - 1];
        dpi_desc_param(stmt, i, &prmDesc.sql_type, &prmDesc.prec, &prmDesc.scale, &prmDesc.nullable);
        //cout << "sqltype=" << sql_type << "\tprec=" << prec << "\tscale=" << scale << "\tnullable=" << nullable << endl;
        // 禁止返回游标类型
        if (DSQL_RSET == prmDesc.sql_type)
            throw TDmdpiException(prmDesc.sql_type, "Disable output parameter cursor", __CURR_CODE_PLACE_C__);
    }
}

// 执行
unsigned int CCtrlDM::TDBCommandDM::ExecuteUpdate(void)
{
    DPIRETURN rt = dpi_exec(stmt);
    TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "dpi_exec");
    // sdint4 err_code = 0;
    // sdbyte err_msg[SDBYTE_MAX + 1];
    // sdint2 len = 0;
    // dpi_get_diag_rec(DSQL_HANDLE_STMT, stmt, 1, &err_code, err_msg, SDBYTE_MAX, &len);
    // int rows_affected = 0;
    // sdint4 len = 0;
    // DPIRETURN rt2 = dpi_get_stmt_attr(stmt, DSQL_ATTR_ROWS_FETCHED_PTR, &rows_affected, sizeof(rows_affected), &len);
    sdint8 rows_affected = 0;
    [[maybe_unused]] DPIRETURN rt2 = dpi_row_count(stmt, &rows_affected);
    return static_cast<unsigned int>(rows_affected);
}

// 批量操作
pair<int, string> CCtrlDM::TDBCommandDM::ExecuteBatch(void)
{
    // int iErrCode = 0;
    // string sErrMsg;
    // try
    // {
    //     DPIRETURN rt = dpi_exec_add_batch(stmt);
    //     TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "dpi_exec_add_batch");
    // }
    // catch (TDmdpiException& ex)
    // {
    //     iErrCode = 0 == ex.m_code ? ecd_ErrCode_KCSqlDM + 4 : ex.m_code;
    //     sErrMsg = ex.m_msg + " <" + typeid(ex).name() + ">";
    // }
    // return make_pair(iErrCode, sErrMsg);

    try
    {
        return make_pair(ExecuteUpdate(), string(""));
    }
    catch (TDmdpiException& ex)
    {
        int iErrCode = 0 == ex.m_code ? ecd_ErrCode_KCSqlDM + 4 : ex.m_code;
        string sErrMsg = ex.m_msg + " <" + typeid(ex).name() + ">";
        return make_pair(iErrCode, sErrMsg);
    }
}
// 批量插入结束
void CCtrlDM::TDBCommandDM::BatchInsertEnd(bool bCommit)
{
    // if (bCommit)
    // {
    //     DPIRETURN rt = dpi_exec_batch(stmt);
    //     TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "dpi_exec_batch");
    // }
}

// 查询
CSqlFunc::TRecordSetPtr CCtrlDM::TDBCommandDM::ExecuteQuery(int &rows_affected)
{
    rows_affected = static_cast<int>(ExecuteUpdate());
    sdint2 iColCount = 0;
    dpi_number_columns(stmt, &iColCount);
    return iColCount > 0 ? CSqlFunc::TRecordSetPtr(new TRecordSetDM(m_own, keepStmt, m_objCtrlD)) : CSqlFunc::TRecordSetPtr();
}
CSqlFunc::TRecordSetPtr CCtrlDM::TDBCommandDM::NextResult(void)
{
    // 下一个数据集
    try
    {
        DPIRETURN rt = dpi_more_results(stmt);
        TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "dpi_more_results");
        return CSqlFunc::TRecordSetPtr(new TRecordSetDM(m_own, keepStmt, m_objCtrlD));
    }
    catch (...) {}
    return CSqlFunc::TRecordSetPtr();
}

// 执行过程
int CCtrlDM::TDBCommandDM::ExecuteProcedures(void)
{
    return static_cast<int>(ExecuteUpdate());
}

// 单独执行sql语句
void CCtrlDM::TDBCommandDM::ExecSQL(const char* sql)
{
    try
    {
        dhstmt _stmt;
        // 创建数据库执行实例
        DPIRETURN rt = dpi_alloc_stmt(conn, &_stmt);
        TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_DBC, conn, __CURR_CODE_PLACE_C__, "dpi_alloc_stmt");
        // 退出时，释放执行实例
        CAutoRelease _auto([&]() {
            try
            {
                rt = dpi_free_stmt(_stmt);
                TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_DBC, conn, __CURR_CODE_PLACE_C__, "dpi_free_stmt");
            }
            catch (...) {}
        });
        // 准备sql语句
        rt = dpi_prepare(_stmt, (sdbyte*)(sql));
        TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, _stmt, __CURR_CODE_PLACE_C__, "dpi_prepare");
        // 执行
        rt = dpi_exec(_stmt);
        TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_STMT, _stmt, __CURR_CODE_PLACE_C__, "dpi_exec");
    }
    catch (TDmdpiException& ex)
    {
        m_own.m_own.WriteLogFatal(ex.m_msg.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch(TException& ex)
    {
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        m_own.m_own.WriteLog(ex);
    }
    catch (std::exception& ex)
    {
        m_own.m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        m_own.m_own.WriteLogFatal("Unknown Exception", __CURR_CODE_PLACE_C__);
    }
}

// CCtrlDM::TDmConnObj类
CCtrlDM::TDmConnObj::TDmConnObj(CCtrlDM& own, unsigned connid) : m_own(own), m_connid(connid)
{
    // 创建链接
    DPIRETURN rt = dpi_alloc_con(own.m_henv, &m_conn);
    TDmdpiException::CheckError(own, rt, DSQL_HANDLE_DBC, m_conn, __CURR_CODE_PLACE_C__, "dpi_alloc_con");
    // 登录
    rt = dpi_login(m_conn,
                   reinterpret_cast<signed char*>(const_cast<char*>(own.m_srv.c_str())),
                   reinterpret_cast<signed char*>(const_cast<char*>(own.m_user.c_str())),
                   reinterpret_cast<signed char*>(const_cast<char*>(own.m_pass.c_str()))
    );
    TDmdpiException::CheckError(own, rt, DSQL_HANDLE_DBC, m_conn, __CURR_CODE_PLACE_C__, "dpi_login");
    // 获取数据库字符集编码
    try
    {
        // 申请语句句柄
        dhstmt stmt = nullptr;
        rt = dpi_alloc_stmt(m_conn, &stmt);
        TDmdpiException::CheckError(own, rt, DSQL_HANDLE_DBC, m_conn, __CURR_CODE_PLACE_C__, "dpi_alloc_stmt");
        // 释放语句句柄
        CAutoRelease _auto([&]() { dpi_free_stmt(stmt); });
        // 执行语句
        rt = dpi_exec_direct(stmt, reinterpret_cast<signed char*>(const_cast<char*>("select UNICODE()")));
        TDmdpiException::CheckError(own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "select UNICODE()");
        // 绑定字段
        slength c_ind = 0;
        sdint4 c1 = 0;
        rt = dpi_bind_col(stmt, 1, DSQL_C_SLONG, &c1, sizeof(c1), &c_ind);
        TDmdpiException::CheckError(own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "dpi_bind_col");
        ulength row_num = 0;
        while(dpi_fetch(stmt, &row_num) != DSQL_NO_DATA)
        {
            own.m_charset = 0 == c1 ? c_RESTful_GBK : c_RESTful_UTF8;
            break;
        }
    }
    catch (...) {}
    // 设置默认模式
    try
    {
        // 申请语句句柄
        dhstmt stmt = nullptr;
        rt = dpi_alloc_stmt(m_conn, &stmt);
        TDmdpiException::CheckError(own, rt, DSQL_HANDLE_DBC, m_conn, __CURR_CODE_PLACE_C__, "dpi_alloc_stmt");
        // 释放语句句柄
        CAutoRelease _auto([&]() { dpi_free_stmt(stmt); });
        // 执行语句
        rt = dpi_exec_direct(stmt, reinterpret_cast<signed char*>(const_cast<char*>(("set schema " + own.m_model).c_str())));
        TDmdpiException::CheckError(own, rt, DSQL_HANDLE_STMT, stmt, __CURR_CODE_PLACE_C__, "set schema " + own.m_model);
    }
    catch (...) {}
}
void CCtrlDM::TDmConnObj::Release(void)
{
    // 退出
    DPIRETURN rt = dpi_logout(m_conn);
    TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_DBC, m_conn, __CURR_CODE_PLACE_C__, "dpi_logout");
    // 释放链接
    rt = dpi_free_con(m_conn);
    TDmdpiException::CheckError(m_own, rt, DSQL_HANDLE_DBC, m_conn, __CURR_CODE_PLACE_C__, "dpi_free_con");
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlDM类
CCtrlDM::CCtrlDM(IKCSqlDM& own, string sName, property_tree::ptree& pt)
    : m_own(own), m_name(sName), m_connID(0)
{
    // 记录函数地址
    m_own.WriteLogDebug((m_name + ": "
                            + "\nCCtrlDM::Exce: \t\t" + CUtilFunc::GetFuncAddr(&CCtrlDM::Perform)
                       ).c_str());
    // 服务器配置
    if (pt.get_child_optional("<xmlattr>.srv")) m_srv = pt.get<string>("<xmlattr>.srv");
    if (pt.get_child_optional("<xmlattr>.uid")) m_user = pt.get<string>("<xmlattr>.uid");
    if (pt.get_child_optional("<xmlattr>.pwd")) m_pass = pt.get<string>("<xmlattr>.pwd");
    if (pt.get_child_optional("<xmlattr>.model")) m_model = pt.get<string>("<xmlattr>.model");
    if (pt.get_child_optional("<xmlattr>.charset")) m_charsetCfg = pt.get<string>("<xmlattr>.charset");
    // 是否加密连接参数
    if (pt.get_child_optional("<xmlattr>.encrypt"))
    {
        m_encryptConnStr = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.encrypt")) == "true";
        if (m_encryptConnStr)
        {
            m_srv = CCtrlCommon::DecryptConnStrDB(m_own.getContext(), m_srv);
            m_user = CCtrlCommon::DecryptConnStrDB(m_own.getContext(), m_user);
            m_pass = CCtrlCommon::DecryptConnStrDB(m_own.getContext(), m_pass);
            m_model = CCtrlCommon::DecryptConnStrDB(m_own.getContext(), m_model);
        }
    }
    // 创建数据库环境
    DPIRETURN rt = dpi_alloc_env(&m_henv);
    TDmdpiException::CheckError(*this, rt, DSQL_HANDLE_ENV, m_henv, __CURR_CODE_PLACE_C__, "dpi_alloc_env");
}
CCtrlDM::~CCtrlDM()
{
    // 释放数据库环境
    DPIRETURN rt = dpi_free_env(m_henv);
    try
    {
        TDmdpiException::CheckError(*this, rt, DSQL_HANDLE_ENV, m_henv, __CURR_CODE_PLACE_C__, "dpi_free_env");
    }
    catch (...) {}
}

// 得到编码
inline string CCtrlDM::GetCharset(void) const
{
    return boost::algorithm::to_upper_copy(m_charsetCfg.empty() ? m_charset : m_charsetCfg);
}

// 获取类型名
CSqlFunc::EDataType CCtrlDM::GetTypeName(int tp)
{
    switch (tp)
    {
    case DSQL_DEC:
    case DSQL_FLOAT:
    case DSQL_DOUBLE:
    case DSQL_BIGINT:
        return CSqlFunc::EDataType::edtNumber;
        break;
    case DSQL_INT:
    case DSQL_SMALLINT:
        return CSqlFunc::EDataType::edtInt;
        break;
    case DSQL_DATE:
    case DSQL_TIME:
    case DSQL_TIMESTAMP:
    case DSQL_TIME_TZ:
    case DSQL_TIMESTAMP_TZ:
        return CSqlFunc::EDataType::edtDatetime;
        break;
    case DSQL_RSET:
        return CSqlFunc::EDataType::edtDbSet;
        break;
    case DSQL_BLOB:
    case DSQL_CLOB:
        return CSqlFunc::EDataType::edtClob;
        break;
    case DSQL_VARCHAR:
    default:
        return CSqlFunc::EDataType::edtString;
        break;
    }
}

// 执行控制器
void CCtrlDM::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pSQL = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sSQL, sLast, sJsonParm, sAct = CUtilFunc::PCharSafeToStr(pAct);
    string sSrvFlag = m_name + "@" + CUtilFunc::PCharSafeToStr(m_own.getAlias(), "kc_sql_dm");
    dhcon conn = nullptr;
    dhstmt stmt = nullptr;
    auto fExceptInfo = [&](void)
    {
        return (format("%s%s%s%s%s%s\n%s\n%s?act=%s\n[Connect: %d-%X][Statement: %X] \t")
                % c_strLineShow % sLast % c_strLineShow % CUtilFunc::PCharSafeToStr(pSQL) % c_strLineShow
                % act.GetSingleInfo("the_request") % sJsonParm % pLocalFile % sAct % m_connID % conn % stmt).str();
    };
    try
    {
        // SQL语句
        // if (nullptr == pSQL || strlen(pSQL) == 0)
        //     throw TKCSqlDMException(ecd_ErrCode_KCSqlDM + 6, __CURR_CODE_PLACE_C__, string(m_own.getHint("SQL_Empty_")) + act.GetSingleInfo("the_request") + " - " + sAct, m_own);
        sSQL = CUtilFunc::PCharSafeToStr(pSQL);
        // 参数
        sJsonParm = objCtrlD.JsonRequest().ToStr();
        cout << "\t" << CUtilFunc::Utf8ToGbk(sJsonParm).substr(0, 500) << endl;
        const char* pKCCLNID = act.GetGlobalVal("KC__SESSION__KCCLNID");
        m_own.WriteLogTrace(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (sSQL.substr(0, 256) + "\r\n" + sJsonParm.substr(0, 256) + "\r\n" + pLocalFile + "\t-\t" + sAct + "\npKCCLNID=" + (nullptr != pKCCLNID ? pKCCLNID : "")).c_str());
        // 方法类型
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, c_RESTful_exec));
        algorithm::to_lower(sMethod);
        // 获取数据库链接
        CSqlFunc::TConnObjWork<TDmConnObj> actObjWork(objCtrlD, /*sSrvFlag,*/ [&](){ return new TDmConnObj(*this, ++m_connID); });
        try
        {
            conn = actObjWork.ActObj().m_conn;
            // 创建数据库执行实例
            DPIRETURN rt = dpi_alloc_stmt(conn, &stmt);
            TDmdpiException::CheckError(*this, rt, DSQL_HANDLE_DBC, conn, __CURR_CODE_PLACE_C__, "dpi_alloc_stmt");
            // 执行
            {
                // 保持dhstmt对象
                KeepStmtDMPtr keepStmt(new KeepStmtDM(stmt));
                // 数据库执行命令
                TDBCommandDM comm(*this, conn, keepStmt, objCtrlD, pLocalFile + string("?act=") + sAct, pSQL, sMethod, sLast);
                // 批量导入（method="batch"）
                if (c_RESTful_batch == sMethod)
                    comm.BatchInsert();
                // 增删改查（method="exec"）
                else if (sMethod.empty() || c_RESTful_exec == sMethod || c_RESTful_query == sMethod || c_RESTful_select == sMethod || c_RESTful_insert == sMethod || c_RESTful_delete == sMethod || c_RESTful_update == sMethod || c_RESTful_procedures == sMethod)
                    comm.Query();
                // 方法类型错误
                else throw TDmdpiException(ecd_ErrCode_KCSqlDM + 5, string(m_own.getHint("Request_Failed_")) + m_own.getHint("Method_Not_Allowed") + " [" + sMethod + "]", __CURR_CODE_PLACE_C__);
            }
            //m_own.WriteLogDebug(pMethod, __CURR_CODE_PLACE_C__, fExceptInfo().c_str());
        }
        catch (...)
        {
            // 异常时，关闭持久连接
            if ("on" == actObjWork.m_keepAlive || "first" == actObjWork.m_keepAlive)
                act.SetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_keepAlive).c_str(), "off");
            throw;
        }
    }
    catch (TDmdpiException& ex)
    {
        string sEx = ex.m_msg;
        string sErr = sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        // if (CUtilFunc::isGBK(sEx) && !CUtilFunc::isUtf8(sEx)) sEx = CUtilFunc::GbkToUtf8(sEx);
        objCtrlD.SetJsonRespond(ex.m_code, ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch (TKCSqlException& ex)
    {
        string sEx = ex.error_info();
        string sErr = "[" + std::to_string(ex.error_id()) + "]" + ex.error_place() + sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iCode ? ecd_ErrCode_KCSqlDM + 6 : iCode, ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        ex.OtherInfo() += fExceptInfo();
        ex.BackTrace() = CBacktraceSymbols::Get();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        m_own.WriteLog(ex);
        int iErrCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iErrCode ? ecd_ErrCode_KCSqlDM + 7 : iErrCode, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlDM + 8, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlDM + 9, ("[Controllers." + sAct + "] " + m_own.getHint("Unknown_exception") + " (" + sSrvFlag + ")").c_str());
    }
}
