#include "../std.h"
#include "func_lib.h"

////////////////////////////////////////////////////////////////////////////////
// TFuncLib类
KC::one_level::TFuncLib::TFuncLib(IIDNameItemWork& wk, IKCActionData& act, string name, const TKcLoadFullFL& load)
    : TFuncLibItemBase(wk, act, name), m_loadSyn(load)
{
}

// 获取函数定义
TKcFuncDefFL& KC::one_level::TFuncLib::GetFuncDef(string funcName, IFuncCallDyn& dyn)
{
    // 模块名称
    string sModName = m_loadSyn.GetVal<0>().GetVal<0>().GetVal<0>().GetVal<0>();
    // 获取函数指针
    KLoadLibrary<>* pLib = m_loadSyn.GetVal<0>().GetVal<1>().get();
    if (nullptr == pLib)
        throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_work.getHint("Not_set_dynamic_libraries")) % (sModName + "." + funcName) % m_act.GetCurrLineID()).str(), m_work, sModName + "." + funcName);
    string sError = pLib->LoadLib();
    if (!sError.empty())
        throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, sError + " [" + sModName + "." + funcName + "], at line " + lexical_cast<string>(m_act.GetCurrLineID()), m_work, sModName + "." + funcName);
    TFuncPointer funcPtr = pLib->GetLibFunc<TFuncPointer>(funcName.c_str());
    if (nullptr == funcPtr)
        throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_work.getHint("Can_t_access_functions")) % (sModName + "." + funcName) % m_act.GetCurrLineID()).str(), m_work, sModName + "." + funcName);
    // 设置函数指针
    dyn.SetFunc(funcPtr);
    // 获取函数定义
    std::vector<TKcFuncDefFL>& fdList = *(m_loadSyn.GetVal<1>().ValList.get());
    for (auto& fd: fdList) if (fd.GetVal<1>() == funcName) return fd;
    throw TIDNameWorkSrvException(m_act.GetCurrLineID(), __FUNCTION__, (boost::format(m_work.getHint("Function_is_not_defined")) % (sModName + "." + funcName) % m_act.GetCurrLineID()).str(), m_work, sModName + "." + funcName);
}
