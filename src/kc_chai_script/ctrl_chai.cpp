#include "ctrl_chai.h"

// 测试
class KcTest
{
public:
    void function() {}
    std::string function2() { return "Function2"; }
    void function3() {}
    std::string functionOverload(double) { return "double"; }
    std::string functionOverload(int) { return "int"; }
};
void TestChai(chaiscript::ChaiScript& chai)
{
    chai.add(chaiscript::fun([&](int i, double j){ return i * j; }), "funcAdd");
    chai.add(chaiscript::fun([&](int i, double j){ return i + j; }), "funcAdd");
    // int i = chai.eval<int>("meet();");

    try
    {
        double d = chai.eval<double>(R"(
            def meet {
                return 2;
            };
            var i = 10.1;
            i = 20.1;
            funcAdd(3, i) + meet();
        )");
        cout << d << endl;
    }
    catch (chaiscript::exception::load_module_error &ex)
    {
        cout << ex.what() << endl;
        throw;
    }
    catch (chaiscript::exception::eval_error &ex)
    {
        cout << ex.pretty_print() << endl << ex.reason << "\t" << ex.start_position.line << ":" << ex.start_position.column << endl;
        throw;
    }
    catch (chaiscript::exception::file_not_found_error &ex)
    {
        cout << ex.what() << endl;
        throw;
    }

    int i = 1;
    chai.add(chaiscript::var(i), "i");
    int j = 2;
    chai.add(chaiscript::var(j), "i");
    chai("print(i)");
    chai("i = 12");
    cout << i << "\t" << j << endl;

    // 添加C++类
    chaiscript::ModulePtr mod(new chaiscript::Module());
    chaiscript::utility::add_class<KcTest>(*mod,
        "KcTest",
        {
            chaiscript::constructor<KcTest()>(),
            chaiscript::constructor<KcTest(const KcTest &)>()
        },
        {
            {chaiscript::fun(&KcTest::function), "function"},
            {chaiscript::fun(&KcTest::function2), "function2"},
            {chaiscript::fun(&KcTest::function2), "function3"},
            {chaiscript::fun(static_cast<std::string (KcTest::*)(double)>(&KcTest::functionOverload)), "functionOverload"},
            {chaiscript::fun(static_cast<std::string (KcTest::*)(int)>(&KcTest::functionOverload)), "functionOverload"}
        }
    );
    chai.add(mod);

    // 添加chai脚本
    mod->eval(R"(
        def fAdd(x, y) {
            return x + y;
        };
        print(fAdd("abc", "123"));
    )");

    // 添加C++方法
    mod->add(chaiscript::fun([&](string sUrl){ return sUrl; }), "ExecApi");

    boost::lexical_cast<string>(1);

    KcTest t;
    chai.add(chaiscript::var(t), "tstCPP");

    chai.eval(R"(
        var tstChai = KcTest();
        print(tstChai.function2());
        print(tstCPP.functionOverload(1.2));
    )");

    chaiscript::Boxed_Value vResult = chai.eval(R"(
        print(this.ExecUrlApi("/api.kc"));
    )");

    return;
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlChai 类
CCtrlChai::CCtrlChai(IKCChaiScript& own, string sName, property_tree::ptree& pt)
    : m_own(own), m_name(sName), m_baseMod(*this, pt)
{
    // 记录函数地址
    m_own.WriteLogDebug((m_name + ": "
                            + "\nCCtrlChai::Perform: \t\t" + CUtilFunc::GetFuncAddr(&CCtrlChai::Perform)
                        ).c_str());
}

// 开始
bool CCtrlChai::start(void)
{
    // 初始化基础模块
    m_baseMod.InitBaseMod();
    return true;
}
// 执行脚本
void CCtrlChai::ExecScript(CSqlFunc::TConnObjWork<TChaiConnObj> &chaiObj, ICtrlApiData& objCtrlD, IAttachParmForCpp& attachP, string sScript, string /*sJsonParm*/)
{
    // 本次活动的脚本执行器
    auto &chai = chaiObj.ActObj().m_chai;

    // 添加模块功能
    try
    {
        // 添加基础模块
        chai.add(m_baseMod.m_baseMod);

        // 添加本控制器的附加库
        chaiscript::ModulePtr additionMod(new chaiscript::Module());
        ChaiBaseModule::InitAdditions(m_baseMod, attachP.GetCtrlApiNode(), additionMod, &objCtrlD);
        chai.add(additionMod);
    }
    catch (std::exception &ex)
    {
        cout << "<" << typeid(ex).name() << "> " << ex.what() << endl;
    }

    // 设置当前控制器api数据
    KcActCtrl actCtrl(objCtrlD, m_baseMod.m_context);
    chai.add(chaiscript::var(actCtrl), "This");

    // 测试
    // TestChai(chai);

    // 执行
    chai.eval(sScript);
}

// 执行控制器
void CCtrlChai::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm& attachP)
{
    // 控制器参数
    const char* pSignName = objCtrlD.SignName();
    string sSignName = CUtilFunc::PCharSafeToStr(pSignName);

    [[maybe_unused]] const char* pLocalFile = objCtrlD.LocalFile();
    const char* pAct = objCtrlD.ActName();
    const char* pMethod = objCtrlD.Method();
    const char* pScript = objCtrlD.Content();
    IActionData& act = objCtrlD.ActionData();
    string sScript, sJsonParm, sAct = CUtilFunc::PCharSafeToStr(pAct);;
    auto fExceptInfo = [&](void)
    {
        return (format("\n%s\n%s\n%s") % sScript % act.GetSingleInfo("the_request") % sJsonParm).str();
    };
    try
    {
        IAttachParmForCpp *pAttchPrm = dynamic_cast<IAttachParmForCpp*>(&attachP);
        if (nullptr != pAttchPrm)
        {
            // 语句
            if (nullptr == pScript || strlen(pScript) == 0)
                throw TKCChaiScriptException(214, __CURR_CODE_PLACE_C__, string(m_own.getHint("SQL_Empty_")) + act.GetSingleInfo("the_request") + " - " + pAct, m_own);
            sScript = pScript;
            // 参数
            sJsonParm = objCtrlD.JsonRequest().ToStr();
            cout << "\t" << CUtilFunc::Utf8ToGbk(sJsonParm) << endl;
            const char* pKCCLNID = act.GetGlobalVal("KC__SESSION__KCCLNID");
            m_own.WriteLogDebug(act.GetSingleInfo("the_request"), __CURR_CODE_PLACE_C__, (sJsonParm + "\nKCCLNID=" + (nullptr != pKCCLNID ? pKCCLNID : "")).c_str());
            // 脚本执行器
            CSqlFunc::TConnObjWork<TChaiConnObj> actObjWork(objCtrlD, /*m_name + "@kc_chai_script",*/ [&](){ return new TChaiConnObj(*this); });
            ExecScript(actObjWork, objCtrlD, *pAttchPrm, sScript, sJsonParm);
        }
    }
    catch (chaiscript::exception::load_module_error &ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecsChai_ErrCode_load_module_error, ex.what());
    }
    catch (chaiscript::exception::eval_error &ex)
    {
        // 得到错误的行列
        int iRow = 0, iColBegin = 0, iColEnd = 0;
        if (!ex.call_stack.empty())
        {
            iRow = ex.call_stack[0].start().line;
            iColBegin = ex.call_stack[0].start().column;
            iColEnd = ex.call_stack[0].end().column;
        }
        // 得到错误的代码
        string sCodeErr = CUtilFunc::GetStrLine(sScript, iRow);
        // if (iColEnd - iColBegin > 0) sCodeErr = sCodeErr.substr(iColBegin - 1, iColEnd - iColBegin);
        // sCodeErr = sCodeErr.substr(iColBegin - 1, std::max(iColEnd - iColBegin, 256));
        // sCodeErr = boost::algorithm::trim_copy(sCodeErr).substr(0, std::max(iColEnd - iColBegin, 256));
        sCodeErr = sCodeErr.substr(0, std::max(iColEnd, 150));
        // 错误信息
        string sErr = (boost::format("[%s (%d: %d~%d)] `%s`  %s") % sSignName % iRow % iColBegin % iColEnd % sCodeErr % ex.what()).str();
        string sErrFull = sErr + "\n" + ex.pretty_print() + fExceptInfo();
        m_own.WriteLogError(sErrFull.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecsChai_ErrCode_eval_error, sErr.c_str());
    }
    catch (chaiscript::exception::file_not_found_error &ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogError(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecsChai_ErrCode_file_not_found_error, ex.what());
    }
    catch(TException& ex)
    {
        ex.LineCode() = __LINE__;
        ex.OtherInfo() = fExceptInfo();
        ex.CurrPosInfo() = __CURR_CODE_PLACE_C__;
        m_own.WriteLog(ex);
        objCtrlD.SetJsonRespond(ex.error_id(), ex.what());
    }
    catch (std::exception& ex)
    {
        string sErr = ex.what() + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__, typeid(ex).name());
        objCtrlD.SetJsonRespond(ecsChai_ErrCode_UnknownCpp, ex.what());
    }
    catch (...)
    {
        string sErr = m_own.getHint("Unknown_exception") + fExceptInfo();
        m_own.WriteLogFatal(sErr.c_str(), __CURR_CODE_PLACE_C__);
        objCtrlD.SetJsonRespond(ecsChai_ErrCode_UnknownOther, (string("ChaiScript ") + m_own.getHint("Unknown_exception")).c_str());
    }
}
