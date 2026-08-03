#pragma once

namespace KC
{
    // Web调用消息队列接口
    class IWebCallMQ
    {
    public:
        // 回调接口
        class ICallback
        {
        public:
            // 接收数据
            virtual void RecvData(const char*, unsigned) = 0;
        };

    public:
        // 初始化/释放
        virtual const char* init(const char* sMQName, const unsigned MQSize = 256, const unsigned MsgSize = 2048) = 0;
        virtual void free(void) = 0;
    };
}
