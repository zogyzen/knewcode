#pragma once

namespace KC
{
    // 进程间共享内存管理
    typedef CProcessNameList<g_SessionConnName, string> TProcNameList;
    struct IProcNameListWork
    {
        virtual ~IProcNameListWork(void) = default;
        virtual TProcNameList& GetProcMemList(void) = 0;
    };

    class CKCSessionWork : public IKCSessionWork
    {
    public:
        CKCSessionWork(void);
        ~CKCSessionWork(void);

        // 获取Session接口
		virtual IKCSession& CALL_TYPE GetSession(IActionData&);

        // 进程间共享内存
		IProcNameListWork& GetProcMemListWork(void);

    private:
        // 获取Session页面数据接口
        IKCRequestRespond::IReStepData& GetKCSessionWorkStepData(IActionData&);

    private:
        // 进程间共享内存管理
        IProcNameListWork* m_procMemList = nullptr;
    };
}
