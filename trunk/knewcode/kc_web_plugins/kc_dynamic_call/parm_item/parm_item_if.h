#pragma once

namespace KC
{
    // �������ͽӿ�
    class IParmItem
    {
    public:
        // ��Ӳ���
        virtual void AddArg(CKCFuncDyncall&) = 0;
        // ���ò����ص�
        virtual void ParmRefBack(void) = 0;

    public:
        virtual ~IParmItem() = default;
    };
    typedef boost::shared_ptr<IParmItem> TParmItemPtr;
    typedef std::vector<TParmItemPtr> TParmItemPtrList;
}
