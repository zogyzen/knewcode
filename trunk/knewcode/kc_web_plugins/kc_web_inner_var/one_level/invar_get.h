#pragma once

#include "../inner_var.h"

namespace KC { namespace one_level
{
    // get变量
    class TInvarGet : public IKCRequestRespond::IReStepData
    {
    public:
        TInvarGet(IKCActionData&, CInnerVar&);
        virtual CALL_TYPE ~TInvarGet() = default;

        // 得到名称
        virtual const char* CALL_TYPE GetName(void);
        static const char* GetNameS(void);

        // 得到内部变量值
        bool Exists(boost::any);
        // 得到内部变量值
        boost::any GetValue(boost::any);
        // 设置内部变量值
        void SetValue(boost::any, boost::any);

    public:
        // 参数列表类型
        template<typename ValT>
        using ArgValListT = map<string, ValT>;
        // 初始化参数
        template<typename ValT>
        static void InitParms(ArgValListT<ValT>& arg, string str, string splt = "&")
        {
            // todo: str需要编码转换
            vector<string> vecArg;
            boost::split(vecArg, str, boost::is_any_of(splt));
            for (string s: vecArg)
            {
                string sOne = boost::trim_copy(s);
                vector<string> vecVal;
                boost::split(vecVal, sOne, boost::is_any_of("="));
                if (vecVal.size() > 1)
                    arg.insert(make_pair(vecVal[0], ValT(vecVal[0], vecVal[1])));
            }
        }
        // 获取参数值
        template<typename ValT>
        static string GetStringValue(ArgValListT<ValT>& arg, boost::any arr)
        {
            string sArr = TInvarGet::GetArrIndex(arr), sResult = "";
            // 获取参数值
            auto itVal = arg.find(sArr);
            if (arg.end() != itVal)
                sResult = itVal->second.val;
            return sResult;
        }
        // 获取下标
        static string GetArrIndex(boost::any);

    private:
        // 参数值类型
        struct ArgValueT
        {
            string name, val;
            // 构造、拷贝
            ArgValueT(string n, string v) : name(n), val(v) {}
            ArgValueT(const ArgValueT& c) : name(c.name), val(c.val) {}
            ArgValueT& operator=(const ArgValueT& c) { name = c.name; val = c.val; return *this; }
        };

    private:
        IKCActionData& m_act;
        CInnerVar& m_InnerVar;
        // 参数组
        ArgValListT<ArgValueT> m_mapArg;
    };
}}
