#include "odbc_mssql.h"
#include "ctrl_odbc.h"

////////////////////////////////////////////////////////////////////////////////
// TParmODBCmssql类
TParmODBCmssql::TParmODBCmssql(TParmGrpODBCmssql& o, std::string s, unsigned p) : m_own(o)
{
    iPos = p;
    sName = s;
}

// 设置参数值
bool TParmODBCmssql::SetNumber(double fVal)
{
    bool bResult = false;
    //if (CSqlFunc::EParmType::eptOutParm != ePTp && CSqlFunc::EParmType::eptInOutParm != ePTp)
    if (CSqlFunc::EParmType::eptOutParm != ePTp)
    {
        if (CUtilFunc::isInt(fVal))
        {
            m_own.m_qry.bindValue((":" + sName).c_str(), static_cast<int>(fVal), QSql::In);
            bResult = true;
        }
        else if (CUtilFunc::isLLong(fVal))
        {
            m_own.m_qry.bindValue((":" + sName).c_str(), static_cast<long long>(fVal), QSql::In);
            bResult = true;
        }
        else if (fabs(fVal) <= INT_MAX)
            m_own.m_qry.bindValue((":" + sName).c_str(), fVal, QSql::In);
        else
            m_own.m_qry.bindValue((":" + sName).c_str(), std::to_string(fVal).c_str(), QSql::In);
    }
    return bResult;
}

void TParmODBCmssql::SetInt(int iVal)
{
    //if (CSqlFunc::EParmType::eptOutParm != ePTp && CSqlFunc::EParmType::eptInOutParm != ePTp)
    if (CSqlFunc::EParmType::eptOutParm != ePTp)
        m_own.m_qry.bindValue((":" + sName).c_str(), static_cast<int>(iVal), QSql::In);
}

void TParmODBCmssql::SetString(string sVal)
{
    //if (CSqlFunc::EParmType::eptOutParm != ePTp && CSqlFunc::EParmType::eptInOutParm != ePTp)
    if (CSqlFunc::EParmType::eptOutParm != ePTp)
        m_own.m_qry.bindValue((":" + sName).c_str(), sVal.c_str(), QSql::In);
}
void TParmODBCmssql::SetClob(string sVal)
{
    this->SetString(sVal);
}

void TParmODBCmssql::SetNull(void)
{
    //if (CSqlFunc::EParmType::eptOutParm != ePTp && CSqlFunc::EParmType::eptInOutParm != ePTp)
    if (CSqlFunc::EParmType::eptOutParm != ePTp)
    {
        QVariant vNull;
        m_own.m_qry.bindValue((":" + sName).c_str(), vNull, QSql::In);
    }
}

// 获取参数值
bool TParmODBCmssql::IsNull(void)
{
    //QVariant val = m_own.m_qry.boundValue((":" + sName).c_str());
    //bIsNull = val.isNull();
    // 全局参数（从参数列表里取）
    if (m_own.CtrlD().ActionData().IsGlobalVal(sName.c_str()))
    {
        auto it = m_own.mapParms.find(sName);
        if (m_own.mapParms.end() != it)
            return it->second->bIsNull;
    }
    // 输出参数（直接取）
    else return bIsNull;
    return true;
}

string TParmODBCmssql::GetString(string sDef)
{
    /*
    QMap<QString, QVariant> m = m_own.m_qry.boundValues();
    QVariant val = m_own.m_qry.boundValue((":" + sName).c_str());
    bIsNull = val.isNull();
    if (bIsNull) return sDef;
    else return val.toString().toStdString();
    */
    // 全局参数（从参数列表里取）
    if (m_own.CtrlD().ActionData().IsGlobalVal(sName.c_str()))
    {
        auto it = m_own.mapParms.find(sName);
        if (m_own.mapParms.end() != it && !it->second->bIsNull)
            return BoostAnyToStdString(it->second->anyVal);
    }
    // 输出参数（直接取）
    else if (!bIsNull) return BoostAnyToStdString(anyVal);
    return sDef;
}
string TParmODBCmssql::GetClob(string sDef)
{
    return this->GetString(sDef);
}

