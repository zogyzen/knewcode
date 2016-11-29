#pragma once

#include <ctime>

#include "util/process_name_lock.h"
#include "util/process_name_memory.h"

namespace KC
{
    constexpr int c_ProcMemNameLen = 100;
    constexpr char c_ProcMemNameLock[] = "^@^#lock!";
    constexpr char c_ProcMemNameNode[] = "^@^&node!";

	// 进程间数据列表
	template<const char* NAME, typename TObj>
	class CProcessNameList
	{
    private:
        // 顶点
        struct CProcMemRoot
        {
            int m_count = 0;
            char m_head[c_ProcMemNameLen] = {0};
            char m_tail[c_ProcMemNameLen] = {0};

            CProcMemRoot(void) = default;

            CProcMemRoot(const CProcMemRoot& c) : m_count(c.m_count)
            {
                memcpy(m_head, c.m_head, c_ProcMemNameLen);
                memcpy(m_tail, c.m_tail, c_ProcMemNameLen);
            }
        };
        typedef CProcessNameMemory<CProcMemRoot> TProcMemRoot;

        // 节点
        struct CProcMemNode
        {
            std::time_t m_time = std::time(nullptr);
            char m_name[c_ProcMemNameLen] = {0};
            char m_last[c_ProcMemNameLen] = {0};
            char m_next[c_ProcMemNameLen] = {0};

            void Clone(const CProcMemNode& c)
            {
                m_time = c.m_time;
                memcpy(m_name, c.m_name, c_ProcMemNameLen);
                memcpy(m_last, c.m_last, c_ProcMemNameLen);
                memcpy(m_next, c.m_next, c_ProcMemNameLen);
            }

            CProcMemNode(string sName)
            {
                memcpy(m_name, sName.c_str(), std::min((int)sName.size(), c_ProcMemNameLen));
            }

            CProcMemNode(const CProcMemNode& c)
            {
                Clone(c);
            }

            CProcMemNode& operator=(const CProcMemNode& c)
            {
                Clone(c);
                return *this;
            }
        };
        typedef CProcessNameMemory<CProcMemNode> TProcMemNode;

        // 数据
        typedef CProcessNameMemory<TObj> TProcMemData;

        // 锁
        struct CScopedLock
        {
            TProcessNameScopedLock<NAME> m_lock;
            CScopedLock(string sName, int timeOut) : m_lock((sName + c_ProcMemNameLock).c_str(), posix_time::second_clock::local_time() + posix_time::seconds(timeOut)) {}
            bool IsLock(void) { return m_lock.IsLock(); }
        };

    private:
        int m_timeOut = 3600;

    private:
        CProcMemRoot& GetRoot(void)
        {
            TProcMemRoot rtWk;
            if (!rtWk.Exists(NAME))
            {
                CProcMemRoot pmr;
                rtWk.Set(NAME, pmr);
            }
            return rtWk.Get(NAME);
        }

        void AdjustListAdd(string sName)
        {
            CScopedLock lck(NAME, m_timeOut);
            if (!lck.IsLock()) return;
            CProcMemRoot& rt = GetRoot();
            string sTail = rt.m_tail;
            string sNodeName = sName + c_ProcMemNameNode;
            {
                CScopedLock lck(sName, m_timeOut);
                if (!lck.IsLock()) return;
                TProcMemNode ndWk;
                if (ndWk.Exists(sNodeName))
                {
                    CProcMemNode& pmn = ndWk.Get(sNodeName);
                    memcpy(pmn.m_last, sTail.c_str(), std::min((int)sTail.size(), c_ProcMemNameLen));
                }
            }
            sNodeName = sTail + c_ProcMemNameNode;
            {
                CScopedLock lck(sTail, m_timeOut);
                if (!lck.IsLock()) return;
                TProcMemNode ndWk;
                if (ndWk.Exists(sNodeName))
                {
                    CProcMemNode& pmn = ndWk.Get(sNodeName);
                    memcpy(pmn.m_next, sName.c_str(), std::min((int)sName.size(), c_ProcMemNameLen));
                }
            }
            memcpy(rt.m_tail, sName.c_str(), std::min((int)sName.size(), c_ProcMemNameLen));
            ++rt.m_count;
        }

