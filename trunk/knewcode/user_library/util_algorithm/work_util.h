#pragma once

namespace KC
{
    class CKCUtilWork : public IKCUtilWork
    {
    public:
        CKCUtilWork(void);

        // 创建公共功能活动接口
		virtual IKCAlgorithmAction& CALL_TYPE GetAlgorithmAction(IActionData&);

    private:
        // 获取公共功能页面数据接口
        IKCRequestRespond::IReStepData& GetKCSessionWorkStepData(IActionData&);
    };
}
