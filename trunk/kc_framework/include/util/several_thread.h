#pragma once

#include <list>
#include <map>
#include <string>
#include <thread>
#include <mutex>
#include <memory>

#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/random.hpp>

#include "util/auto_release.h"

namespace KC
{
	// 任务线程
	template<int ID>
	class CSeveralThread
	{
	public:
		CSeveralThread(void) {};
		~CSeveralThread(void) {};

		// 线程类
		typedef std::shared_ptr<std::thread> CRunThread;

		// 新线程
		template<typename _Functor>
		CRunThread NewThread(string sIndex, _Functor functor)
		{
		    CRunThread thrd;
            void (CSeveralThread::*pFunc)(string, _Functor) = &CSeveralThread::RunThread;
            auto Ftor = boost::lambda::bind(pFunc, this, sIndex, boost::lambda::protect(functor));
		    {
                m_Mutex.lock();
                CAutoRelease _autoB(boost::bind(&std::mutex::unlock, &this->m_Mutex));
                thrd = CRunThread(new std::thread(Ftor));
                m_thrdList.insert(make_pair(sIndex, thrd));
		    }
			//Ftor();
            return thrd;
		}
		template<typename _Functor>
		CRunThread NewThread(_Functor functor)
		{
			string sIndex = MakeUniqueStr("T");
			return NewThread(sIndex, functor);
		}
		template<typename _Functor>
		string NewThreadGetIndex(_Functor functor)
		{
			string sIndex = MakeUniqueStr("T");
			NewThread(sIndex, functor);
			return sIndex;
		}

		// 通过编号，得到线程
		CRunThread GetThreadByIndex(string sIndex)
		{
            this->m_Mutex.lock();
            CAutoRelease _autoB(boost::bind(&std::mutex::unlock, &this->m_Mutex));
			CThreadList::iterator iter = this->m_thrdList.find(sIndex);
			return iter->second;
		}

		// 结束线程
		void StopThread(string sIndex)
		{
            this->m_Mutex.lock();
            CAutoRelease _autoB(boost::bind(&std::mutex::unlock, &this->m_Mutex));
			CThreadList::iterator iter = this->m_thrdList.find(sIndex);
			if (this->m_thrdList.end() != iter)
			{
				CRunThread thrd = iter->second;
				//thrd->interrupt();
				pthread_cancel(thrd->native_handle());
				this->m_thrdList.erase(sIndex);
			}
		}

		// 结束线程
		void StopAllThread()
		{
			this->m_Mutex.lock();
            CAutoRelease _autoB(boost::bind(&std::mutex::unlock, &this->m_Mutex));
			for (CThreadList::iterator iter = this->m_thrdList.begin(); iter != this->m_thrdList.end(); ++iter)
			{
				CRunThread thrd = iter->second;
				//thrd->interrupt();
				pthread_cancel(thrd->native_handle());
			}
			this->m_thrdList.clear();
		}

		// 等待所有线程结束
		void JoinAllThread()
		{
			for (CThreadList::iterator iter = this->m_thrdList.begin(); iter != this->m_thrdList.end(); ++iter)
			{
				CRunThread thrd = iter->second;
				thrd->join();
			}
		}

	protected:
		// 线程列表
		typedef std::map<string, CRunThread> CThreadList;
		CThreadList m_thrdList;

		// 锁
		std::mutex m_Mutex;

	protected:
		// 执行线程
		template<typename _Functor>
		void RunThread(string sIndex, _Functor functor)
		{
			functor();
            this->m_Mutex.lock();
            CAutoRelease _autoB(boost::bind(&std::mutex::unlock, &this->m_Mutex));
			CThreadList::iterator iter = this->m_thrdList.find(sIndex);
			if (this->m_thrdList.end() != iter)
			{
				CRunThread thrd = iter->second;
				this->m_thrdList.erase(sIndex);
			}
		}

		// 生成随机且唯一的字符串
		string MakeUniqueStr(string sHeadStr)
		{
			// 随机数
			boost::variate_generator<boost::mt19937, boost::uniform_int<> > _Random(boost::mt19937(), boost::uniform_int<>(1000, 9999));
			// 随机字符串
			string strSrc = sHeadStr + boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time()) + "R"
				+ boost::lexical_cast<string>(_Random());
			return strSrc;
		}
	};
}
