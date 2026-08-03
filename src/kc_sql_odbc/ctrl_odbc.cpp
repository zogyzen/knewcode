#include "ctrl_odbc.h"
#include "odbc_mssql.h"

////////////////////////////////////////////////////////////////////////////////
// TOdbcConnObj类
TOdbcConnObj::TOdbcConnObj(CCtrlODBC& own, string sConnName, unsigned connid)
    : m_own(own), m_connName(sConnName), m_odbc(QSqlDatabase::addDatabase("QODBC", sConnName.c_str())), m_connid(connid)
{
    m_odbc.setDatabaseName(m_own.m_connStr.c_str());
}
void TOdbcConnObj::Release(void)
{
    if (m_odbc.isOpen()) m_odbc.close();
    QSqlDatabase::removeDatabase(m_connName.c_str());
    delete this;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlODBC类
CCtrlODBC::CCtrlODBC(IKCSqlODBC& own, string sName, property_tree::ptree& pt)
    : m_own(own), m_name(sName), m_connID(0)
{
    // 记录函数地址
    m_own.WriteLogDebug((m_name + ": "
                            + "\nCCtrlODBC::GetConnParm: \t\t" + CUtilFunc::GetFuncAddr(&CCtrlODBC::GetConnParm)
                            + "\nCCtrlODBC::Exce: \t\t" + CUtilFunc::GetFuncAddr(&CCtrlODBC::Perform)
                       ).c_str());
    // 数据库连接参数
    GetConnParm(pt);
}
CCtrlODBC::~CCtrlODBC()
{
}

// 获取类型名
CSqlFunc::EDataType CCtrlODBC::GetTypeName(int tp)
{
    switch (tp)
    {
    case QMetaType::Double:
    case QMetaType::Float:
    case QMetaType::LongLong:
        return CSqlFunc::EDataType::edtNumber;
        break;
    case QMetaType::Int:
    case QMetaType::Short:
        return CSqlFunc::EDataType::edtInt;
        break;
    case QMetaType::QDate:
    case QMetaType::QDateTime:
    case QMetaType::QTime:
        return CSqlFunc::EDataType::edtDatetime;
        break;
    case QMetaType::QJsonValue:
    case QMetaType::QJsonObject:
    case QMetaType::QJsonArray:
        return CSqlFunc::EDataType::edtJson;
        break;
    case QMetaType::QCursor:
        return CSqlFunc::EDataType::edtDbSet;
        break;
    case QMetaType::QString:
    case QMetaType::QChar:
    default:
        return CSqlFunc::EDataType::edtString;
        break;
    }
}

// 数据库连接参数
void CCtrlODBC::GetConnParm(property_tree::ptree& pt)
{
    try
    {
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
        // 数据库类型
        if (pt.get_child_optional("<xmlattr>.dbType"))
            m_dbType = pt.get<string>("<xmlattr>.dbType");
        // 参数名称是否区分大小写
        if (pt.get_child_optional("<xmlattr>.parmCaseSensitive"))
            m_parmCaseSensitive = algorithm::to_lower_copy(pt.get<string>("<xmlattr>.parmCaseSensitive")) == "true";
    }
    catch (QSqlError& ex)
    {
        string sEx = ex.text().toStdString();
        if (!CUtilFunc::is_str_utf8(sEx.c_str())) sEx = CUtilFunc::GbkToUtf8(sEx);
        string sErr = sEx + "\n" + m_connStr;
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        throw;
    }
}

// 执行控制器
void CCtrlODBC::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pSQL = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sJsonParm, sOther, sAct = CUtilFunc::PCharSafeToStr(pAct);
    auto fExceptInfo = [&](void)
    {
        return (format("%s%s%s%s%s%s\n%s\n%s?act=%s\n")
                % c_strLineShow % CUtilFunc::PCharSafeToStr(pSQL) % c_strLineShow % sOther % c_strLineShow
                % act.GetSingleInfo("the_request") % sJsonParm % pLocalFile % sAct).str();
    };
    try
    {
        // SQL语句
        // if (nullptr == pSQL || strlen(pSQL) == 0)
        //     throw TKCSqlODBCException(ecd_ErrCode_KCSqlODBC + 4, __CURR_CODE_PLACE_C__, string(m_own.getHint("SQL_Empty_")) + act.GetSingleInfo("the_request") + " - " + sAct, m_own);
        string sSQL = CUtilFunc::PCharSafeToStr(pSQL);
        // 方法类型
        string sMethod = boost::algorithm::to_lower_copy(CUtilFunc::PCharSafeToStr(pMethod, c_RESTful_exec));
        algorithm::to_lower(sMethod);
        // 参数
        //sJsonParm = objCtrlD.GetPostStr();
        sJsonParm = objCtrlD.JsonRequest().ToStr();
        cout << "\t" << CUtilFunc::Utf8ToGbk(sJsonParm).substr(0, 500) << endl;
        const char* pKCCLNID = act.GetGlobalVal("KC__SESSION__KCCLNID");
        m_own.WriteLogTrace(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (sSQL.substr(0, 256) + "\r\n" + sJsonParm.substr(0, 256) + "\r\n" + pLocalFile + "\t-\t" + sAct + "\nKCCLNID=" + (nullptr != pKCCLNID ? pKCCLNID : "")).c_str());
        // 数据库链接
        string sActObjName = m_name + "@kc_sql_odbc-" + m_dbType;
        CSqlFunc::TConnObjWork<TOdbcConnObj> actObjWork(objCtrlD, /*sActObjName,*/ [&]()
        {
            string sConnName = sActObjName + "#" + std::to_string(++m_connID);
            return new TOdbcConnObj(*this, sConnName, m_connID);
        });
        try
        {
            QSqlDatabase &odbc = actObjWork.ActObj().m_odbc;
            if (!odbc.isOpen() && !odbc.open())
                throw TKCSqlODBCException(ecd_ErrCode_KCSqlODBC + 5, __CURR_CODE_PLACE_C__, string(m_own.getHint("Database_Connection_Error_")) + sAct + "\n" + odbc.lastError().text().toStdString(), m_own);
            // 移除注释
            sSQL = CCtrlCommon::RemoveComment(sSQL);
            // 数据库执行实例
            QSqlQuery query(odbc);
            //  预执行SQL语句
            if (!query.prepare(sSQL.c_str()))
                throw TKCSqlODBCException(ecd_ErrCode_KCSqlODBC + 6, __CURR_CODE_PLACE_C__, string(m_own.getHint("SQL_Error_")) + sAct + "\n" + odbc.lastError().text().toStdString(), m_own);
            // 数据库执行命令
            std::shared_ptr<CSqlFunc::TDBCommand<QSqlError>> commPtr;
            if (c_RESTful_DB_mssql == m_dbType)
                commPtr.reset(new TDBCommandODBCmssql(*this, actObjWork.ActObj(), query, objCtrlD, pLocalFile + string("?act=") + sAct, sSQL, sMethod, sOther));
            else
                throw TKCSqlODBCException(ecd_ErrCode_KCSqlODBC + 7, __CURR_CODE_PLACE_C__, m_own.getHint("Database_Type_Error_") + m_dbType, m_own);
            // 查询（method="query"）
            if (c_RESTful_select == sMethod || c_RESTful_query == sMethod)
                commPtr->Query();
            // 批量导入（method="batch"）
            else if (c_RESTful_batch == sMethod)
                commPtr->BatchInsert();
            // 创建（method="create"）
            else if (c_RESTful_create == sMethod)
                commPtr->CreateDDL();
            // 执行（method="exec"）
            else if (sMethod.empty() || c_RESTful_exec == sMethod || c_RESTful_insert == sMethod || c_RESTful_delete == sMethod || c_RESTful_update == sMethod || c_RESTful_procedures == sMethod)
                commPtr->Procedures();
            // 方法类型错误
            else throw TKCSqlODBCException(ecd_ErrCode_KCSqlODBC + 8, __CURR_CODE_PLACE_C__, string(m_own.getHint("Request_Failed_")) + m_own.getHint("Method_Not_Allowed") + " [" + sMethod + "]", typeid(*this).name());
        }
        catch (...)
        {
            // 异常时，关闭持久连接
            if ("on" == actObjWork.m_keepAlive || "first" == actObjWork.m_keepAlive)
                act.SetGlobalVal((c_RESTful_KCAct + string("") + c_RESTful_keepAlive).c_str(), "off");
            throw;
        }
    }
    catch (QSqlError& ex)
    {
        string sEx = ex.text().toStdString();
        string sErr = "[" + ex.nativeErrorCode().toStdString() + "]" + sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iCode = ex.type();
        objCtrlD.SetJsonRespond(0 == iCode ? ecd_ErrCode_KCSqlODBC + 9 : iCode, ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch (TKCSqlException& ex)
    {
        string sEx = ex.error_info();
        string sErr = "[" + std::to_string(ex.error_id()) + "]" + ex.error_place() + sEx + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        int iCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iCode ? ecd_ErrCode_KCSqlODBC + 10 : iCode, ("[Controllers." + sAct + "] " + sEx).c_str());
    }
    catch(TException& ex)
    {
        ex.OtherInfo() += fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        ex.LineCode() = __LINE__;
        m_own.WriteLog(ex);
        int iCode = ex.error_id();
        objCtrlD.SetJsonRespond(0 == iCode ? ecd_ErrCode_KCSqlODBC + 11 : iCode, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlODBC + 12, ("[Controllers." + sAct + "] " + ex.what()).c_str());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecd_ErrCode_KCSqlODBC + 13, ("[Controllers." + sAct + "] " + m_own.getHint("Unknown_exception") + " (ODBC)").c_str());
    }
}
