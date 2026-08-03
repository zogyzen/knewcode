#pragma once

#include "for_user/kc_object_i.h"
#include "framework/service_i.h"

namespace KC
{
    class IServiceRegistration : public IKCObject
	{
	public:
		// 得到服务特征码
		virtual const char* CALL_TYPE getGUID(void) const = 0;
		// 得到服务名（类名）
		virtual const char* CALL_TYPE getName(void) const = 0;
		// 获取服务接口
		virtual IService& CALL_TYPE getService(void) const = 0;
		// 获取模块接口
		virtual const IBundle& CALL_TYPE getBundle(void) const = 0;

  protected:
                virtual ~IServiceRegistration() = default;
	};
}
