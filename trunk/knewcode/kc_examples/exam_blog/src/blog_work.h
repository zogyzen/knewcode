#pragma once

#include <string>
#include <stdexcept>
using namespace std;

#include "for_user/page_interface.h"
#include "for_user/lib/session/kc_session_i.h"
#include "for_user/lib/util/util_algorithm_i.h"
using namespace KC;

#include "blog_work_i.h"

// ���ӽӿ�
class CExamBlog : public IExamBlog
{
public:
    CExamBlog(IActionData&, IDBAction&, IDBParms&, IKCSession&, IKCUtilWork&);
    virtual CALL_TYPE ~CExamBlog() = default;

    // ע���û�
    virtual bool CALL_TYPE EnrollUser(const char*, const char*, const char*, const char*);
    // �û���½
    virtual bool CALL_TYPE UserLogin(const char*, const char*);
    // ˵��ʲô
    virtual bool CALL_TYPE SaySomething(const char*, const char*, const char*, const char*);
    // ��ѯ
    virtual IDBTable& CALL_TYPE SelectBlog(int, int);
    virtual int CALL_TYPE SelectBlogCount(int);

private:
    IActionData& m_act;
    IDBAction& m_db;
    IDBParms& m_parm;
    IKCSession& m_session;
    IKCUtilWork& m_util;
};
