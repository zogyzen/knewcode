#pragma once

namespace KC
{
    class CKCSession : public IKCSession
    {
    public:
        CKCSession(IActionData&, IKCSessionWork&);

		// 设置Session值
		virtual bool CALL_TYPE Set(const char*, const char*);
		// 获取Session值
		virtual const char* CALL_TYPE Get(const char*);
		// 移除Session
		virtual bool CALL_TYPE Remove(const char*);
		// 是否存在Session
		virtual bool CALL_TYPE Exists(const char*);

    public:
        // 清除过期Session
        void ClearUp(void);

    private:
        // 得到session编号
        string GetSessionID(void);

    private:
        IActionData& m_act;
        IKCSessionWork& m_work;
        IServiceReferenceEx& m_invar;
        string m_tmpStr;
    };
}
