#pragma once

#include "std.h"

// 日志功能库
class CChaiSQL : public IChaiModAddition
{
public:
    CChaiSQL(ICtrlChai& own, string dir);
    ~CChaiSQL() override = default;

    // 向模块里添加语法
    void Add(chaiscript::ModulePtr, std::string name, const boost::property_tree::ptree& pt) override;
};
