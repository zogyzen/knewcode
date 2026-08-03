#pragma once

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

namespace KC
{
	// 进程间共享内存
    template<typename TObj>
    struct TObjRefType
    {
        typedef TObj& TResult;
    };
    template<>
    struct TObjRefType<string>
    {
        typedef string TResult;
    };

	template<typename TObj>
	class CProcessNameMemory
	{
	public:
	    void Set(string sName, typename TObjRefType<TObj>::TResult sVal)
	    {
            this->Destroy(sName);
            boost::interprocess::managed_shared_memory managed_shm(boost::interprocess::create_only, sName.c_str(), sizeof(TObj) + 32);
	        managed_shm.construct<TObj>(boost::interprocess::unique_instance)(sVal);
	    }

	    typename TObjRefType<TObj>::TResult Get(string sName)
	    {
            boost::interprocess::managed_shared_memory managed_shm(boost::interprocess::open_only, sName.c_str());
	        return *managed_shm.find<TObj>(boost::interprocess::unique_instance).first;
	    }

		void Destroy(string sName)
		{
            try
            {
                boost::interprocess::managed_shared_memory managed_shm(boost::interprocess::open_only, sName.c_str());
                managed_shm.destroy<TObj>(boost::interprocess::unique_instance);
            }
            catch (...) {}
            this->Remove(sName);
		}

		void Remove(string sName)
		{
            boost::interprocess::shared_memory_object::remove(sName.c_str());
		}

		bool Exists(string sName)
		{
            bool bResult = true;
            try
            {
                boost::interprocess::shared_memory_object(boost::interprocess::open_only, sName.c_str(), boost::interprocess::read_only);
            }
            catch (...)
            {
                bResult = false;
            }
            return bResult;
		}
	};

	template<>
    inline void CProcessNameMemory<string>::Set(string sName, string sVal)
    {
        this->Remove(sName);
        boost::interprocess::shared_memory_object shm(boost::interprocess::create_only, sName.c_str(), boost::interprocess::read_write);
        shm.truncate(sVal.size());
        boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
        memcpy(region.get_address(), sVal.c_str(), sVal.size());
    }

	template<>
    inline string CProcessNameMemory<string>::Get(string sName)
    {
        string sResult;
        try
        {
            boost::interprocess::shared_memory_object shm(boost::interprocess::open_only, sName.c_str(), boost::interprocess::read_only);
            boost::interprocess::mapped_region region(shm, boost::interprocess::read_only);
            sResult.append((char*)region.get_address(), region.get_size());
        }
        catch (...) {}
        return sResult;
    }

	template<>
    inline void CProcessNameMemory<string>::Destroy(string sName)
    {
        this->Remove(sName);
    }
}
