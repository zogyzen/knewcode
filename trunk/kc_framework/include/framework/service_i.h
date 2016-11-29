#pragma once

#include "framework/bundle_i.h"

namespace KC
{
	class IService
	{
	public:
		// 得到服务特征码
		virtual const char* CALL_TYPE getGUID(void) const = 0;
		// 得到服务名（类名：return typeid(*this).name();）
		virtual const char* CALL_TYPE getName(void) const = 0;
		// 对应的模块
		virtual const IBundle& CALL_TYPE getBundle(void) const = 0;

    protected:
        virtual CALL_TYPE ~IService() = default;
	};
}
