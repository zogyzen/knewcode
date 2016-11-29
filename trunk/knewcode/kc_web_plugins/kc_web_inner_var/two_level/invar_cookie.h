#pragma once

#include "../inner_var.h"
#include "../one_level/invar_get.h"

namespace KC { namespace two_level
{
    // get����
    class TInvarCookie : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarCookie(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarCookie();

        // �õ�����
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // �õ��ڲ�����ֵ
        bool Exists(TAnyTypeValList&);
        // �õ��ڲ�����ֵ
        boost::any GetValue(TAnyTypeValList&);
        // �����ڲ�����ֵ
        void SetValue(TAnyTypeValList&, boost::any);

    private:
        // ת��ʱ��
        string TimeToStr(posix_time::ptime) const;
        posix_time::ptime StrToTime(string) const;

    private:
        // Cookie�ṹ
        struct CookieValueT
        {
            // ���ơ�ֵ��·��������
            string name, val, path = "", domain = "";
            // ��Ч��
            posix_time::ptime expires;
            string Expires(void) const { return expires.is_not_a_date_time() ? "" : posix_time::to_simple_string(expires); }
            // �Ƿ�����
            bool new_send = false;
            // ���졢����
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
        // ������
        one_level::TInvarGet::ArgValListT<CookieValueT> m_CookieList;
    };
}}
