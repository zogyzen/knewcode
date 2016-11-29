#pragma once

namespace KC
{
    class CKCNetWork : public IKCNetWork
    {
    public:
        CKCNetWork(void);

        // �����������ܻ�ӿ�
		virtual IKCNetAction& CALL_TYPE GetNetAction(IActionData&);

    private:
        // ��ȡ��������ҳ�����ݽӿ�
        IKCRequestRespond::IReStepData& GetKCSessionWorkStepData(IActionData&);

    private:
        asio::io_service m_io_serv;
    };
}
