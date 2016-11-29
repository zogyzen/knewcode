#pragma once

#include <string>

#include "framework_ex/framework_exception.h"

namespace KC
{
    // KC异常
    class CKCException : public TFWSrvException
    {
		public:
			CKCException(int id, string place, string msg, string name, string guid, string other = "")
					: TFWSrvException(id, place, msg, name, guid), m_other(other) {}
			virtual string error_info(void) const { return TFWSrvException::error_info() + "[" + m_other + "]"; }

		protected:
			string m_other;
    };

    // 页面异常
    class CKCPageException : public CKCException
    {
		public:
			CKCPageException(int id, string place, string msg, string name, string guid, string other = "")
					: CKCException(id, place, msg, name, guid, other) {}
    };

    // 活动异常
    class CKCActionException : public CKCException
    {
		public:
			CKCActionException(int id, string place, string msg, string name, string guid, string other = "")
					: CKCException(id, place, msg, name, guid, other) {}
    };
}
