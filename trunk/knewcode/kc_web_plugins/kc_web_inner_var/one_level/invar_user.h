#pragma once

#include "../inner_var.h"

namespace KC { namespace one_level
{
    // user变量
    class TInvarUser : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarUser(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarUser() = default;

        // 得到名称
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // 得到内部变量值
        bool Exists(boost::any);
        // 得到内部变量值
        boost::any GetValue(boost::any);
        // 设置内部变量值
        void SetValue(boost::any, boost::any);

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;

    private:
        // user内部变量
        struct TInnerVarKey
        {
            // 键值
            boost::any key;
            // 构造、拷贝
            TInnerVarKey(boost::any k);
            TInnerVarKey(const TInnerVarKey& clone);
            TInnerVarKey operator=(const TInnerVarKey& rv);
            // 比较
            bool operator<(const TInnerVarKey& rv) const;
        };
        typedef map<TInnerVarKey, boost::any> TUserInVarList;
        TUserInVarList m_UserInVarList;

    private:
        // 添加user内部变量
        void SetUserInVar(boost::any, boost::any);
        // 获取user内部变量
        boost::any GetUserInVar(boost::any);
    };
}}