string TParmODBCmssql::GetDate(string fmt)
{
    if (fmt.empty()) fmt = "yyyy-MM-dd";
    /*
    QMap<QString, QVariant> m = m_own.m_qry.boundValues();
    auto val = m_own.m_qry.boundValue((":" + sName).c_str());
    bIsNull = val.isNull();
    if (bIsNull) return "";
    return val.toDate().toString(fmt.c_str()).toStdString();
    */
    QVariant val;
    // 全局参数（从参数列表里取）
    if (m_own.CtrlD().ActionData().IsGlobalVal(sName.c_str()))
    {
        auto it = m_own.mapParms.find(sName);
        if (m_own.mapParms.end() != it && !it->second->bIsNull)
            val = QString(BoostAnyToStdString(it->second->anyVal).c_str());
    }
    // 输出参数（直接取）
    else if (!bIsNull) val = QString(BoostAnyToStdString(anyVal).c_str());
    if (!val.isValid() || val.isNull()) return "";
    return val.toDate().toString(fmt.c_str()).toStdString();
}

string TParmODBCmssql::GetDateTime(string fmt)
{
    if (fmt.empty()) fmt = "yyyy-MM-dd hh:mm:ss";
    /*
    QMap<QString, QVariant> m = m_own.m_qry.boundValues();
    auto val = m_own.m_qry.boundValue((":" + sName).c_str());
    bIsNull = val.isNull();
    if (bIsNull) return "";
    return val.toDateTime().toString(fmt.c_str()).toStdString();
    */
    QVariant val;
    // 全局参数（从参数列表里取）
    if (m_own.CtrlD().ActionData().IsGlobalVal(sName.c_str()))
    {
        auto it = m_own.mapParms.find(sName);
        if (m_own.mapParms.end() != it && !it->second->bIsNull)
            val = QString(BoostAnyToStdString(it->second->anyVal).c_str());
    }
    // 输出参数（直接取）
    else if (!bIsNull) val = QString(BoostAnyToStdString(anyVal).c_str());
    if (!val.isValid() || val.isNull()) return "";
    return val.toDateTime().toString(fmt.c_str()).toStdString();
}

int TParmODBCmssql::GetInt(int iDef)
{
    /*
    QMap<QString, QVariant> m = m_own.m_qry.boundValues();
    auto val = m_own.m_qry.boundValue((":" + sName).c_str());
    bIsNull = val.isNull();
    if (bIsNull) return iDef;
    return val.toInt();
    */
    // 全局参数（从参数列表里取）
    if (m_own.CtrlD().ActionData().IsGlobalVal(sName.c_str()))
    {
        auto it = m_own.mapParms.find(sName);
        if (m_own.mapParms.end() != it && !it->second->bIsNull)
            return static_cast<int>(CastBoostAnyToInt(it->second->anyVal));
    }
    // 输出参数（直接取）
    else if (!bIsNull) return static_cast<int>(CastBoostAnyToInt(anyVal));
    return iDef;
}

double TParmODBCmssql::GetNumber(double fDef)
{
    /*
    QMap<QString, QVariant> m = m_own.m_qry.boundValues();
    auto val = m_own.m_qry.boundValue((":" + sName).c_str());
    bIsNull = val.isNull();
    if (bIsNull) return fDef;
    return val.toDouble();
    */
    // 全局参数（从参数列表里取）
    if (m_own.CtrlD().ActionData().IsGlobalVal(sName.c_str()))
    {
        auto it = m_own.mapParms.find(sName);
        if (m_own.mapParms.end() != it && !it->second->bIsNull)
            return CastBoostAnyToFloat(it->second->anyVal);
    }
    // 输出参数（直接取）
    else if (!bIsNull) return CastBoostAnyToFloat(anyVal);
    return fDef;
}

// 获取临时表数据集
CSqlFunc::TRecordSetPtr TParmODBCmssql::GetDbSet(void)
{
    try
    {
        // 临时表名
        string sTmpName = "##" + this->sName;
        auto it = m_own.mapTmpTabs.find(sTmpName);
        if (m_own.mapTmpTabs.end() != it) sTmpName = it->second;
        // 读取临时表
        QSqlQuery qry(m_own.m_connObj.m_odbc);
        if (!qry.exec(("select * from " + sTmpName).c_str()))
            throw qry.lastError();
        return CSqlFunc::TRecordSetPtr(new TRecordSetODBCmssqlAlone(m_own.m_own, std::move(qry), m_own.m_objCtrlD));
    }
    catch (QSqlError& ex)
    {
        string sErr = ex.text().toStdString() + "\n" + m_own.CtrlD().ActionData().GetSingleInfo("the_request") + " \t\t" + c_RESTful_outParm + ": " + sName;
        m_own.m_own.Srv().WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        return CSqlFunc::TRecordSetPtr();
    }
}

