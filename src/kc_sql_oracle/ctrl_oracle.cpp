#include "ctrl_oracle.h"

////////////////////////////////////////////////////////////////////////////////
// CCtrlOracle::TParmOra类
CCtrlOracle::TParmOra::TParmOra(TParmGrpOra& own, std::string s, unsigned p) : m_own(own)
{
    iPos = p;
    sName = s;
}

// 设置参数值
bool CCtrlOracle::TParmOra::SetNumber(double fVal)
{
    m_own.stmt.setNumber(iPos, fVal);
    eDTp = CSqlFunc::EDataType::edtNumber;
    return false;
}

void CCtrlOracle::TParmOra::SetInt(int iVal)
{
    m_own.stmt.setInt(iPos, iVal);
    eDTp = CSqlFunc::EDataType::edtInt;
}

void CCtrlOracle::TParmOra::SetString(string strVal)
{
    if (CSqlFunc::EParmType::eptOutParm == ePTp || CSqlFunc::EParmType::eptInOutParm == ePTp || CSqlFunc::EParmType::eptSessionParm == ePTp)
        m_own.stmt.setMaxParamSize(iPos, c_KCMaxParmBufSize);
    m_own.stmt.setString(iPos, strVal.substr(0, c_KCMaxParmBufSize));
    eDTp = CSqlFunc::EDataType::edtString;
}

void CCtrlOracle::TParmOra::SetClob(Connection& conn, Statement& stmt, unsigned iPos, string strVal)
{
    // 创建临时表空间，用于clob
    Statement *tmpStmt = conn.createStatement("begin dbms_lob.createtemporary(:p1,TRUE);end;");
    CAutoRelease _auto([&](){ conn.terminateStatement (tmpStmt); });
    tmpStmt->registerOutParam(1, OCCICLOB);
    tmpStmt->executeUpdate();
    Clob clob = tmpStmt->getClob(1);
    // 退出后，释放临时表空间
    CAutoRelease _auto2([&](){
        // clob.setEmpty();
        Statement *tmp2Stmt = conn.createStatement("begin dbms_lob.freetemporary(:p1);end;");
        CAutoRelease _auto3([&](){ conn.terminateStatement (tmp2Stmt); });
        tmp2Stmt->setClob(1, clob);
        tmp2Stmt->executeUpdate();
    });
    // 设置字符类型，避免中文乱码
    clob.setCharSetForm(OCCI_SQLCS_IMPLICIT);
    // 填入值
    if (!strVal.empty())
        clob.write(static_cast<unsigned>(strVal.size()), (unsigned char*)strVal.c_str(), static_cast<unsigned>(strVal.size()));
    stmt.setClob(iPos, clob);
    /*
    if (CSqlFunc::EParmType::eptInOutParm == ePTp)
        m_own.stmt.setMaxParamSize(iPos, 1024 * 1024 * 30);     // 参数允许的最大宽度
    m_own.stmt.setString(iPos, strVal.substr(0, 1000 * 910));     // 字符串允许的最大宽度
    */
}
void CCtrlOracle::TParmOra::SetClob(string strVal)
{
    SetClob(m_own.conn, m_own.stmt, iPos, strVal);
    eDTp = CSqlFunc::EDataType::edtClob;
}

void CCtrlOracle::TParmOra::SetNull(void)
{
    m_own.stmt.setNull(iPos, OCCISTRING);
}

// 获取参数值
bool CCtrlOracle::TParmOra::IsNull(void)
{
    bIsNull = m_own.stmt.isNull(iPos);
    return bIsNull;
}

string CCtrlOracle::TParmOra::GetString(string sDef)
{
    if (IsNull()) return sDef;
    string sResult = m_own.stmt.getString(iPos);
    if ("ZHS16GBK" == m_own.m_ctrl.m_Charset) sResult = CUtilFunc::GbkToUtf8(sResult);
    return sResult;
}

string CCtrlOracle::TParmOra::GetClob(Statement& stmt, unsigned iPos, string sDef)
{
    Clob clob = stmt.getClob(iPos);
    if (clob.isNull()) return sDef;
    else
    {
        // 缓冲区
        unsigned iLen = clob.length();
        char* pBuf = new char[iLen + 1] {0};
        boost::shared_array<char> autoDel(pBuf);
        // 设置字符类型，避免中文乱码
        clob.setCharSetForm(OCCI_SQLCS_IMPLICIT);
        // 读取clob中的数据
        string sResult = "";
        clob.open();
        CAutoRelease _auto([&](){ clob.close(); });
        Stream* strm = clob.getStream();
        for (int c = 1; c > 0; )
        {
            memset(pBuf, 0, iLen);
            c = strm->readBuffer(pBuf, iLen);
            if (c > 0) sResult.append(pBuf, c);
        }
        return sResult;
    }
    /*
    if (IsNull()) return sDef;
    return m_own.stmt.getString(iPos);
    */
}
string CCtrlOracle::TParmOra::GetClob(string sDef)
{
    string sResult = GetClob(m_own.stmt, iPos, sDef);
    if ("ZHS16GBK" == m_own.m_ctrl.m_Charset) sResult = CUtilFunc::GbkToUtf8(sResult);
    return sResult;
}

