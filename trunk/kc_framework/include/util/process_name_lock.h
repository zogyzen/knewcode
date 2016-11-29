#pragma once

#include <string>

#include <boost/interprocess/sync/named_upgradable_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/sharable_lock.hpp>
#include <boost/interprocess/sync/upgradable_lock.hpp>
#include <boost/date_time.hpp>

namespace KC
{
	// 进程间名称锁
	template<const char* NAME, template<typename> class TLck = boost::interprocess::scoped_lock>
	class CProcessNameLock
	{
	public:
	    CProcessNameLock(void) = delete;
		CProcessNameLock(const char* sName) : mutex(boost::interprocess::open_or_create, sName), lock(mutex) {}
		CProcessNameLock(const char* sName, const boost::posix_time::ptime& abs_time) : mutex(boost::interprocess::open_or_create, sName), lock(mutex, abs_time)
		{
		    if (!lock.owns()) boost::interprocess::named_upgradable_mutex::remove(sName);
		}
		bool IsLock(void) { return lock.owns(); }

	private:
		boost::interprocess::named_upgradable_mutex mutex;
		TLck<boost::interprocess::named_upgradable_mutex> lock;
	};
    // 共享锁
    template<const char* NAME>
    using TProcessNameSharedLock = CProcessNameLock<NAME, boost::interprocess::sharable_lock>;
    // 升级锁
    template<const char* NAME>
    using TProcessNameUpgradLock = CProcessNameLock<NAME, boost::interprocess::upgradable_lock>;
    // 独占锁
    template<const char* NAME>
    using TProcessNameScopedLock = CProcessNameLock<NAME>;
}