void TParmODBCmssql::RegOutParam(void)
{
    /*
    QVariant val;
    if (eDTp == CSqlFunc::EDataType::edtInt) val = static_cast<int>(0);
    else if (eDTp == CSqlFunc::EDataType::edtNumber) val = static_cast<double>(0.0);
    else if (eDTp == CSqlFunc::EDataType::edtDatetime) val = QDateTime();
    else val = QString(127, ' ');
    m_own.m_qry.bindValue((":" + sName).c_str(), val, QSql::Out);
    */
}

// TParmGrpODBCmssql类
TParmGrpODBCmssql::TParmGrpODBCmssql(CCtrlODBC& own, TOdbcConnObj& db, QSqlQuery& q, ICtrlApiData& ctrlD, string sAct)
    : TParmGrpODBCmssqlBase(true), m_own(own), m_connObj(db), m_qry(q), m_objCtrlD(ctrlD), m_act(sAct)
{
}

// 所属的服务
IKCSql& TParmGrpODBCmssql::Srv(void)
{
    return m_own.Srv();
}

//  控制器 信息
string TParmGrpODBCmssql::ActInfo(void)
{
    return m_act;
}

// 创建参数
CSqlFunc::TParmPtr TParmGrpODBCmssql::MakePram(std::string s, unsigned p)
{
    return CSqlFunc::TParmPtr(new TParmODBCmssql(*this, s, p));
}

//  解析sql语句里的参数
void TParmGrpODBCmssql::ParseSqlParms(std::string, std::string)
{
    //CSqlFunc::TParmGrp::ParseSqlParms(sSQL, bMulti, sFmt);
    unsigned i = 1;
    const auto &m = m_qry.boundValueNames();
    for (auto &p : m)
    {
        qDebug() << p << "\n";
        std::string sParm = p.toStdString();
        sParm = sParm.substr(1, sParm.size() - 1);
        CSqlFunc::TParmPtr parmPtr = MakePram(sParm, i);
        mapParms.insert(std::make_pair(sParm, parmPtr));
        ++i;
    }
}

// 异常信息
string TParmGrpODBCmssql::ParmExceptInfo(QSqlError& ex)
{
    return ex.text().toStdString();
}

// TRecordSetODBCmssql类
TRecordSetODBCmssqlBase::TRecordSetODBCmssqlBase(CCtrlODBC& own, ICtrlApiData& ctrlD)
    : m_own(own), m_objCtrlD(ctrlD)
{
}

