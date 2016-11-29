#pragma once

#include "../inner_var.h"
#include "../one_level/invar_get.h"

namespace KC { namespace two_level
{
    // get变量
    class TInvarCookie : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarCookie(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarCookie();

        // 得到名称
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // 得到内部变量值
        bool Exists(TAnyTypeValList&);
        // 得到内部变量值
        boost::any GetValue(TAnyTypeValList&);
        // 设置内部变量值
        void SetValue(TAnyTypeValList&, boost::any);

    private:
        // 转换时间
        string TimeToStr(posix_time::ptime) const;
        posix_time::ptime StrToTime(string) const;

    private:
        // Cookie结构
        struct CookieValueT
        {
            // 名称、值、路径、域名
            string name, val, path = "", domain = "";
            // 有效期
            posix_time::ptime expires;
            string Expires(void) const { return expires.is_not_a_date_time() ? "" : posix_time::to_simple_string(expires); }
            // 是否新项
            bool new_send = false;
            // 构造、拷贝
            CookieValueT(string n = "", string v = "") : name(n), val(v) {}
            CookieValueT(const CookieValueT& c)
            : name(c.name), val(c.val), path(c.path), domain(c.domain), expires(c.expires), new_send(c.new_send) {}
            CookieValueT& operator=(const CookieValueT& c)
            { name = c.name; val = c.val; expires = c.expires; path = c.path; domain = c.domain; new_send = c.new_send; return *this; }
        };

    private:
        IKCActionData& m_act;
        IKCRequestRespond& m_re;
        CInnerVar& m_InnerVar;
        // 参数组
        one_level::TInvarGet::ArgValListT<CookieValueT> m_CookieList;
    };
}}