string CCtrlOracle::TParmOra::GetDate(string fmt)
{
    if (IsNull()) return "";
    if (fmt.empty()) fmt = "yyyy-MM-dd";
    return m_own.stmt.getDate(iPos).toText(fmt);
}

string CCtrlOracle::TParmOra::GetDateTime(string fmt)
{
    if (IsNull()) return "";
    if (fmt.empty()) fmt = "yyyy-MM-dd hh:mm:ss";
    return m_own.stmt.getDate(iPos).toText(fmt);
}

int CCtrlOracle::TParmOra::GetInt(int iDef)
{
    if (IsNull()) return iDef;
    return m_own.stmt.getInt(iPos);
}

double CCtrlOracle::TParmOra::GetNumber(double fDef)
{
    if (IsNull()) return fDef;
    return m_own.stmt.getNumber(iPos);
}

// 获取游标数据集
CSqlFunc::TRecordSetPtr CCtrlOracle::TParmOra::GetDbSet(void)
{
    try
    {
        return CSqlFunc::TRecordSetPtr(new TRecordSetOra(m_own.m_ctrl, m_own.keepStmt, *m_own.stmt.getCursor(iPos), m_own.m_objCtrlD));
    }
    catch (SQLException& ex)
    {
        string sErr = ex.getMessage() + "\n" + m_own.m_objCtrlD.ActionData().GetSingleInfo("the_request") + " \t\t" + c_RESTful_outParm + ": " + sName;
        m_own.m_own.WriteLogWarning(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        return CSqlFunc::TRecordSetPtr();
    }
}

void CCtrlOracle::TParmOra::SetDbTp(void)
{
    if (eDTp == CSqlFunc::EDataType::edtInt) eDbTP = OCCIINT;
    else if (eDTp == CSqlFunc::EDataType::edtNumber) eDbTP = OCCINUMBER;
    else if (eDTp == CSqlFunc::EDataType::edtDatetime) eDbTP = OCCIDATE;
    else if (eDTp == CSqlFunc::EDataType::edtDbSet) eDbTP = OCCICURSOR;
    else if (eDTp == CSqlFunc::EDataType::edtDbSetArray) eDbTP = OCCICURSOR;
    else if (eDTp == CSqlFunc::EDataType::edtDbSetOne) eDbTP = OCCICURSOR;
    else if (eDTp == CSqlFunc::EDataType::edtClob) eDbTP = OCCICLOB;
    //else if (eDTp == CSqlFunc::EDataType::edtClob) eDbTP = OCCISTRING;
    else if (eDTp == CSqlFunc::EDataType::edtUnknown)
    {
        // 未指定类型，通过值类型
        if (anyVal.type() == typeid(int) || anyVal.type() == typeid(long long))
        {
            eDbTP = OCCIINT;
            eDTp = CSqlFunc::EDataType::edtInt;
        }
        else if (anyVal.type() == typeid(double))
        {
            eDbTP = OCCINUMBER;
            eDTp = CSqlFunc::EDataType::edtNumber;
        }
        else
        {
            eDbTP = OCCISTRING;
            eDTp = CSqlFunc::EDataType::edtString;
        }
    }
    else eDbTP = OCCISTRING;
}

void CCtrlOracle::TParmOra::RegOutParam(void)
{
    if (OCCISTRING == eDbTP)
        m_own.stmt.registerOutParam(iPos, eDbTP, c_KCMaxParmBufSize);
    else
        m_own.stmt.registerOutParam(iPos, eDbTP);
}

// CCtrlOracle::TParmGrpOra类
CCtrlOracle::TParmGrpOra::TParmGrpOra(CCtrlOracle& ctrl, KeepStmtOraPtr st, ICtrlApiData& ctrlD, string sAct, bool bMultiParm)
    : TParmGrpOraBase(false, bMultiParm, false), m_ctrl(ctrl), m_own(ctrl.m_own), keepStmt(st), conn(st->conn), stmt(st->stmt), m_objCtrlD(ctrlD), m_act(sAct)
{
}

// 所属的服务
IKCSql& CCtrlOracle::TParmGrpOra::Srv(void)
{
    return m_own;
}

//  控制器 信息
string CCtrlOracle::TParmGrpOra::ActInfo(void)
{
    return m_act;
}

// 创建参数
CSqlFunc::TParmPtr CCtrlOracle::TParmGrpOra::MakePram(std::string s, unsigned p)
{
    return CSqlFunc::TParmPtr(new TParmOra(*this, s, p));
}

// 异常信息
string CCtrlOracle::TParmGrpOra::ParmExceptInfo(SQLException& ex)
{
    return ex.getMessage();
}

// CCtrlOracle::TRecordSetOra类
CCtrlOracle::TRecordSetOra::TRecordSetOra(CCtrlOracle& ctrl, KeepStmtOraPtr st, ResultSet& rs, ICtrlApiData& ctrlD)
    : m_ctrl(ctrl), keepStmt(st), stmt(st->stmt), rset(rs), m_objCtrlD(ctrlD)
{
    if (ctrl.m_prefetchRowCount >= 0) rset.setPrefetchRowCount(ctrl.m_prefetchRowCount);
    // rset.setPrefetchMemorySize(1024 * 1024 * 100);
}
CCtrlOracle::TRecordSetOra::~TRecordSetOra(void)
{
    stmt.closeResultSet(&rset);
}

// 获取字段信息
bool CCtrlOracle::TRecordSetOra::GetColumnListMetaDataBeforeFetch(CSqlFunc::TFields &vecFeilds)
{
    map<string, CSqlFunc::TFieldPtr> mapFeilds;
    vector<MetaData> metafields = rset.getColumnListMetaData();
    for (unsigned i = 0, c = static_cast<unsigned>(metafields.size()); i < c; ++i)
    {
        // 列属性值
        string sColName = metafields[i].getString(MetaData::ATTR_NAME);
        int iColSize = metafields[i].getInt(MetaData::ATTR_DATA_SIZE);
        int iColType = metafields[i].getInt(MetaData::ATTR_DATA_TYPE);
        auto eDataType = CCtrlOracle::GetTypeName(iColType);
        //cout << sColName << "\t" << iColType << "\t" << iColSize << endl;
        // 字段名
        // string sFieldName = algorithm::to_lower_copy(sColName);
        string sFieldName = sColName;
        if ("lower" == m_ctrl.m_feildCaseSensitive)
            algorithm::to_lower(sFieldName);
        else if ("upper" == m_ctrl.m_feildCaseSensitive)
            algorithm::to_upper(sFieldName);
        // 插入
        CSqlFunc::TField *pFeild = new CSqlFunc::TField(sFieldName, i + 1, iColType, eDataType);
        if ("ZHS16GBK" == m_ctrl.m_Charset) pFeild->m_nameShow = CUtilFunc::GbkToUtf8(sFieldName);
        mapFeilds.insert(make_pair(sFieldName, std::shared_ptr<CSqlFunc::TField>(pFeild)));
        // 设置空值的列宽（解除ORA-32108错误）
        if (0 == iColSize)
            rset.setMaxColumnSize(i + 1, 1);
        // 字符串类型的列，列宽翻倍（防止中文被截断）。GBK类型的字段字符串，按UTF-8返回时，因列宽不够，中文会被截断。
        else if (SQLT_AFC == iColType || SQLT_AVC == iColType || SQLT_CHR == iColType || SQLT_VCS == iColType)
        {
            // string sset = rset.getCharSet(i + 1);
            // cout << sset << endl;
            // rset.setCharSet(i + 1, m_ctrl.m_Charset);

            // if (SQLT_CHR == iColType || SQLT_AFC == iColType) rset.setDatabaseNCHARParam(i + 1, true);

            rset.setMaxColumnSize(i + 1, iColSize * 3 + 6);
        }
    }
    if (vecFeilds.empty())
        //for (auto it : mapFeilds) vecFeilds.push_back(it);
        for (auto &md : metafields)
        {
            // string sFieldName = algorithm::to_lower_copy(md.getString(MetaData::ATTR_NAME));
            string sFieldName = md.getString(MetaData::ATTR_NAME);
            if ("lower" == m_ctrl.m_feildCaseSensitive)
                algorithm::to_lower(sFieldName);
            else if ("upper" == m_ctrl.m_feildCaseSensitive)
                algorithm::to_upper(sFieldName);
            auto it = mapFeilds.find(sFieldName);
            if (mapFeilds.end() != it) vecFeilds.push_back(it->second);
        }
    else
        for (auto &fd : vecFeilds)
        {
            auto it = mapFeilds.find(fd->m_name);
            if (mapFeilds.end() != it) fd = it->second;
        }
    return true;
}
// 下一条
bool CCtrlOracle::TRecordSetOra::Next(void)
{
    return rset.next() != ResultSet::END_OF_FETCH;
}
// 添加值
void CCtrlOracle::TRecordSetOra::AddVal(IKCJson& jsonRow, CSqlFunc::TField& fld)
{
    // 字段值
    if (fld.m_pos > 0 && !rset.isNull(fld.m_pos))
        switch (fld.m_type)
        {
        case SQLT_NUM:
            jsonRow.AddVal(fld.m_nameShow.c_str(), rset.getDouble(fld.m_pos));
            break;
        case SQLT_LNG:
        case SQLT_INT:
            jsonRow.AddVal(fld.m_nameShow.c_str(), rset.getInt(fld.m_pos));
            break;
        case SQLT_DAT:
            jsonRow.AddStr(fld.m_nameShow.c_str(), rset.getDate(fld.m_pos).toText("YYYY-MM-DD HH24:MI:SS").c_str());
            break;
        case SQLT_BLOB:
        case SQLT_CHR:
        case SQLT_STR:
        default:
            {
                /*
                rset.setBinaryStreamMode(iPos, 200);
                Stream *p = rset.getStream(iPos);
                char buffer[200] = {0};
                p->readLastBuffer(buffer, 200);

                UString str = rset.getUString(iPos);
                string str2 = rset.getString(iPos);
                string str3 = CUtilFunc::GbkToUtf8(str2);
                */

                // int iSize = rset.getMaxColumnSize(fld.m_pos);
                //if (rset.getMaxColumnSize(iPos) < 5) rset.setMaxColumnSize(iPos, 500);

                string strValue = rset.getString(fld.m_pos);
                if ("ZHS16GBK" == m_ctrl.m_Charset) strValue = CUtilFunc::GbkToUtf8(strValue);
                jsonRow.AddStr(fld.m_nameShow.c_str(), strValue.c_str());
                break;
            }
        }
    //else jsonRow.AddStr(sName.c_str(), "");
    else jsonRow.AddNull(fld.m_nameShow.c_str());
}
// 得到值
CSqlFunc::TKcValVariant CCtrlOracle::TRecordSetOra::GetVal(CSqlFunc::TField& fld) const
{
    CSqlFunc::TKcValVariant result;
    // 字段值
    if (fld.m_pos > 0 && !rset.isNull(fld.m_pos))
        switch (fld.m_type)
        {
        case SQLT_NUM:
            result = rset.getDouble(fld.m_pos);
            break;
        case SQLT_LNG:
        case SQLT_INT:
            result = rset.getInt(fld.m_pos);
            break;
        case SQLT_DAT:
            result = rset.getDate(fld.m_pos).toText("YYYY-MM-DD HH24:MI:SS");
            break;
        case SQLT_BLOB:
        case SQLT_CHR:
        case SQLT_STR:
        default:
            {
                string strValue = rset.getString(fld.m_pos);
                if ("ZHS16GBK" == m_ctrl.m_Charset) strValue = CUtilFunc::GbkToUtf8(strValue);
                result = strValue;
                break;
            }
        }
    return result;
}

// CCtrlOracle::TDBCommandOra类
CCtrlOracle::TDBCommandOra::TDBCommandOra(CCtrlOracle& ctrl, KeepStmtOraPtr st, ICtrlApiData& ctrlD, string sAct, string sActName, string sMethod, TOraConnObj& connObj)
    : m_own(ctrl.m_own), m_ctrl(ctrl), keepStmt(st), conn(st->conn), stmt(st->stmt), m_objCtrlD(ctrlD), m_act(sAct), m_actName(sActName), m_method(sMethod), m_connObj(connObj)
    , m_batchIsProc(c_RESTful_batch == sMethod && CCtrlCommon::IsSQLProcedures(stmt.getSQL()))
    , m_parms(ctrl, st, m_objCtrlD, sAct, c_RESTful_select == sMethod || c_RESTful_insert == sMethod || c_RESTful_query == sMethod || (c_RESTful_batch == sMethod && !m_batchIsProc))
{
}

//  控制器 信息
string CCtrlOracle::TDBCommandOra::ActInfo(void)
{
    return m_act;
}
string CCtrlOracle::TDBCommandOra::ActName(void)
{
    return m_actName;
}

// 事务
void CCtrlOracle::TDBCommandOra::TranBegin(void)
{
    stmt.setAutoCommit(false);
}
void CCtrlOracle::TDBCommandOra::TranCommit(void)
{
    stmt.getConnection()->commit();
    stmt.setAutoCommit(true);
}
void CCtrlOracle::TDBCommandOra::TranRollback(void)
{
    stmt.getConnection()->rollback();
    stmt.setAutoCommit(true);
}

// 预执行
void CCtrlOracle::TDBCommandOra::PrepareSQL(void)
{
    string sSQL = stmt.getSQL();
    if (sSQL.empty())
        throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 5, __CURR_CODE_PLACE_C__, string(m_own.getHint("SQL_Empty_")) + m_objCtrlD.ActionData().GetSingleInfo("the_request") + " - " + m_actName, m_own);
    // DDL语句，创建临时对象
    if (CreateTmpParmPackage()) ;
    // 非DDL语句
    else
    {
        // 执行非DDL语句，替换临时对象名
        sSQL = ReplaceTmpObjName(sSQL);
        // 更新sql语句
        if (!m_parms.mapTmpTabs.empty()) stmt.setSQL(sSQL);
    }
}