// 获取字段信息
void TRecordSetODBCmssqlBase::GetColumnListMetaData(CSqlFunc::TFields &vecFeilds)
{
    map<string, std::shared_ptr<CSqlFunc::TField>> mapFeilds;
    QSqlRecord rec = qry().record();
    unsigned iFieldCount = rec.count();
    for (unsigned i = 0; i < iFieldCount; ++i)
    {
        // 列属性值
        int iColType = rec.field(i).metaType().id();
        auto sTypeName = CCtrlODBC::GetTypeName(iColType);
        string sColName = rec.fieldName(i).toStdString();
        //cout << sColName << "\t" << iColType << "\t" << iColSize << endl;
        // 插入
        // 按配置区分大小写
        string sFieldName = m_own.m_parmCaseSensitive ? sColName : algorithm::to_lower_copy(sColName);
        mapFeilds.insert(make_pair(sFieldName, std::shared_ptr<CSqlFunc::TField>(new CSqlFunc::TField(sFieldName, i + 1, iColType, sTypeName))));
    }
    if (vecFeilds.empty())
        for (unsigned i = 0; i < iFieldCount; ++i)
        {
            string sFieldName = rec.fieldName(i).toStdString();
            // 按配置区分大小写
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
// 添加值
void TRecordSetODBCmssqlBase::AddVal(IKCJson& jsonRow, CSqlFunc::TField& fld)
{
    if (!qry().isNull(fld.m_name.c_str()))
        switch (fld.m_type)
        {
        case QMetaType::Double:
            jsonRow.AddVal(fld.m_name.c_str(), qry().value(fld.m_name.c_str()).toDouble());
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            jsonRow.AddVal(fld.m_name.c_str(), qry().value(fld.m_name.c_str()).toInt());
            break;
        case QMetaType::QDate:
            jsonRow.AddStr(fld.m_name.c_str(), qry().value(fld.m_name.c_str()).toDate().toString("yyyy-MM-dd").toStdString().c_str());
            break;
        case QMetaType::QDateTime:
            jsonRow.AddStr(fld.m_name.c_str(), qry().value(fld.m_name.c_str()).toDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str());
            break;
        default:
            jsonRow.AddStr(fld.m_name.c_str(), qry().value(fld.m_name.c_str()).toString().toStdString().c_str());
            break;
        }
    else jsonRow.AddNull(fld.m_name.c_str());
}
// 得到值
CSqlFunc::TKcValVariant TRecordSetODBCmssqlBase::GetVal(CSqlFunc::TField& fld) const
{
    CSqlFunc::TKcValVariant result;
    if (!qry().isNull(fld.m_name.c_str()))
        switch (fld.m_type)
        {
        case QMetaType::Double:
            result = qry().value(fld.m_name.c_str()).toDouble();
            break;
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::LongLong:
        case QMetaType::ULongLong:
            result = qry().value(fld.m_name.c_str()).toInt();
            break;
        case QMetaType::QDate:
            result = qry().value(fld.m_name.c_str()).toDate().toString("yyyy-MM-dd").toStdString();
            break;
        case QMetaType::QDateTime:
            result = qry().value(fld.m_name.c_str()).toDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString();
            break;
        default:
            result = qry().value(fld.m_name.c_str()).toString().toStdString();
            break;
        }
    return result;
}

TRecordSetODBCmssql::TRecordSetODBCmssql(CCtrlODBC& own, QSqlQuery& q, ICtrlApiData& ctrlD)
    : TRecordSetODBCmssqlBase(own, ctrlD), m_qry(q)
{
}
TRecordSetODBCmssqlAlone::TRecordSetODBCmssqlAlone(CCtrlODBC& own, QSqlQuery&& q, ICtrlApiData& ctrlD)
    : TRecordSetODBCmssqlBase(own, ctrlD), m_qry(std::move(q))
{
}

// TDBCommandODBCmssql类
TDBCommandODBCmssql::TDBCommandODBCmssql(CCtrlODBC& own, TOdbcConnObj& db, QSqlQuery& q, ICtrlApiData& ctrlD, string sAct, string sSQL, string sMethod, string &sBack)
    : m_own(own), m_connObj(db), m_qry(q), m_objCtrlD(ctrlD), m_parms(m_own, db, q, m_objCtrlD, sAct), m_act(sAct), m_sql(sSQL), m_method(sMethod), m_back(sBack)
    , m_nowTime(posix_time::to_iso_string(posix_time::second_clock::local_time()))
    , m_parmTmpTab((format("##__%d_%d_%d_%s_%d__") % boost::this_process::get_id() % boost::this_thread::get_id() % db.m_connid % m_nowTime % (rand() % 1000)).str())
{
}

// 所属的服务
IKCSql& TDBCommandODBCmssql::Srv(void)
{
    return m_own.Srv();
}

//  控制器 信息
string TDBCommandODBCmssql::ActInfo(void)
{
    return m_act;
}

// 事务
void TDBCommandODBCmssql::TranBegin(void)
{
    QSqlQuery qry(m_connObj.m_odbc);
    if (!m_connObj.m_odbc.transaction())
        qry.exec("begin tran");
}
void TDBCommandODBCmssql::TranCommit(void)
{
    QSqlQuery qry(m_connObj.m_odbc);
    if (!m_connObj.m_odbc.commit())
        qry.exec("commit tran");
}
void TDBCommandODBCmssql::TranRollback(void)
{
    QSqlQuery qry(m_connObj.m_odbc);
    if (!m_connObj.m_odbc.rollback())
        qry.exec("rollback tran");
}

// 预执行
void TDBCommandODBCmssql::PrepareSQL(void)
{
    // SQL语句不能为空
    if (m_sql.empty())
        throw TKCSqlODBCException(ecd_ErrCode_KCSqlODBC + 5, __CURR_CODE_PLACE_C__, string(m_own.Srv().getHint("SQL_Empty_")) + m_objCtrlD.ActionData().GetSingleInfo("the_request") + " - " + m_act, m_own.own());
    // 解析临时表
    m_parms.ParseSqlTmpTab(m_sql, std::string("##") + c_RESTful_KCTmpTab + "\\w+");
    // 循环每个临时表
    for (auto &tmpTab : m_parms.mapTmpTabs)
    {
        // 原临时表名
        string sName = tmpTab.first.substr(2, tmpTab.first.size() - 2);
        // 替换后临时表名
        tmpTab.second = (format("##_%d_%d______%s") % boost::this_process::get_id() % m_connObj.m_connid % sName).str();
        // 替换临时表
        algorithm::replace_all(m_sql, tmpTab.first, tmpTab.second);
    }
    // 本次请求的活动对象
    IActionData& act = CtrlD().ActionData();
    // 拼接SQL语句，用临时表返回输出参数
    if (c_RESTful_batch != m_method && c_RESTful_create != m_method)
    {
        auto fGetParmType = [&](CSqlFunc::TParmPtr p)
        {
            string sType = "varchar(max)";
            if (CSqlFunc::EDataType::edtInt == p->eDTp) sType = "int";
            else if (CSqlFunc::EDataType::edtNumber == p->eDTp) sType = "numeric(18, 4)";
            else if (CSqlFunc::EDataType::edtDatetime == p->eDTp) sType = "datetime";
            return sType;
        };
        string sParmStr, sParmField;
        // 循环所有参数
        for (auto &parm : m_parms.mapParms)
        {
            auto itOutParm = m_parms.mapOutParms.find(parm.first);
            auto tpParm = act.GetGlobalValType(parm.first.c_str());
            // 输出参数或全局参数
            if (m_parms.mapOutParms.end() != itOutParm || IActionData::egtSession == tpParm || IActionData::egtAction == tpParm)
            {
                // 参数名
                string sParmName = "@__" + m_nowTime + "__" + parm.first + "__";
                sParmStr += "declare " + sParmName + " " + (m_parms.mapOutParms.end() != itOutParm ? fGetParmType(itOutParm->second) : "varchar(max)") + ";";
                sParmStr += "set " + sParmName + " = :" + parm.first + ";";
                sParmField += sParmName + " " + parm.first + ",";
                algorithm::replace_all(m_sql, ":" + parm.first, sParmName);
            }
        }
        algorithm::trim_right_if(sParmField, algorithm::is_any_of(","));
        // 标号
        string sLabel = "__L" + m_nowTime + "__";
        // 替换sql语句中的return语句
        algorithm::ireplace_all(m_sql, "return;", "goto " + sLabel + ";");
        // 在sql语句结尾处添加标号
        algorithm::trim_right(m_sql);
        if (m_sql.back() != ';') m_sql += ";";
        m_sql += "\n" + sLabel + ":";
        // 拼接创建临时表语句
        if (sParmField.empty()) sParmField = "1 a";
        m_sql = sParmStr + "\n" + m_sql + "\nSELECT " + sParmField + " into " +  m_parmTmpTab +  + ";";
        // 删除返回参数的临时表
        QSqlQuery qry(m_connObj.m_odbc);
        qry.exec(("if not object_id('" + m_parmTmpTab + "') is null drop table " + m_parmTmpTab).c_str());
    }
    // 重新预执行sql语句
    if (!m_qry.prepare(m_sql.c_str()))
    {
        m_back = m_qry.lastQuery().toStdString();
        throw m_qry.lastError();
    }
}

// 统一获取参数值
void TDBCommandODBCmssql::MakeParmVals(std::function<void(string, QVariant)> fSetOutParm)
{
    m_qry.finish();
    CAutoRelease _auto([&](){
        QSqlQuery qry(m_connObj.m_odbc);
        qry.exec(("if not object_id('" + m_parmTmpTab + "') is null drop table " + m_parmTmpTab).c_str());
    });
    // 查询临时表，获取输出参数值，循环更新到参数映射里
    QSqlQuery qry(m_connObj.m_odbc);
    if (!qry.exec(("select * from " + m_parmTmpTab).c_str()))
    {
        m_back = m_qry.lastQuery().toStdString();
        throw m_qry.lastError();
    }
    // 获取首条数据
    if (qry.next())
    {
        auto rd = qry.record();
        // 每个字段的值（输出参数）
        for (int i = 0, c = rd.count(); i < c; ++i)
            // 参数名和值
            fSetOutParm(rd.fieldName(i).toStdString(), rd.value(i));
    }
    else if (c_RESTful_batch != m_method)
    {
        m_back = m_qry.lastQuery().toStdString();
        throw std::runtime_error("Parameter not returned. \n" + m_qry.lastError().text().toStdString());
    }
}
void TDBCommandODBCmssql::MakeParmVals(void)
{
    MakeParmVals([&](string sParmName, QVariant val){
        auto itParm = m_parms.mapParms.find(sParmName);
        if (m_parms.mapParms.end() != itParm)
        {
            itParm->second->bIsNull = !val.isValid() || val.isNull();
            if (itParm->second->bIsNull) itParm->second->anyVal.clear();
            else
            {
                if (CSqlFunc::EDataType::edtInt == itParm->second->eDTp) itParm->second->anyVal = val.toInt();
                else if (CSqlFunc::EDataType::edtNumber == itParm->second->eDTp) itParm->second->anyVal = val.toDouble();
                else if (CSqlFunc::EDataType::edtUnknown == itParm->second->eDTp)
                {
                    if (val.type() == QVariant::Int) itParm->second->anyVal = val.toInt();
                    else if (val.type() == QVariant::LongLong) itParm->second->anyVal = val.toLongLong();
                    else if (val.type() == QVariant::Double) itParm->second->anyVal = val.toDouble();
                    else itParm->second->anyVal = val.toString().toStdString();
                }
                else itParm->second->anyVal = val.toString().toStdString();
            }
            // 更新输出参数值
            auto itOutParm = m_parms.mapOutParms.find(sParmName);
            if (m_parms.mapOutParms.end() != itOutParm)
            {
                itOutParm->second->bIsNull = itParm->second->bIsNull;
                itOutParm->second->anyVal = itParm->second->anyVal;
            }
        }
    });
}

// 执行
unsigned int TDBCommandODBCmssql::ExecuteUpdate(void)
{
    // 执行
    if (!m_qry.exec())
    {
        m_back = m_qry.lastQuery().toStdString();
        throw m_qry.lastError();
    }
    return static_cast<unsigned>(m_qry.numRowsAffected());
}
// 批量操作
pair<int, string> TDBCommandODBCmssql::ExecuteBatch(void)
{
    try
    {
        unsigned iAffect = ExecuteUpdate();
        // 获取返回参数
        int iErrCode = 0;
        string sErrMsg, sLogMsg;
        MakeParmVals([&](string sParmName, QVariant val){
            if (c_RESTful_errCode == sParmName) iErrCode = val.toInt();
            else if (c_RESTful_errMsg == sParmName) sErrMsg = val.toString().toStdString();
            else if (c_RESTful_logMsg == sParmName) sLogMsg = val.toString().toStdString();
        });
        if (!sLogMsg.empty())
            m_own.Srv().WriteLogInfo((boost::format("ExecuteBatch: %s - %s\n%s") % CtrlD().LocalFile() % m_act % sLogMsg).str().c_str(), __CURR_CODE_PLACE_C__);
        if (0 != iErrCode) return make_pair(iErrCode, sErrMsg.empty() ? "批量插入错误" : sErrMsg);
        else return make_pair(iAffect, "");
    }
    catch (QSqlError& ex)
    {
        return make_pair(max(static_cast<int>(ex.type()), 1), ex.text().toStdString());
    }
}

// 查询
CSqlFunc::TRecordSetPtr TDBCommandODBCmssql::ExecuteQuery(int &rows_affected)
{
    rows_affected = ExecuteUpdate();
    return CSqlFunc::TRecordSetPtr(new TRecordSetODBCmssql(m_own, m_qry, m_objCtrlD));
}
CSqlFunc::TRecordSetPtr TDBCommandODBCmssql::NextResult(void)
{
    if (m_qry.nextResult())
        return CSqlFunc::TRecordSetPtr(new TRecordSetODBCmssql(m_own, m_qry, m_objCtrlD));
    m_qry.finish();
    return CSqlFunc::TRecordSetPtr();
}

// 执行过程
int TDBCommandODBCmssql::ExecuteProcedures(void)
{
    return static_cast<int>(ExecuteUpdate());
}
