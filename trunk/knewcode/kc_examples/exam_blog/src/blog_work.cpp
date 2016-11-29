#include "blog_work.h"

////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	IExamBlog& CreateInf(IActionData& act, IDBAction& db, IDBParms& pr, IKCSession& ss, IKCUtilWork& ut)
	{
	    return *new CExamBlog(act, db, pr, ss, ut);
	}

	void FreeInf(IExamBlog& eb)
	{
	    delete &eb;
	}
}

////////////////////////////////////////////////////////////////////////////////
// CExamBlog类
CExamBlog::CExamBlog(IActionData& act, IDBAction& db, IDBParms& pr, IKCSession& ss, IKCUtilWork& ut)
    : m_act(act.GetActRoot()), m_db(db), m_parm(pr), m_session(ss), m_util(ut)
{
}

// 注册用户
bool CExamBlog::EnrollUser(const char* n, const char* a, const char* p, const char* p2)
{
    if (string(p) != p2)
    {
        m_act.GetRequestRespond().OutputHTML("The two passwords are not identical");
        return false;
    }
    if (string(n).empty())
    {
        m_act.GetRequestRespond().OutputHTML("Please input name");
        return false;
    }
    if (string(a).empty())
    {
        m_act.GetRequestRespond().OutputHTML("Please input account");
        return false;
    }
    if (string(p).empty())
    {
        m_act.GetRequestRespond().OutputHTML("Please input password");
        return false;
    }
    m_parm.Clear();
    m_parm.Add("s_name", n);
    m_parm.Add("s_account", a);
    m_parm.Add("s_Pass", p);
    IDBTable& tab = m_db.Select("select s_name from BAS_User where s_account = :s_account", m_parm);
    if (!tab.Empty())
    {
        m_act.GetRequestRespond().OutputHTML((string("Already existing \"") + a + "\"").c_str());
        return false;
    }
    m_db.ExecSQL("insert into BAS_User(s_name, s_account, s_Pass) values(:s_name, :s_account, :s_Pass)", m_parm);
    m_act.GetRequestRespond().OutputHTML("Successful registration");
    return true;
}

// 用户登陆
bool CExamBlog::UserLogin(const char* a, const char* p)
{
    m_parm.Clear();
    m_parm.Add("s_account", a);
    m_parm.Add("s_Pass", p);
    IDBTable& tab = m_db.Select("select s_name from BAS_User where s_account = :s_account and s_Pass = :s_Pass", m_parm);
    if (tab.Empty())
    {
        m_act.GetRequestRespond().OutputHTML("Login failure");
        return false;
    }
    m_session.Set("Account", a);
    m_session.Set("Name", tab.Row().Value(0).AsString());
    return true;
}

// 说点什么
bool CExamBlog::SaySomething(const char* t, const char* t2, const char* f, const char* c)
{
    if (string(t).empty())
    {
        m_act.GetRequestRespond().OutputHTML("Please input title");
        return false;
    }
    m_parm.Clear();
    m_parm.Add("s_account", m_session.Get("Account"));
    m_parm.Add("s_ip", m_act.GetRequestRespond().GetClientIP());
    m_parm.Add("s_title", t);
    m_parm.Add("s_titleSub", t2);
    m_parm.Add("s_image", f);
    m_parm.Add("s_content", c);
    m_db.ExecSQL("insert into DTL_Content(s_account, s_ip, s_title, s_titleSub, s_image, s_content) \
                 values(:s_account, :s_ip, :s_title, :s_titleSub, :s_image, :s_content)", m_parm);
    m_act.GetRequestRespond().OutputHTML("Successful");
    return true;
}

// 查询
IDBTable& CExamBlog::SelectBlog(int iPage, int iMy)
{
    m_parm.Clear();
    m_parm.Add("iPage", iPage);
    m_parm.Add("s_account", m_session.Get("Account"));
    if (1 == iMy)
        return m_db.Select("select * from DTL_Content where s_account = :s_account limit 10 * :iPage, 10", m_parm);
    else
        return m_db.Select("select * from DTL_Content limit 10 * :iPage, 10", m_parm);
}

int CExamBlog::SelectBlogCount(int iMy)
{
    m_parm.Clear();
    m_parm.Add("s_account", m_session.Get("Account"));
    IDBTable* pTab = nullptr;
    if (1 == iMy)
        pTab = &m_db.Select("select count(1) c from DTL_Content where s_account = :s_account", m_parm);
    else
        pTab = &m_db.Select("select count(1) c from DTL_Content");
    int iRecCount = pTab->Row().Value(0).AsInt();
    return (int)(iRecCount / 10) + (iRecCount % 10 > 0 ? 1 : 0);
}
