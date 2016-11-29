#pragma once

#include "for_user/lib/db/kc_db_i.h"

// 例子接口
class IExamBlog
{
public:
    // 注册用户
    virtual bool CALL_TYPE EnrollUser(const char*, const char*, const char*, const char*) = 0;
    // 用户登陆
    virtual bool CALL_TYPE UserLogin(const char*, const char*) = 0;
    // 说点什么
    virtual bool CALL_TYPE SaySomething(const char*, const char*, const char*, const char*) = 0;
    // 查询
    virtual IDBTable& CALL_TYPE SelectBlog(int, int) = 0;
    virtual int CALL_TYPE SelectBlogCount(int) = 0;

public:
    virtual CALL_TYPE ~IExamBlog() = default;
};
