#pragma once

#include <string>
#include <map>

#include "for_user/kc_web_request_i.h"
#include "for_user/page_interface.h"

// 单独加载动态库时，使用到的接口
namespace KC
{
    // Web调用接口
    class ISLWebRequestEx : public ISLWebRequest
    {
    public:
        // 转发请求
        virtual std::string PostGet(ICtrlApiData&, std::map<std::string, std::string>&, bool isPost = false) = 0;
    };
}
