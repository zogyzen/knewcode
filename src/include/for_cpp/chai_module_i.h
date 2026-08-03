#pragma once

#include <boost/property_tree/ptree.hpp>

#include <chaiscript/chaiscript.hpp>

#include "for_user/page_interface.h"
#include "framework/bundle_context_i.h"

namespace KC
{
    // chai扩展模块接口
    class IChaiModAddition : public IKCObject
    {
    public:
        virtual ~IChaiModAddition() = default;

    public:
        // 向模块里添加语法
        virtual void Add(chaiscript::ModulePtr, std::string name, const boost::property_tree::ptree& pt) = 0;
    };

    // 脚本中环境上下文接口
    class IChaiContext
    {
    public:
        // 得到环境上下文
        virtual IKCContext& getContext(void) const = 0;
    };

    // 脚本中当前活动控制器接口
    class IChaiKcActCtrl
    {
    public:
        // 得到脚本环境上下文
        virtual IChaiContext& getContext(void) const = 0;

        // 得到当前控制器Api数据接口
        virtual ICtrlApiData& getCtrlApiData(void) const = 0;
    };
}
