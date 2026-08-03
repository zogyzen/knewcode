#include "mq_webcall.h"

extern "C"
{
    IWebCallMQ& create(IWebCallMQ::ICallback& cb, const char* path)
    {
        return *new CWebCallMQ(cb, CUtilFunc::PCharSafeToStr(path));
    }
    void destroy(IWebCallMQ& mq)
    {
        delete &mq;
    }
}

////////////////////////////////////////////////////////////////////////////////
// CWebCallMQ 类
CWebCallMQ::CWebCallMQ(ICallback& cb, std::string path) : m_cb(cb)
    , m_mqHelper(*this, [&](std::string data){ m_cb.RecvData(data.data(), data.size()); })
{
}

// 错误日志
void CWebCallMQ::WriteErr(const char* msg, const char* place, const char* oth)
{
    std::cout << msg << std::endl << oth << std::endl;
}

// 初始化/释放
const char* CWebCallMQ::init(const char* sMQName, const unsigned MQSize, const unsigned MsgSize)
{
    static thread_local string sErr;
    sErr = m_mqHelper.init(CUtilFunc::PCharSafeToStr(sMQName), MQSize, MsgSize);
    return sErr.c_str();
}
void CWebCallMQ::free(void)
{
    m_mqHelper.free();
}
