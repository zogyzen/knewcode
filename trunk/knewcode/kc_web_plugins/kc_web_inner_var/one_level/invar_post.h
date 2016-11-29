#pragma once

#include "../inner_var.h"
#include "invar_get.h"

namespace KC { namespace one_level
{
    // post变量
    class TInvarPost : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarPost(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarPost() = default;

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
        // 获取一个缓冲区参数
        void GetOneDataArg(const char*, const char*);
        // 获取参数二进制缓冲区
        const char* GetOneDataArgBuf(const char*, const char*);

    private:
        // 参数值类型
        struct ArgValueT
        {
            string name, val;
            enum ValTypeEnum { vteString = 0, vteFile };
            ValTypeEnum val_type = vteString;
            boost::shared_array<char> buf;
            int buf_size = 0;
            // 构造、拷贝
            ArgValueT(string n, string v = "") : name(n), val(v) {}
            ArgValueT(const ArgValueT& c) : name(c.name), val(c.val), val_type(c.val_type), buf(c.buf), buf_size(c.buf_size) {}
            ArgValueT& operator=(const ArgValueT& c)
            { name = c.name; val = c.val; val_type = c.val_type; buf = c.buf; buf_size = c.buf_size; return *this; }
        };

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;
        // POST类型
        string m_PostType;
        // POST参数组
        TInvarGet::ArgValListT<ArgValueT> m_mapArg;
    };
}}
