#pragma once

#include <string>

#include "framework_ex/framework_exception.h"

namespace KC
{
    // KC异常
    class CKCException : public TFWSrvException
    {
    public:
        CKCException(int id, std::string place, std::string msg, std::string name, std::string guid, std::string other = "")
            : TFWSrvException(id, place, msg, name, guid, other) {}
    };

    // 页面异常
    class CKCPageException : public CKCException
    {
    public:
        CKCPageException(int id, std::string place, std::string msg, std::string name, std::string guid, std::string other = "")
            : CKCException(id, place, msg, name, guid, other) {}
    };

    // 活动异常
    class CKCActionException : public CKCException
    {
    public:
        CKCActionException(int id, std::string place, std::string msg, std::string name, std::string guid, std::string other = "")
            : CKCException(id, place, msg, name, guid, other) {}
    };
}