// 单独执行sql语句
void CCtrlOracle::TDBCommandOra::ExecSQL(const char* sql)
{
    m_connObj.ExecSQL(sql);
}

// 针对DDL语句，创建参数临时包
bool CCtrlOracle::TDBCommandOra::CreateTmpParmPackage(void)
{
    const bool bResult = c_RESTful_create == m_method;
    if (bResult)
        stmt.setSQL(CreateTmpObjAndParmPack(stmt.getSQL()));
    return bResult;
}

// 临时对象的随机名称
string CCtrlOracle::TDBCommandOra::GetTmpObjRandName(string sName)
{
    if (m_ctrl.m_version < 12.2)
    {
        // 天数
        boost::gregorian::date dtToday = boost::gregorian::day_clock::local_day();
        boost::gregorian::date dtBase(2015, 11, 6);     // 鸣野创建
        int days = abs((dtToday - dtBase).days());
        // 根据数据库版本，设置标识符最大长度
        unsigned sIdnLen = 30;
        // 临时包名称
        return (format("Z%x$%x_%X_%X_%s") % days % boost::this_process::get_id() % m_connObj.m_connid % (++m_connObj.m_tmpID) % sName).str().substr(0, sIdnLen);
    }
    else return CSqlFuncOra::TDBCommandOraBase<SQLException>::GetTmpObjRandName(sName);
}

