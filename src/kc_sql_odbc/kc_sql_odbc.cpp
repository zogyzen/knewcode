#include "kc_sql_odbc.h"

////////////////////////////////////////////////////////////////////////////////
// CKCSqlODBC类
CKCSqlODBC::CKCSqlODBC(const IBundle& bundle) : TKCCtrlWorkCommon<IKCSqlODBC, CCtrlODBC>(bundle)
{
    // 数据库驱动
    QCoreApplication::addLibraryPath(m_context.getPath());
    QStringList drivers = QSqlDatabase::drivers();
    string sDBInfo = "";
    foreach(QString driver, drivers)
        sDBInfo += driver.toStdString() + "\n";
    WriteLogInfo("Available DB Drivers: ", __CURR_CODE_PLACE_C__, sDBInfo.c_str());
    //cout << "Available DB Drivers: " << endl << sDBInfo << endl;
}
