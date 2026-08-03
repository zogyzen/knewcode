#pragma once

#include "for_user/kc_object_i.h"

// 单独加载动态库时，使用到的众接口
namespace KC
{
    // 通用回调接口
    class IKCSingleLoadSoCB : public IKCObject
    {
    public:
        virtual bool WriteLogError(const char* info, const char* place = "", const char* other = "") const = 0;
        virtual bool WriteLogDebug(const char* info, const char* place = "", const char* other = "") const = 0;
        virtual bool WriteLogTrace(const char* info, const char* place = "", const char* other = "") const = 0;

    public:
        virtual ~IKCSingleLoadSoCB() = default;
    };

    // Web调用接口（使用kc_request_relay库）
    class ISLWebRequest : public IKCObject
    {
    public:
        // POST请求
        virtual unsigned POST(const char*& respond, const char* srv, const char* pth, const char* prm, const char* body, int len = 0, const char* cookie = nullptr) = 0;
        // GET请求
        virtual unsigned GET(const char*& respond, const char* srv, const char* pth, const char* prm, const char* cookie = nullptr) = 0;
        // 得到cookie
        virtual const char* Cookie(const char* srv, const char* cookie) = 0;
        virtual const char* Cookie(const char* srv) = 0;
        // 得到响应头
        virtual const char* RespondHeader(const char* key, const unsigned pos = 0) = 0;

    public:
        virtual ~ISLWebRequest() = default;
    };

}
