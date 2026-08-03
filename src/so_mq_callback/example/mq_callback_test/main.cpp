#include <iostream>

#include "util/load_sub_lib.h"
#include "util/auto_release.h"
#include "inf/mq_webcall_i.h"

class TMQCallback : public KC::IWebCallMQ::ICallback
{
public:
    // 接收数据
    void RecvData(const char* data, unsigned len) override
    {
        if (nullptr != data && len > 0)
            std::cout << std::string(data, len) << std::endl;
    }
};

int main(int argc, char *argv[])
{
    KC::TSubModule<KC::IWebCallMQ> mqLib;
    TMQCallback cb;
    string sErr = mqLib.Load(cb, "mq_webcall_lib", argv[0]);
    if (sErr.empty())
    {
        sErr = KC::CUtilFunc::PCharSafeToStr(mqLib.Inf().init("my@work", 512, 8192));
        if (!sErr.empty()) std::cout << sErr << std::endl;
        KC::CAutoRelease autoDel([&](){
            mqLib.Inf().free();
            mqLib.UnLoad();
        });
        // 暂停
        std::cin.get();
    }
    else std::cout << sErr << std::endl;
    return 0;
}
