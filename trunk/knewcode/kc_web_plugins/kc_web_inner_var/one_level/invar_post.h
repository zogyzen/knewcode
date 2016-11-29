#pragma once

#include "../inner_var.h"
#include "invar_get.h"

namespace KC { namespace one_level
{
    // post����
    class TInvarPost : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarPost(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarPost() = default;

        // �õ�����
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // �õ��ڲ�����ֵ
        bool Exists(boost::any);
        // �õ��ڲ�����ֵ
        boost::any GetValue(boost::any);
        // �����ڲ�����ֵ
        void SetValue(boost::any, boost::any);

    private:
        // ��ȡһ������������
        void GetOneDataArg(const char*, const char*);
        // ��ȡ���������ƻ�����
        const char* GetOneDataArgBuf(const char*, const char*);

    private:
        // ����ֵ����
        struct ArgValueT
        {
            string name, val;
            enum ValTypeEnum { vteString = 0, vteFile };
            ValTypeEnum val_type = vteString;
            boost::shared_array<char> buf;
            int buf_size = 0;
            // ���졢����
            ArgValueT(string n, string v = "") : name(n), val(v) {}
            ArgValueT(const ArgValueT& c) : name(c.name), val(c.val), val_type(c.val_type), buf(c.buf), buf_size(c.buf_size) {}
            ArgValueT& operator=(const ArgValueT& c)
            { name = c.name; val = c.val; val_type = c.val_type; buf = c.buf; buf_size = c.buf_size; return *this; }
        };

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;
        // POST����
        string m_PostType;
        // POST������
        TInvarGet::ArgValListT<ArgValueT> m_mapArg;
    };
}}
