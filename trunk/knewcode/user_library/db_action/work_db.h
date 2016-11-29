#pragma once

namespace KC
{
    class CKCDBWork : public IKCDBWork
    {
    public:
        CKCDBWork(string);

		// 创建数据库活动（参数依次为链接名称、驱动名、连接串）
		virtual IDBAction& CALL_TYPE CreateDBAction(IActionData&, const char*, const char*, const char*);
		// 创建数据库参数
		virtual IDBParms& CALL_TYPE NewParms(IActionData&);

    private:
        // 获取数据库管理页面数据接口
        IKCRequestRespond::IReStepData& GetKCDBWorkStepData(IActionData&);

    private:
        // 默认目录
        string m_dir;
    };
}