        string CheckForRemove(string sName, bool bUserDel = false)
        {
            string sLast, sNext;
            string sNodeName = sName + c_ProcMemNameNode;
            {
                CScopedLock lck(sName, m_timeOut);
                if (!lck.IsLock()) return sNext;
                TProcMemNode ndWk;
                if (!ndWk.Exists(sNodeName)) return sNext;
                CProcMemNode& pmn = ndWk.Get(sNodeName);
                sLast = pmn.m_last;
                sNext = pmn.m_next;
                if (bUserDel || difftime(std::time(nullptr), pmn.m_time) < m_timeOut) return sNext;
            }
            if (!sLast.empty())
            {
                sNodeName = sLast + c_ProcMemNameNode;
                CScopedLock lck(sLast, m_timeOut);
                if (lck.IsLock())
                {
                    TProcMemNode ndWk;
                    if (ndWk.Exists(sNodeName))
                    {
                        CProcMemNode& pmn = ndWk.Get(sNodeName);
                        memcpy(pmn.m_next, sNext.c_str(), std::min((int)sNext.size(), c_ProcMemNameLen));
                    }
                }
            }
            if (!sNext.empty())
            {
                sNodeName = sNext + c_ProcMemNameNode;
                CScopedLock lck(sNext, m_timeOut);
                if (lck.IsLock())
                {
                    TProcMemNode ndWk;
                    if (ndWk.Exists(sNodeName))
                    {
                        CProcMemNode& pmn = ndWk.Get(sNodeName);
                        memcpy(pmn.m_last, sLast.c_str(), std::min((int)sLast.size(), c_ProcMemNameLen));
                    }
                }
            }
            CProcMemRoot& rt = GetRoot();
            if (rt.m_head == sName)
                memcpy(rt.m_head, sLast.c_str(), std::min((int)sLast.size(), c_ProcMemNameLen));
            if (rt.m_tail == sName)
                memcpy(rt.m_tail, sNext.c_str(), std::min((int)sNext.size(), c_ProcMemNameLen));
            --rt.m_count;
            {
                CScopedLock lck(sName, m_timeOut);
                if (lck.IsLock())
                {
                    TProcMemData dbWk;
                    dbWk.Destroy(sName);
                    TProcMemNode ndWk;
                    ndWk.Destroy(sNodeName);
                }
            }
            return sNext;
        }

	public:
	    CProcessNameList(int timeOut = 3600) : m_timeOut(timeOut)
	    {
	    }

	    void SetTimeOut(int timeOut)
	    {
	        m_timeOut = timeOut;
	    }

	    int GetTimeOut(void)
	    {
	        return m_timeOut;
	    }

	    void Set(string sName, typename TObjRefType<TObj>::TResult obj)
	    {
	        bool isNew = false;
            string sNodeName = sName + c_ProcMemNameNode;
	        // 添加节点数据
	        {
                CScopedLock lck(sName, m_timeOut);
                if (!lck.IsLock()) return;
                // 添加数据
                TProcMemData dbWk;
                dbWk.Set(sName, obj);
                // 添加节点
                TProcMemNode ndWk;
                if (ndWk.Exists(sNodeName))
                {
                    CProcMemNode& pmn = ndWk.Get(sNodeName);
                    pmn.m_time = std::time(nullptr);
                }
                else
                {
                    CProcMemNode pmn(sName);
                    ndWk.Set(sNodeName, pmn);
                    isNew = true;
                }
	        }
	        // 调整节点链表
	        if (isNew) AdjustListAdd(sNodeName);
	    }

	    typename TObjRefType<TObj>::TResult Get(string sName)
	    {
            string sNodeName = sName + c_ProcMemNameNode;
            CScopedLock lck(sName, m_timeOut);
            // 节点
            TProcMemNode ndWk;
            if (ndWk.Exists(sNodeName))
            {
                CProcMemNode& pmn = ndWk.Get(sNodeName);
                pmn.m_time = std::time(nullptr);
            }
             // 得到数据
            TProcMemData dbWk;
            return dbWk.Get(sName);
	    }

	    void Remove(string sName)
	    {
            CheckForRemove(sName, true);
	    }

	    bool Exists(string sName)
	    {
            string sNodeName = sName + c_ProcMemNameNode;
            CScopedLock lck(sName, m_timeOut);
            // 节点
            TProcMemNode ndWk;
            return ndWk.Exists(sNodeName);
	    }

	    void GC(void)
	    {
            CScopedLock lck(NAME, m_timeOut);
            if (!lck.IsLock()) return;
            CProcMemRoot& rt = GetRoot();
            for (string sNode = rt.m_head; !sNode.empty(); sNode = CheckForRemove(sNode));
	    }
	};
}
