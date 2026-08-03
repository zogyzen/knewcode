#pragma once

#include "std.h"

// 日志功能库
class CChaiSys : public IChaiModAddition
{
public:
    CChaiSys(ICtrlChai& own, string dir);
    ~CChaiSys() override = default;

    // 向模块里添加语法
    void Add(chaiscript::ModulePtr, std::string name, const boost::property_tree::ptree& pt) override;
};