// 执行
unsigned int CCtrlOracle::TDBCommandOra::ExecuteUpdate(void)
{
    // 执行主sql
    return stmt.executeUpdate();
}
// 批量操作
pair<int, string> CCtrlOracle::TDBCommandOra::ExecuteBatch(void)
{
    try
    {
        /*
            // 创建环境与连接
            Environment* env = Environment::createEnvironment("ZHS16GBK", "UTF8");
            Connection* conn = env->createConnection("scott", "tiger", "127.0.0.1/orclxxx");
            // 定义SQL语句并绑定参数
            Statement* stmt = conn->createStatement("INSERT INTO TEST VALUES(:1, :2, :3)");
            stmt->setSQL("INSERT INTO TEST VALUES(:1, :2, :3)");
            stmt->setMaxIterations(OCCI_MAX_ARRAY_SIZE); // 设置最大迭代次数为数组最大容量
            stmt->setMaxParamSize(1U, 64U); // 设置第一个参数最大内存占用64字节
            stmt->setMaxParamSize(2U, 20U); // 设置第二个参数最大内存占用20字节
            // 模拟批量数据插入（每秒5000条）
            for(int i = 0; i < OCCI_MAX_ARRAY_SIZE; ++i) {
                stmt->setString(1U, "value1");
                stmt->setInt(2U, i);
                stmt->setDate(3U, date[i]);
                if (i < OCCI_MAX_ARRAY_SIZE - 1) {
                    stmt->addIteration(); // 添加迭代
                }
            }
            stmt->executeArrayUpdate(); // 执行批量操作
            conn->commit(); // 提交事务
        */
        return make_pair(m_batchIsProc ? ExecuteProcedures() : ExecuteUpdate(), "");
    }
    catch (SQLException& ex)
    {
        return make_pair(ex.getErrorCode(), ex.getMessage());
    }
}

