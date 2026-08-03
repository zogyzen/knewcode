#include "chai_sys.h"

CChaiSys *g_ctrl = nullptr;
extern "C"
{
    IChaiModAddition& CALL_TYPE create(ICtrlChai& own, const char* path)
    {
        if (nullptr == g_ctrl)
        {
            // ExceptBacktrace::SetExceptFilter(own.getPath());
            g_ctrl = new CChaiSys(own, CUtilFunc::PCharSafeToStr(path));
        }
        return *g_ctrl;
    }
    void CALL_TYPE destroy(IChaiModAddition&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// 数学算法类
class KcMath
{
public:
    double Pow(double x, double y)
    {
        return std::pow(x, y);
    }
};

////////////////////////////////////////////////////////////////////////////////
// CChaiSys 类
CChaiSys::CChaiSys(ICtrlChai& own, string path)
{
    // own.Own().getContext();
}

// 向模块里添加语法
void CChaiSys::Add(chaiscript::ModulePtr mod, std::string name, const boost::property_tree::ptree& pt)
{
    // 类名前缀
    string sPrefix;
    std::string sAttrPrefix = string(c_RESTful_xmlattr) + ".classPrefix";
    if (pt.get_child_optional(sAttrPrefix))
        sPrefix = pt.get<string>(sAttrPrefix);

    // 添加模块功能
    try
    {
        // 添加数学算法类
        chaiscript::utility::add_class<KcMath>(*mod,
            sPrefix + "KcMath",
            {
                chaiscript::constructor<KcMath()>()
            },
            {
                {chaiscript::fun(&KcMath::Pow), "Pow"},
            }
        );
        // 添加系统内置函数和运算符
        mod->add(chaiscript::fun([](const std::string &l, int r){ return l + std::to_string(r); }), "+");
        mod->add(chaiscript::fun([](int l, const std::string &r){ return std::to_string(l) + r; }), "+");
        mod->add(chaiscript::fun([](const std::string &l, double r){ return l + std::to_string(r); }), "+");
        mod->add(chaiscript::fun([](double l, const std::string &r){ return std::to_string(l) + r; }), "+");
    }
    catch (std::exception &ex)
    {
        cout << "<" << typeid(ex).name() << "> " << ex.what() << endl;
    }
}
