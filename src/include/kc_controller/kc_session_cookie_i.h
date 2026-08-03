#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"
#include "for_user/page_interface.h"

namespace KC
{
    constexpr const char c_KCSessionCookieSrvGUID[] = "IKCSessionCookie_B6E289BB-ADB8-55F8-CF31-BD411D2F52BF";
    class TKCSessionCookieException;

    // Session控制器接口
    class IKCSessionCookie : public IKCCtrlWork
    {
	public:
		// session值
		virtual bool CALL_TYPE IsSession(const char*) const = 0;
        virtual const char* CALL_TYPE GetSessionVal(const char*, IActionData&) = 0;
        virtual const char* CALL_TYPE GetSessionVal(const char*, unsigned&, IActionData&) = 0;
        virtual const char* CALL_TYPE GetSessionVal(const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") = 0;
        virtual const char* CALL_TYPE GetSessionVal(const char*, unsigned&, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") = 0;
        virtual void CALL_TYPE SetSessionVal(const char*, const char*, IActionData&) = 0;
        virtual void CALL_TYPE SetSessionVal(const char*, const char*, unsigned, IActionData&) = 0;
        virtual void CALL_TYPE SetSessionVal(const char*, const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") = 0;
        virtual void CALL_TYPE SetSessionVal(const char*, const char*, unsigned, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") = 0;
        virtual void CALL_TYPE SetSessionExpire(const char*, const char*, IActionData&) = 0;
        virtual void CALL_TYPE SetSessionExpire(const char*, const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") = 0;
        virtual const char* CALL_TYPE GetSessionNextVal(const char*, const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") = 0;
        virtual const char* CALL_TYPE GetSessionNextVal(const char*, const char*, IActionData&) = 0;

        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCSessionCookieSrvGUID;
        }

        typedef TKCSessionCookieException TCtrlException;

    protected:
        ~IKCSessionCookie() override = default;
    };

    class TKCSessionCookieException : public TFWSrvException
	{
	public:
        TKCSessionCookieException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
            : TFWSrvException(id, place, msg, name, c_KCSessionCookieSrvGUID, oth) {}
        TKCSessionCookieException(int id, std::string place, std::string msg, IKCSessionCookie& srv, std::string oth = "")
            : TFWSrvException(id, place, msg, srv, oth) {}
	};
}

