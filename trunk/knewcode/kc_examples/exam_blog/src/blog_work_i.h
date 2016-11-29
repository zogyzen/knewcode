#pragma once

#include "for_user/lib/db/kc_db_i.h"

// ���ӽӿ�
class IExamBlog
{
public:
    // ע���û�
    virtual bool CALL_TYPE EnrollUser(const char*, const char*, const char*, const char*) = 0;
    // �û���½
    virtual bool CALL_TYPE UserLogin(const char*, const char*) = 0;
    // ˵��ʲô
    virtual bool CALL_TYPE SaySomething(const char*, const char*, const char*, const char*) = 0;
    // ��ѯ
    virtual IDBTable& CALL_TYPE SelectBlog(int, int) = 0;
    virtual int CALL_TYPE SelectBlogCount(int) = 0;

public:
    virtual CALL_TYPE ~IExamBlog() = default;
};
