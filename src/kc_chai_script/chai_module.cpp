#include "chai_module.h"
#include "ctrl_chai.h"

////////////////////////////////////////////////////////////////////////////////
/// 基本模块 类
ChaiBaseModule::ChaiBaseModule(CCtrlChai& own, property_tree::ptree& pt)
    : m_own(own), m_pt(pt), m_baseMod(new chaiscript::Module()), m_context(own)
{
}
ChaiBaseModule::~ChaiBaseModule(void)
{
    m_exModList.clear();
    m_baseMod.reset();
}

// 初始化基础模块
void ChaiBaseModule::InitBaseMod(void)
{
    /// 添加chai脚本
    // m_baseMod->eval(R"(
    //     def fAdd(x, y) {
    //         return x + y;
    //     };
    //     print(fAdd("abc", "123"));
    // )");

    /// 添加C++方法
    // m_baseMod->add(chaiscript::fun([&](string sUrl){ ExecUrlApi(sUrl); }), "ExecApi");

    /// 添加C++类
    InitBaseCPPClass();

    /// 添加附加库
    InitAdditions(*this, m_pt, m_baseMod);
}
// 初始化基础C++类
void ChaiBaseModule::InitBaseCPPClass(void)
{
    // 添加 KcContext 类
    chaiscript::utility::add_class<KcContext>(*m_baseMod,
        "KcContext",
        {},
        {
            // 加密
            // {chaiscript::fun(&KcContext::DecryptDBConnStr), "DecryptDBConnStr"},
            {chaiscript::fun(&KcContext::EncryptDBConnStr), "EncryptDBConnStr"}
        }
    );
    m_baseMod->add(chaiscript::base_class<IChaiContext, KcContext>());

    // 添加 json 类
    chaiscript::utility::add_class<KcJson>(*m_baseMod,
        "KcJson",
        {},
        {
            // 获取值
            {chaiscript::fun(&KcJson::GetInt), "GetInt"},
            {chaiscript::fun(&KcJson::GetInt64), "GetInt64"},
            {chaiscript::fun(&KcJson::GetNum), "GetNum"},
            {chaiscript::fun(&KcJson::GetStr), "GetStr"},
            {chaiscript::fun(&KcJson::HasItem), "HasItem"},
            {chaiscript::fun(&KcJson::IsNull), "IsNull"},

            // 设置值
            {chaiscript::fun(static_cast<void(KcJson::*)(string, int)>(&KcJson::SetVal)), "SetVal"},
            {chaiscript::fun(static_cast<void(KcJson::*)(string, long long)>(&KcJson::SetVal)), "SetVal"},
            {chaiscript::fun(static_cast<void(KcJson::*)(string, double)>(&KcJson::SetVal)), "SetVal"},
            {chaiscript::fun(static_cast<void(KcJson::*)(string, string)>(&KcJson::SetVal)), "SetVal"},
            {chaiscript::fun(&KcJson::AddArray), "AddArray"}
        }
    );

    // 添加 全局值 类
    chaiscript::utility::add_class<KcGlobalVal>(*m_baseMod,
        "KcGlobalVal",
        {},
        {
            // 获取值
            {chaiscript::fun(&KcGlobalVal::GetInt), "GetInt"},
            {chaiscript::fun(&KcGlobalVal::GetInt64), "GetInt64"},
            {chaiscript::fun(&KcGlobalVal::GetNum), "GetNum"},
            {chaiscript::fun(&KcGlobalVal::GetStr), "GetStr"},

            // 设置值
            {chaiscript::fun(static_cast<void(KcGlobalVal::*)(string, int)>(&KcGlobalVal::SetVal)), "SetVal"},
            {chaiscript::fun(static_cast<void(KcGlobalVal::*)(string, long long)>(&KcGlobalVal::SetVal)), "SetVal"},
            {chaiscript::fun(static_cast<void(KcGlobalVal::*)(string, double)>(&KcGlobalVal::SetVal)), "SetVal"},
            {chaiscript::fun(static_cast<void(KcGlobalVal::*)(string, string)>(&KcGlobalVal::SetVal)), "SetVal"}
        }
    );

    // 添加 当前活动控制器类
    chaiscript::utility::add_class<KcActCtrl>(*m_baseMod,
        "KcActCtrl",
        {},
        {
            // 脚本环境上下文
            {chaiscript::fun(&KcActCtrl::Context), "Context"},

            // json
            {chaiscript::fun(&KcActCtrl::Request), "Request"},
            {chaiscript::fun(&KcActCtrl::Json), "Json"},
            {chaiscript::fun(&KcActCtrl::JsonPrev), "JsonPrev"},
            {chaiscript::fun(&KcActCtrl::Attach), "Attach"},

            // 全局值
            {chaiscript::fun(&KcActCtrl::GlobalVal), "GlobalVal"},

            // 执行其他控制器api
            {chaiscript::fun(&KcActCtrl::ExecUrlApi), "ExecUrlApi"}
        }
    );
    m_baseMod->add(chaiscript::base_class<IChaiKcActCtrl, KcActCtrl>());
}

