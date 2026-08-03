#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_controller/kc_ctrl_work_i.h"

namespace KC
{
    constexpr const char c_KCOSFileSrvGUID[] = "IKCOSFile_B6E289BB-ADB8-93F8-CF31-BD411D2F52BF";
    class TKCOSFileException;

    // 系统磁盘文件管理控制器接口
    class IKOSFile : public IKCCtrlWork
    {
    public:
        // 得到服务特征码
        const char* CALL_TYPE getGUID(void) const override
        {
            return c_KCOSFileSrvGUID;
        }

        typedef TKCOSFileException TCtrlException;

    protected:
        ~IKOSFile() override = default;
    };

    class TKCOSFileException : public TFWSrvException
	{
	public:
                TKCOSFileException(int id, std::string place, std::string msg, std::string name, std::string oth = "")
                                : TFWSrvException(id, place, msg, name, c_KCOSFileSrvGUID, oth) {}
                TKCOSFileException(int id, std::string place, std::string msg, IKOSFile& srv, std::string oth = "")
                : TFWSrvException(id, place, msg, srv, oth) {}
	};
}

