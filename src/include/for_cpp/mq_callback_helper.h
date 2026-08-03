#pragma once

#include <string>
#include <atomic>

#include <boost/format.hpp>
#include <boost/shared_array.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include "util/util_funcs.h"

namespace KC
{
    // 消息队列帮助类
    template<typename TOwn>
    class MQCallbackHelper
    {
    public:
        typedef std::function<void(std::string)> TFRecvCB;

        // 构造函数
        MQCallbackHelper(TOwn &own, TFRecvCB fRecv) : m_own(own), m_fRecv(fRecv) {}

        // 初始化
        std::string init(std::string sMQName, const unsigned MQSize = 256, const unsigned MsgSize = 2048)
        {
            std::string sReuslt;
            try
            {
                m_MQName = sMQName;
                m_running = true;
                // 开始进程间消息队列
                std::cout << "Begin: " << sMQName << std::endl;
                m_mqThrd.reset(new boost::thread(&MQCallbackHelper<TOwn>::MQThrd, this, sMQName, MQSize, MsgSize));
            }
            catch (std::exception& ex)
            {
                sReuslt = "初始化消息队列错误：" + sMQName + ". " + ex.what();
            }
            catch (...)
            {
                sReuslt = "初始化消息队列错误：" + sMQName;
            }
            return sReuslt;
        }
        void free(void)
        {
            try
            {
                // 终止进程间消息队列
                m_running = false;
                try
                {
                    boost::interprocess::message_queue::remove(m_MQName.c_str());
                }
                catch (...) {}
                if (m_mqThrd.get() != nullptr && m_mqThrd->joinable()) m_mqThrd->interrupt();
                m_mqThrd.reset();
                std::cout << "End: " << m_MQName << std::endl;
            }
            catch (...) {}
        }

    protected:
        // 消息线程
        void MQThrd(std::string sMQName, const unsigned MQSize = 256, const unsigned MsgSize = 2048)
        {
            try
            {
                const unsigned iMaxMQSize = std::max(MQSize, 128u);
                const unsigned iMaxMsgSize = std::max(MsgSize, 300u);
                try
                {
                    boost::interprocess::message_queue::remove(sMQName.c_str());
                }
                catch (...) {}
                boost::interprocess::permissions permissions;
                permissions.set_unrestricted();
                boost::interprocess::message_queue mq(boost::interprocess::create_only, sMQName.c_str(), iMaxMQSize, iMaxMsgSize, permissions);
                unsigned int priority = 0;
                boost::interprocess::message_queue::size_type recvd_size = 0;
                while (m_running)
                {
                    boost::this_thread::interruption_point();
                    try
                    {
                        std::string sMsg;
                        // 消息缓冲区
                        char *pBuf = new char[iMaxMsgSize + 1] { 0 };
                        boost::shared_array<char> autoDel(pBuf);
                        // 读取消息
                        mq.receive(pBuf, iMaxMsgSize, recvd_size, priority);
                        // 首字母是消息类型：T文本；F文件
                        const char msgType = pBuf[0];
                        // 文本
                        if ('T' == msgType)
                            sMsg.append(pBuf + 1, recvd_size - 1);
                        // 文件
                        else if ('F' == msgType)
                        {
                            // 文件名
                            std::string sFilename(pBuf + 1, recvd_size - 1);
                            // 读文件
                            sMsg = CUtilFunc::ReadFile(sFilename);
                            // 删除文件
                            boost::filesystem::remove(sFilename);
                        }
                        // 处理消息
                        m_fRecv(sMsg);
                    }
                    catch (boost::thread_interrupted&) { throw; }
                    catch (std::exception& ex)
                    {
                        m_own.WriteErr((string("☹️ 主业务模块消息队列失败！\t") + ex.what()).c_str(), __FUNCTION__, typeid(ex).name());
                    }
                    catch (...)
                    {
                        m_own.WriteErr("☹️ 主业务模块消息队列失败！", __FUNCTION__);
                    }
                }
            }
            catch (boost::thread_interrupted&) {}
            catch (...) {}
        }

    private:
        TOwn &m_own;
        // 接收回调函数
        TFRecvCB m_fRecv;
        // 是否允许
        std::atomic_bool m_running = false;
        // 线程
        std::shared_ptr<boost::thread> m_mqThrd;
        // 消息队列名称
        std::string m_MQName;

    public:
        // 发送消息
        static bool SendData(std::string sMQName, std::string data, const unsigned MsgSize = 2048, std::string sPath = "./temp/mq")
        {
            bool bResult = false;
            try
            {
                // 数据大小
                unsigned iMsgSize = std::max(MsgSize, 300u);
                bool bSendTxt = data.size() < iMsgSize;
                // 消息缓冲区
                char *pBuf = nullptr;
                // 写入消息
                if (bSendTxt)
                {
                    iMsgSize = data.size() + 1;
                    pBuf = new char[iMsgSize + 1] { 0 };
                    pBuf[0] = 'T';
                    memcpy(pBuf + 1, data.data(), iMsgSize - 1);
                }
                // 写入文件
                else
                {
                    // 执行序号
                    static unsigned id = 0;
                    ++id;
                    // 文件名
                    if (!boost::filesystem::exists(sPath)) boost::filesystem::create_directories(sPath);
                    std::string sFilename = (boost::format("%s/mq%d.%06d") % sPath % getpid() % id).str();
                    // 内容写入文件
                    if (boost::filesystem::exists(sFilename)) boost::filesystem::remove(sFilename);
                    CUtilFunc::SaveFile(sFilename, data);
                    // 写入消息
                    iMsgSize = sFilename.size() + 1;
                    pBuf = new char[iMsgSize + 1] { 0 };
                    pBuf[0] = 'F';
                    memcpy(pBuf + 1, sFilename.data(), iMsgSize - 1);
                }
                boost::shared_array<char> autoDel(pBuf);
                // 发送消息
                boost::interprocess::message_queue mq(boost::interprocess::open_only, sMQName.c_str());
                mq.send(pBuf, iMsgSize, 0);
                bResult = true;
            }
            catch (...) {}
            return bResult;
        }
    };
}
