#pragma once

namespace KC
{
    // 参数类型接口
    class IParmItem
    {
    public:
        // 添加参数
        virtual void AddArg(CKCFuncDyncall&) = 0;
        // 引用参数回调
        virtual void ParmRefBack(void) = 0;

    public:
        virtual ~IParmItem() = default;
    };
    typedef boost::shared_ptr<IParmItem> TParmItemPtr;
    typedef std::vector<TParmItemPtr> TParmItemPtrList;
}