// 查询
CSqlFunc::TRecordSetPtr CCtrlOracle::TDBCommandOra::ExecuteQuery(int &rows_affected)
{
    rows_affected = -1;
    if (m_ctrl.m_prefetchRowCount >= 0) stmt.setPrefetchRowCount(m_ctrl.m_prefetchRowCount);
    // stmt.setPrefetchMemorySize(1024 * 1024 * 100);
    return CSqlFunc::TRecordSetPtr(new TRecordSetOra(m_ctrl, keepStmt, *stmt.executeQuery(), m_objCtrlD));
}

// 执行过程
int CCtrlOracle::TDBCommandOra::ExecuteProcedures(void)
{
    return stmt.execute();
}

// CCtrlOracle::TOraConnObj类
CCtrlOracle::TOraConnObj::TOraConnObj(CCtrlOracle& own, unsigned connid)
    : CSqlFuncOra::TOraConnObjBase(connid), m_own(own)
{
    try
    {
        m_own.m_own.WriteLogDebug("Create Oracle Connect Object", __CURR_CODE_PLACE_C__, (format("Connect ID: %d") % m_connid).str().c_str());
        // 从连接池里获取链接
        if (m_own.m_usePool && nullptr != m_own.m_connPool)
            m_conn = m_own.m_connPool->getConnection();
        // 创建链接
        else
            m_conn = m_own.m_env.createConnection(m_own.m_user, m_own.m_pass, m_own.m_srv);
        if (nullptr == m_conn)
            throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 5, __CURR_CODE_PLACE_C__, m_own.m_own.getHint("Database_Connection_Error_") + m_own.m_srv, m_own.m_own);
        m_own.m_own.WriteLogDebug("Connect Oracle Success", __CURR_CODE_PLACE_C__, (format("[Connect Addr: %X]%s/%s@%s\n%s/%s/%s/%s") % m_conn % m_own.m_user % m_own.m_pass % m_own.m_srv
                                                                           % m_conn->getClientCharSet() % m_conn->getTag() % m_conn->getServerVersion() % m_conn->getServerVersion()).str().c_str());
    }
    catch (SQLException& ex)
    {
        m_own.m_own.WriteLogFatal((ex.getMessage() + " \n" + m_own.m_srv).c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
}

// 单独执行sql语句
void CCtrlOracle::TOraConnObj::ExecSQL(const char* sql)
{
    if (nullptr == m_conn)
        throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 5, __CURR_CODE_PLACE_C__, m_own.m_own.getHint("Database_Connection_Error_") + m_own.m_srv, m_own.m_own);
    Statement *stmtTmp = m_conn->createStatement();
    if (nullptr != stmtTmp)
    {
        CAutoRelease _auto([&](){ m_conn->terminateStatement(stmtTmp); });
        stmtTmp->execute(sql);
    }
}