// 初始化附加库
void ChaiBaseModule::InitAdditions(ChaiBaseModule& ths, const property_tree::ptree& pt, chaiscript::ModulePtr mod, ICtrlApiData *ctrlD)
{
    // 循环所有控制器
    string sAddition = "addition";
    if (pt.get_child_optional(sAddition))
    {
        const property_tree::ptree& ptAddition = pt.get_child(sAddition);
        BOOST_FOREACH(const boost::property_tree::ptree::value_type &v, ptAddition)
        {
            string sName = v.first;
            if (c_RESTful_xmlcomment != sName && c_RESTful_xmlattr != sName)
                InitAdditionOne(ths, sName, v.second, mod, ctrlD);
        }
    }
}
void ChaiBaseModule::InitAdditionOne(ChaiBaseModule& ths, string sName, const property_tree::ptree& pt, chaiscript::ModulePtr mod, ICtrlApiData *ctrlD)
{
    IBundleContext& cntx = ths.m_own.Own().getContext();
    // 链接对应的子脚本
    std::string sAttrUrl = string(c_RESTful_xmlattr) + "." + c_RESTful_url;
    if (nullptr != ctrlD && pt.get_child_optional(sAttrUrl))
    {
        // 获取脚本
        string sUrl = pt.get<string>(sAttrUrl);
        string FilePartFlagB = CUtilFunc::PCharSafeToStr(ctrlD->GetOtherInfo("FilePartFlag/begin", "------part::%s{{")),
               FilePartFlagE = CUtilFunc::PCharSafeToStr(ctrlD->GetOtherInfo("FilePartFlag/end", "------}}part::~%s"));
        string sScript = CScriptFunc::GetReplaceContent(*ctrlD, sUrl, FilePartFlagB, FilePartFlagE);
        // 执行脚本
        mod->eval(sScript);
    }

    // 对应的脚本文件
    std::string sAttrFile = string(c_RESTful_xmlattr) + "." + c_RESTful_file;
    if (pt.get_child_optional(sAttrFile))
    {
        // 读文件
        string sFile = pt.get<string>(sAttrFile);
        std::string sLocalFile = nullptr != ctrlD ? CUtilFunc::PCharSafeToStr(ctrlD->transItemUrlToFullPath(sFile.c_str()))
                                                  : cntx.transCfgPathToFullPath(sFile.c_str());
        string sScript = CUtilFunc::ReadTxtFile(sLocalFile);
        // 执行脚本
        mod->eval(sScript);
    }

    // C++扩展模块
    std::string sAttrSo = string(c_RESTful_xmlattr) + "." + c_RESTful_so;
    if (pt.get_child_optional(sAttrSo))
    {
        // 动态库
        string sSo = pt.get<string>(sAttrSo);
        std::string sLocalSo = nullptr != ctrlD ? CUtilFunc::PCharSafeToStr(ctrlD->transItemUrlToFullPath(sSo.c_str()))
                                                : cntx.transCfgPathToFullPath(sSo.c_str());
        // cout << sLocalSo << endl;
        // 加载动态库
        auto it = ths.m_exModList.find(sLocalSo);
        if (ths.m_exModList.end() != it && it->second.get() != nullptr)
            it->second->Inf().Add(mod, sName, pt);
        else
        {
            TAdditionModLibPtr newMod(new TAdditionModLib);
            newMod->Load(ths.m_own, sLocalSo, sLocalSo.c_str());
            ths.m_exModList.insert(std::make_pair(sLocalSo, newMod));
            newMod->Inf().Add(mod, sName, pt);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// KcContext 类
KcContext::KcContext(CCtrlChai& own) : m_own(own)
{
}

// 得到环境上下文
IKCContext& KcContext::getContext(void) const
{
    return m_own.Own().getContext();
}

// 加密数据库连接
string KcContext::EncryptDBConnStr(string str)
{
    return CCtrlCommon::EncryptConnStrDB(getContext(), str);
}
string KcContext::DecryptDBConnStr(string str)
{
    return CCtrlCommon::DecryptConnStrDB(getContext(), str);
}

/// json 类
KcJson::KcJson(IKCJson& kcJsn) : m_kcJson(kcJsn)
{
}
KcJson::KcJson(const IKCJson& kcJsn) : m_kcJson(const_cast<IKCJson&>(kcJsn))
{
}

// 获取值
int KcJson::GetInt(string sName) const
{
    return static_cast<int>(CCtrlCommon::GetSubItem(m_kcJson, sName).GetVal());
}
long long KcJson::GetInt64(string sName) const
{
    return static_cast<long long>(CCtrlCommon::GetSubItem(m_kcJson, sName).GetVal());
}
double KcJson::GetNum(string sName) const
{
    return CCtrlCommon::GetSubItem(m_kcJson, sName).GetVal();
}
string KcJson::GetStr(string sName) const
{
    return CCtrlCommon::GetSubItem(m_kcJson, sName).GetStr();
}
bool KcJson::IsNull(string sName) const
{
    return CCtrlCommon::GetSubItem(m_kcJson, sName).IsNull();
}
bool KcJson::HasItem(string sName) const
{
    auto [pJson, sLeaf] = CCtrlCommon::GetSubItemPath(m_kcJson, sName);
    return nullptr != pJson && pJson->IsValid() && pJson->HasItem(sLeaf.c_str());
}

// 设置值
void KcJson::SetVal(string sName, int val)
{
    CCtrlCommon::AddSubItem(m_kcJson, sName, [&](IKCJson& jsnParent, std::string sLeaf){
        if (jsnParent.GetType() == IKCJson::edtArray)
            jsnParent.SetVal(atoi(sLeaf.c_str()), val);
        else
            jsnParent.SetVal(sLeaf.c_str(), val);
    });
}
void KcJson::SetVal(string sName, long long val)
{
    CCtrlCommon::AddSubItem(m_kcJson, sName, [&](IKCJson& jsnParent, std::string sLeaf){
        if (jsnParent.GetType() == IKCJson::edtArray)
            jsnParent.SetVal(atoi(sLeaf.c_str()), static_cast<double>(val));
        else
            jsnParent.SetVal(sLeaf.c_str(), static_cast<double>(val));
    });
}
void KcJson::SetVal(string sName, double val)
{
    CCtrlCommon::AddSubItem(m_kcJson, sName, [&](IKCJson& jsnParent, std::string sLeaf){
        if (jsnParent.GetType() == IKCJson::edtArray)
            jsnParent.SetVal(atoi(sLeaf.c_str()), val);
        else
            jsnParent.SetVal(sLeaf.c_str(), val);
    });
}
void KcJson::SetVal(string sName, string val)
{
    CCtrlCommon::AddSubItem(m_kcJson, sName, [&](IKCJson& jsnParent, std::string sLeaf){
        if (jsnParent.GetType() == IKCJson::edtArray)
            jsnParent.SetStr(atoi(sLeaf.c_str()), val.c_str());
        else
            jsnParent.SetStr(sLeaf.c_str(), val.c_str());
    });
}
void KcJson::AddArray(string sName)
{
    CCtrlCommon::AddSubItem(m_kcJson, sName, [&](IKCJson& jsnParent, std::string sLeaf){
        if (jsnParent.GetType() == IKCJson::edtArray)
            jsnParent.AddArray(atoi(sLeaf.c_str()), true);
        else
            jsnParent.AddArray(sLeaf.c_str());
    });
}

////////////////////////////////////////////////////////////////////////////////
/// 全局值 类
KcGlobalVal::KcGlobalVal(IActionData& act) : m_act(act)
{
}

// 获取值
int KcGlobalVal::GetInt(string sName) const
{
    return atoi(m_act.GetGlobalVal(sName.c_str()));
}
long long KcGlobalVal::GetInt64(string sName) const
{
    return atoll(m_act.GetGlobalVal(sName.c_str()));
}
double KcGlobalVal::GetNum(string sName) const
{
    return atof(m_act.GetGlobalVal(sName.c_str()));
}
string KcGlobalVal::GetStr(string sName) const
{
    return CUtilFunc::PCharSafeToStr(m_act.GetGlobalVal(sName.c_str()));
}

// 设置值
void KcGlobalVal::SetVal(string sName, int val)
{
    m_act.SetGlobalVal(sName.c_str(), std::to_string(val).c_str());
}
void KcGlobalVal::SetVal(string sName, long long val)
{
    m_act.SetGlobalVal(sName.c_str(), std::to_string(val).c_str());
}
void KcGlobalVal::SetVal(string sName, double val)
{
    m_act.SetGlobalVal(sName.c_str(), std::to_string(val).c_str());
}
void KcGlobalVal::SetVal(string sName, string val)
{
    m_act.SetGlobalVal(sName.c_str(), val.c_str(), val.size());
}

////////////////////////////////////////////////////////////////////////////////
/// 当前活动控制器类
KcActCtrl::KcActCtrl(ICtrlApiData& ctrlD, KcContext& cont) : m_objCtrlD(ctrlD), m_context(cont)
    , m_jsonRequest(ctrlD.JsonRequest()), m_jsonRespond(ctrlD.JsonRespond())
    , m_jsonRespondLast(ctrlD.JsonRespondLast()) , m_jsonAttach(ctrlD.JsonAttach())
    , m_globalVal(ctrlD.ActionData())
{
}

// 得到脚本环境上下文
KcContext& KcActCtrl::Context(void) const
{
    return m_context;
}
IChaiContext& KcActCtrl::getContext(void) const
{
    return this->Context();
}

// 得到当前控制器Api数据接口
ICtrlApiData& KcActCtrl::getCtrlApiData(void) const
{
    return m_objCtrlD;
}

// 操作json
const KcJson& KcActCtrl::Request(void)
{
    return m_jsonRequest;
}
KcJson& KcActCtrl::Json(void)
{
    return m_jsonRespond;
}
const KcJson& KcActCtrl::JsonPrev(void)
{
    return m_jsonRespondLast;
}
const KcJson& KcActCtrl::Attach(void)
{
    return m_jsonAttach;
}

// 操作全局值
KcGlobalVal& KcActCtrl::GlobalVal(void)
{
    return m_globalVal;
}

// 执行其他api
int KcActCtrl::ExecUrlApi(string sUrl)
{
    return m_objCtrlD.WorkAct(sUrl.c_str(), "");
}
