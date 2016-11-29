#pragma once

#include <stdexcept>
#include <string>
using namespace std;

#include <boost/lexical_cast.hpp>

namespace KC
{
	// 基础异常
	class TException : public std::runtime_error
	{
	public:
		TException(int id, string place, string msg) : std::runtime_error(msg), m_id(id), m_place(place) {}
		virtual ~TException() throw () {}

		virtual const char* what() const _GLIBCXX_USE_NOEXCEPT { return std::runtime_error::what(); }

		virtual string error_info(void) const
		{
		    return string() + std::runtime_error::what()
                    + " [" + lexical_cast<string>(getpid()) + ":" + lexical_cast<string>(pthread_self())
                    + "][" + m_place + "][" + boost::lexical_cast<string>(m_id) + "][" + m_ExceptType + "]"
                    + m_backtrace;
        }
		virtual int error_id(void) const { return m_id; }
		virtual string error_place(void) const { return m_place; }

    public:
        string& CurrPosInfo(void) { return m_CurrPosInfo; }
        string& ExceptType(void) { return m_ExceptType; }
        string& OtherInfo(void) { return m_OtherInfo; }
        string& BackTrace(void) { return m_backtrace; }

	protected:
		const int m_id = 0;
		const string m_place = "";
		string m_CurrPosInfo = "", m_ExceptType = typeid(*this).name(), m_OtherInfo = "", m_backtrace = "";
	};
}