// 释放
void CCtrlOracle::TOraConnObj::Release(void)
{
    try
    {
        m_own.m_own.WriteLogDebug("Remove Oracle Connect Object", __CURR_CODE_PLACE_C__, (format("Connect ID: %d") % m_connid).str().c_str());
        // 删除临时对象
        DropTmpObj();
        // 释放链接
        if (m_own.m_usePool && nullptr != m_own.m_connPool)
            m_own.m_connPool->releaseConnection(m_conn);
        else
            m_own.m_env.terminateConnection(m_conn);
        m_conn = nullptr;
        // 删除自己
        delete this;
    }
    catch (std::exception& ex)
    {
        m_own.m_own.WriteLogFatal(ex.what(), __CURR_CODE_PLACE_C__, typeid(ex).name());
    }
    catch (...)
    {
        m_own.m_own.WriteLogFatal(Srv().getHint("Unknown_exception"), __CURR_CODE_PLACE_C__);
    }
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlOracle类
CCtrlOracle::CCtrlOracle(IKCSqlOracle& own, string sName, property_tree::ptree& pt)
    : m_own(own), m_name(sName)
    , m_Charset(boost::algorithm::to_upper_copy(CUtilFunc::GetXmlAttrValue(pt, "<xmlattr>.charset", "AL32UTF8")))
    , m_NCharset(boost::algorithm::to_upper_copy(CUtilFunc::GetXmlAttrValue(pt, "<xmlattr>.ncharset", "AL32UTF8")))
    , m_feildCaseSensitive(algorithm::to_lower_copy(CUtilFunc::GetXmlAttrValue(pt, "<xmlattr>.feildCaseSensitive", "")))
    , m_prefetchRowCount(std::min(atoi(CUtilFunc::GetXmlAttrValue(pt, "<xmlattr>.prefetchRowCount", "128").c_str()), 10240))
    , m_env(CreateOracleEnv(pt)), m_connPool(CreateConnPool(pt)), m_connID(0)
{
    // 记录函数地址
    m_own.WriteLogDebug((m_name + ": "
                            + "\nCCtrlOracle::Exce: \t\t" + CUtilFunc::GetFuncAddr(&CCtrlOracle::Perform)
                       ).c_str());
}
CCtrlOracle::~CCtrlOracle()
{
    // 释放数据库链接池
    if (m_usePool) m_env.terminateStatelessConnectionPool(m_connPool);
}

// 获取类型名
CSqlFunc::EDataType CCtrlOracle::GetTypeName(int sqlt)
{
    switch (sqlt)
    {
    case OCCIDOUBLE:
    case OCCINUMBER:
    case OCCI_SQLT_NUM:
        return CSqlFunc::EDataType::edtNumber;
        break;
    case OCCIINT:
        return CSqlFunc::EDataType::edtInt;
        break;
    case OCCIDATE:
    case OCCITIMESTAMP:
    case OCCI_SQLT_DAT:
        return CSqlFunc::EDataType::edtDatetime;
        break;
    case OCCICURSOR:
        return CSqlFunc::EDataType::edtDbSet;
        break;
    case OCCIBLOB:
    case OCCICLOB:
        return CSqlFunc::EDataType::edtClob;
        break;
    case OCCISTRING:
    case OCCICHAR:
    default:
        return CSqlFunc::EDataType::edtString;
        break;
    }
}

// 创建oracle环境
Environment& CCtrlOracle::CreateOracleEnv(property_tree::ptree& pt)
{
    try
    {
        // 创建oracle环境变量
        m_own.WriteLogDebug("Create Oracle Environment", __CURR_CODE_PLACE_C__, (m_name + ": " + m_Charset + ", " + m_NCharset).c_str());
        //return *Environment::createEnvironment(Environment::DEFAULT);
        Environment *env = Environment::createEnvironment(m_Charset.c_str(), m_NCharset.c_str(), Environment::THREADED_MUTEXED);
        if (nullptr == env)
            throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 6, __CURR_CODE_PLACE_C__, m_own.getHint("Create_Fail_") + string("Oracle Environment"), m_own);
        //env->setNLSTerritory("CHINA");
        //env->setNLSLanguage("SIMPLIFIED CHINESE");
        env->setNLSTerritory("AMERICA");
        env->setNLSLanguage("AMERICAN");
        return *env;
    }
    catch (SQLException& ex)
    {
        string sErr = (format("[%s][%d]%s") % m_name % ex.getErrorCode() % ex.getMessage()).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        string sErr = (format("[%s][%d]%s") % m_name % ex.error_id() % ex.error_info()).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
    catch (std::exception& ex)
    {
        string sErr = (format("[%s][219]%s") % m_name % ex.what()).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
    catch (...)
    {
        string sErr = (format("[%s][218]%s") % m_name % m_own.getHint("Unknown_exception")).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        throw;
    }
}

// 创建连接池
StatelessConnectionPool* CCtrlOracle::CreateConnPool(property_tree::ptree& pt)
{
    string sConnStr = "";
    try
    {
        // 是否加密连接串
        if (pt.get_child_optional("<xmlattr>.encrypt"))
            m_encryptConnStr = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.encrypt")) == "true";
        // 连接串
        if (pt.get_child_optional("<xmlattr>.str"))
        {
            sConnStr = pt.get<string>("<xmlattr>.str");
            if (m_encryptConnStr) sConnStr = CCtrlCommon::DecryptConnStrDB(m_own.getContext(), sConnStr);
            vector<string> strVec;
            algorithm::split(strVec, sConnStr, is_any_of("@/"));
            if (strVec.size() < 3)
                throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 7, __CURR_CODE_PLACE_C__, m_own.getHint("Database_Connection_Error_") + sConnStr, m_own);
            m_user = strVec[0];
            m_pass = strVec[1];
            m_srv = strVec.size() > 3 ? strVec[2] + "/" + strVec[3] : strVec[2];
        }
        // 最大连接数
        if (pt.get_child_optional("<xmlattr>.maxConnNum"))
            try
            {
                m_maxConnNum = pt.get<int>("<xmlattr>.maxConnNum");
            }
            catch (...) {}
        if (m_maxConnNum <= 0) m_maxConnNum = 512;
        // Oracle版本
        if (pt.get_child_optional("<xmlattr>.version"))
            try
            {
                m_version = pt.get<double>("<xmlattr>.version");
            }
            catch (...) {}
        if (m_version <= 0) m_version = 19;
        m_own.WriteLogInfo((format("%s%s\n%s\t%f(%d)") % m_own.getHint("Database_Connection_") % m_name % sConnStr % m_version % m_maxConnNum).str().c_str(), __CURR_CODE_PLACE_C__);
        // 使用链接池
        if (pt.get_child_optional("<xmlattr>.usePool"))
            m_usePool = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.usePool")) == "true";
        if (m_usePool)
        {
            // 创建连接池
            StatelessConnectionPool *pool = m_env.createStatelessConnectionPool(m_user, m_pass, m_srv, m_maxConnNum, 0, 16, StatelessConnectionPool::HETEROGENEOUS);
            if (nullptr == pool)
                throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 8, __CURR_CODE_PLACE_C__, string(m_own.getHint("Database_Connection_Error_")) + " - " + m_user + "/" + m_pass + "@" + m_srv, m_own);
            pool->setTimeOut(3000);
            pool->setBusyOption(StatelessConnectionPool::FORCEGET);
            return pool;
        }
    }
    catch (SQLException& ex)
    {
        string sErr = (format("[%s][%d]%s\n%s \t%d") % m_name % ex.getErrorCode() % ex.getMessage() % sConnStr % m_maxConnNum).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        if (m_usePool) throw;
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        string sErr = (format("[%s][%d]%s\n%s \t%d") % m_name % ex.error_id() % ex.error_info() % sConnStr % m_maxConnNum).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        if (m_usePool) throw;
    }
    catch (std::exception& ex)
    {
        string sErr = (format("[%s][217]%s\n%s \t%d") % m_name % ex.what() % sConnStr % m_maxConnNum).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        if (m_usePool) throw;
    }
    catch (...)
    {
        string sErr = (format("[%s][216]%s\n%s \t%d") % m_name % m_own.getHint("Unknown_exception") % sConnStr % m_maxConnNum).str();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        if (m_usePool) throw;
    }
    return nullptr;
}

