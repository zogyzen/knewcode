#include "chai_sql.h"

CChaiSQL *g_ctrl = nullptr;
extern "C"
{
    IChaiModAddition& CALL_TYPE create(ICtrlChai& own, const char* path)
    {
        if (nullptr == g_ctrl)
        {
            // ExceptBacktrace::SetExceptFilter(own.getPath());
            g_ctrl = new CChaiSQL(own, CUtilFunc::PCharSafeToStr(path));
        }
        return *g_ctrl;
    }
    void CALL_TYPE destroy(IChaiModAddition&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// sqlite 数据库类
class KcSQLite
{
public:
    KcSQLite(IChaiKcActCtrl& objCtrl, string srv, string ctrl) : m_objCtrl(objCtrl), m_srv(srv), m_ctrl(ctrl)
    {
    }

    void Create(string sql)
    {
        m_objCtrl.getCtrlApiData().ExecBundleSub(m_srv.c_str(), m_ctrl.c_str(), c_RESTful_create, sql.c_str());
    }
    void Batch(string sql)
    {
        m_objCtrl.getCtrlApiData().ExecBundleSub(m_srv.c_str(), m_ctrl.c_str(), c_RESTful_batch, sql.c_str());
    }
    void Query(string sql)
    {
        m_objCtrl.getCtrlApiData().ExecBundleSub(m_srv.c_str(), m_ctrl.c_str(), c_RESTful_query, sql.c_str());
    }
    void Exec(string sql)
    {
        m_objCtrl.getCtrlApiData().ExecBundleSub(m_srv.c_str(), m_ctrl.c_str(), c_RESTful_exec, sql.c_str());
    }

private:
    IChaiKcActCtrl& m_objCtrl;
    string m_srv, m_ctrl;
};

////////////////////////////////////////////////////////////////////////////////
// CChaiSQL 类
CChaiSQL::CChaiSQL(ICtrlChai& own, string path)
{
}

// 向模块里添加语法
void CChaiSQL::Add(chaiscript::ModulePtr mod, std::string name, const boost::property_tree::ptree& pt)
{
    // 类名前缀
    string sPrefix;
    std::string sAttrPrefix = string(c_RESTful_xmlattr) + ".classPrefix";
    if (pt.get_child_optional(sAttrPrefix))
        sPrefix = pt.get<string>(sAttrPrefix);
    // 添加SQLite类
    chaiscript::utility::add_class<KcSQLite>(*mod,
        sPrefix + "KcSQLite",
        {},
        {
            {chaiscript::fun(&KcSQLite::Create), "Create"},
            {chaiscript::fun(&KcSQLite::Batch), "Batch"},
            {chaiscript::fun(&KcSQLite::Query), "Query"},
            {chaiscript::fun(&KcSQLite::Exec), "Exec"}
        }
    );
    // 添加PostgreSQL类

    // 循环添加数据库的创建函数
    BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, pt)
    {
        string sName = v.first;
        if (c_RESTful_xmlcomment != sName && c_RESTful_xmlattr != sName)
        {
            string srv, ctrl;
            std::string sAttrSrv = string(c_RESTful_xmlattr) + ".srv";
            if (v.second.get_child_optional(sAttrSrv))
                srv = v.second.get<string>(sAttrSrv);
            std::string sAttrCtrl = string(c_RESTful_xmlattr) + ".ctrl";
            if (v.second.get_child_optional(sAttrCtrl))
                ctrl = v.second.get<string>(sAttrCtrl);
            mod->add(chaiscript::fun([=](IChaiKcActCtrl& objCtrl){ return KcSQLite(objCtrl, srv, ctrl); }), sPrefix + sName);
        }
    }
}
