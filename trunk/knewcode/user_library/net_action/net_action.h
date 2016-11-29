#pragma once

namespace KC
{
    class CKCNetAction : public IKCNetAction
    {
    public:
        CKCNetAction(IActionData&, asio::io_service&);

        // ªÒ»°Õ¯“≥
		virtual const char* CALL_TYPE SyncHttpGet(const char*, const char*, int = 80);

    private:
        IActionData& m_act;
        asio::io_service& m_io_serv;
        string m_SyncHttpGet;
    };
}