// 执行控制器
void CCtrlOracle::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pSQL = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sLastSQL, sJsonParm, sAct = CUtilFunc::PCharSafeToStr(pAct);
    string sSrvFlag = m_name + "@" + CUtilFunc::PCharSafeToStr(m_own.getAlias(), "kc_sql_oracle");
    Connection *conn = nullptr;
    unsigned iConnID = 0;
    Statement *stmt = nullptr;
    auto fExceptInfo = [&](void)
    {
        return (format("%s%s%s%s%s%s\n%s\n\n%s?act=%s\n[Connect: %d-%X][Statement: %X] \t")
                % c_strLineShow % sLastSQL % c_strLineShow % CUtilFunc::PCharSafeToStr(pSQL) % c_strLineShow
                % act.GetSingleInfo("the_request") % (sJsonParm.size() < 4096 ? sJsonParm : (sJsonParm.substr(0, 4096) + " ..."))
                % pLocalFile % sAct % iConnID % conn % stmt).str();
    };
    try
    {
        // SQL语句
        // if (nullptr == pSQL || strlen(pSQL) == 0)
        //     throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 10, __CURR_CODE_PLACE_C__, string(m_own.getHint("SQL_Empty_")) + act.GetSingleInfo("the_request") + " - " + sAct, m_own);
        string sSQL = CUtilFunc::PCharSafeToStr(pSQL);
        // 参数
        //sJsonParm = objCtrlD.GetPosStr();
        sJsonParm = objCtrlD.JsonRequest().ToStr();
        cout << "\t" << CUtilFunc::Utf8ToGbk(sJsonParm).substr(0, 500) << endl;
        const char* pKCCLNID = act.GetGlobalVal("KC__SESSION__KCCLNID");
        m_own.WriteLogTrace(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (sSQL.substr(0, 256) + "\r\n" + sJsonParm.substr(0, 256) + "\r\n" + pLocalFile + "\t-\t" + sAct + "\npKCCLNID=" + (nullptr != pKCCLNID ? pKCCLNID : "")).c_str());
        // 方法类型
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, c_RESTful_exec));
        algorithm::to_lower(sMethod);
        // 获取数据库链接
        CSqlFunc::TConnObjWork<TOraConnObj> actObjWork(objCtrlD, /*sSrvFlag,*/ [&](){ return new TOraConnObj(*this, ++m_connID); });
        try
        {
            conn = actObjWork.ActObj().m_conn;
            // 创建数据库执行实例
            stmt = conn->createStatement();
            if (nullptr == stmt)
                throw TKCSqlOracleException(ecd_ErrCode_KCSqlOracle + 11, __CURR_CODE_PLACE_C__, string(m_own.getHint("Database_Connection_Error_")) + sAct + ".", m_own);
            // 执行
            {
                KeepStmtOraPtr keepStmt(new KeepStmtOra(*conn, *stmt));
                CAutoRelease _auto2([&](){ sLastSQL = stmt->getSQL(); });
                // 更改提示为英文
                stmt->execute(R"raw_str_literal(alter session set nls_language='american')raw_str_literal");
                // 移除注释
                sSQL = CCtrlCommon::RemoveComment(sSQL);
                // 如果数据库的字符集是GBK，则sql语句也要转换为GBK
                if ("ZHS16GBK" == m_Charset)
                    // sSQL = boost::locale::conv::from_utf(sSQL, "GBK");
                    sSQL = CUtilFunc::Utf8ToGbkThrow(sSQL);
                // 设置SQL语句
                stmt->setSQL(sSQL);
                // 数据库执行命令
                TDBCommandOra comm(*this, keepStmt, objCtrlD, pLocalFile + string("?act=") + sAct, sAct, sMethod, actObjWork.ActObj());
                // 查询（method="select"）
                if (c_RESTful_select == sMethod || c_RESTful_query == sMethod)
                    comm.Select();
                // 存储过程（method="procedures"）
                else if (c_RESTful_procedures == sMethod)
                    comm.Procedures();
                // 批量导入（method="batch"）
                else if (c_RESTful_batch == sMethod)
                    comm.BatchInsert();
                // 创建（method="create"）
                else if (c_RESTful_create == sMethod)
                    comm.CreateDDL();
                // 临时包（method="package"）
                else if (c_RESTful_package == sMethod)
                    comm.CreateTmpPackage(objCtrlD, sSQL);
                // 增删改（method="update", "insert", "delete"）
                else if (sMethod.empty() || c_RESTful_exec == sMethod || c_RESTful_insert == sMethod || c_RESTful_delete == sMethod || c_RESTful_update == sMethod)
                    comm.InsertDeleteUpdate();
                // 方法类型错误
                else throw TKCSqlException(ecd_ErrCode_KCSqlOracle + 10, __CURR_CODE_PLACE_C__, string(m_own.getHint("Request_Failed_")) + m_own.getHint("Method_Not_Allowed") + " [" + sMethod + "]", typeid(*this).name());
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
    catch (BatchSQLException& ex)
    {
        string sEx = ex.getMessage();
        for (int i = 0, c = ex.getFailedRowCount(); i < c; ++i)
            sEx += " \t" + ex.getException(i).getMessage();
        string sErr = sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        if (CUtilFunc::isGBK(sEx) && !CUtilFunc::isUtf8(sEx)) sEx = CUtilFunc::GbkToUtf8(sEx);
        objCtrlD.SetJsonRespond(ex.getErrorCode(), ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch (SQLException& ex)
    {
        string sEx = ex.getMessage();
        string sErr = sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        // 中文乱码
        //if (!CUtilFunc::isUtf8(sEx)) sEx = CUtilFunc::GbkToUtf8(sEx);
        //if (!CUtilFunc::is_str_utf8(sEx.c_str())) sEx = CUtilFunc::GbkToUtf8(sEx);
        if (CUtilFunc::isGBK(sEx) && !CUtilFunc::isUtf8(sEx)) sEx = CUtilFunc::GbkToUtf8(sEx);
        objCtrlD.SetJsonRespond(ex.getErrorCode(), ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch (TKCSqlException& ex)
    {
        string sEx = ex.error_info();
        string sErr = "[" + std::to_string(ex.error_id()) + "]" + ex.error_place() + sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iCode ? ecd_ErrCode_KCSqlOracle + 11 : iCode, ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        ex.OtherInfo() += fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        m_own.WriteLog(ex);
        int iErrCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iErrCode ? ecd_ErrCode_KCSqlOracle + 12 : iErrCode, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlOracle + 13, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlOracle + 14, ("[Controllers." + sAct + "] " + m_own.getHint("Unknown_exception") + "　(" + sSrvFlag + ")").c_str());
    }
}
