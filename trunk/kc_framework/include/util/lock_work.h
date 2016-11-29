#pragma once

#include <string>
using namespace std;

#include "for_user/common_define.h"

namespace KC
{
	// 锁
	class CKcMutex
	{
	public:
	    CKcMutex(IKcLockWork &lw) : m_lw(lw), m_mtx(lw.NewMutex()) {}
	    ~CKcMutex(void) { m_lw.DelMutex(m_mtx); }

    private:
        IKcLockWork &m_lw;
        IKcLockWork::IMutex *m_mtx = nullptr;

        friend class CKcLock;
	};
	class CKcLock
	{
	public:
	    CKcLock(CKcMutex &mtx, string info = string(), string pos = string()) : m_mtx(mtx), m_info(info), m_pos(pos)
	    {
	        if (nullptr != m_mtx.m_mtx)
                m_mtx.m_mtx->Lock(m_info.empty() ? nullptr : m_info.c_str(), m_pos.empty() ? nullptr : m_pos.c_str());
	    }
	    ~CKcLock(void)
	    {
	        if (nullptr != m_mtx.m_mtx)
                m_mtx.m_mtx->Unlock(m_info.empty() ? nullptr : m_info.c_str(), m_pos.empty() ? nullptr : m_pos.c_str());
	    }

    private:
        CKcMutex &m_mtx;
        string m_info, m_pos;
	};
}
