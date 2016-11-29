#pragma once

namespace KC
{
    class CKCNetWork : public IKCNetWork
    {
    public:
        CKCNetWork(void);

        // 创建公共功能活动接口
		virtual IKCNetAction& CALL_TYPE GetNetAction(IActionData&);

    private:
        // 获取公共功能页面数据接口
        IKCRequestRespond::IReStepData& GetKCSessionWorkStepData(IActionData&);

    private:
        asio::io_service m_io_serv;
    };
}
