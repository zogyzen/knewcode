#pragma once

namespace KC
{
    class CKCUtilWork : public IKCUtilWork
    {
    public:
        CKCUtilWork(void);

        // �����������ܻ�ӿ�
		virtual IKCAlgorithmAction& CALL_TYPE GetAlgorithmAction(IActionData&);

    private:
        // ��ȡ��������ҳ�����ݽӿ�
        IKCRequestRespond::IReStepData& GetKCSessionWorkStepData(IActionData&);
    };
}
