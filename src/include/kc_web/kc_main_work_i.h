#pragma once

#include "for_user/kc_object_i.h"
#include "kc_web/kc_request_respond.h"

namespace KC
{
    class IKCWebLongConn;

    // 启动接口
    class IKCStartWork : public IKCObject
    {
    public:
        // 宿主名称
        virtual const char* CALL_TYPE OwnName(void) const = 0;
        // 宿主版本
        virtual const char* CALL_TYPE OwnVersion(void) const = 0;

        // Web链接类型
        enum EWebConnType { ewctWebsocket, ewctSSE };
        // 打包Web链接
        virtual IKCWebLongConn& CALL_TYPE PackWebConn(IBaseRequestRespond&, EWebConnType) = 0;
        // 释放web连接
        virtual void CALL_TYPE ReleaseWebConn(IKCWebLongConn&) = 0;

        // 虚拟目录
        virtual unsigned CALL_TYPE VirtualPathCount(void) = 0;
        virtual const char* CALL_TYPE GetVirtualPath(unsigned) = 0;
        virtual const char* CALL_TYPE GetVirtualPathUri(unsigned) = 0;
        // 得到url对应的本地完整文件名
        virtual const char* CALL_TYPE GetUrlLocalPath(const char*) = 0;
        // 得到网页根路径
        virtual const char* CALL_TYPE GetUrlPageRootPath(const char*) = 0;
        // 得到网站或应用根路径
        virtual const char* CALL_TYPE GetWebsiteRootPath(void) = 0;
        // 得到主平台根路径
        virtual const char* CALL_TYPE GetPlatformRootPath(void) = 0;
        // 得到web服务程序或应用程序的根目录
        virtual const char* CALL_TYPE GetApPath(void) = 0;

    protected:
        ~IKCStartWork() override = default;
    };
}

