#include "../std.h"
#include "var_interface.h"

////////////////////////////////////////////////////////////////////////////////
// TVarInterface类
KC::two_level::TVarInterface::TVarInterface(IIDNameItemWork& wk, IKCActionData& act, string name)
    : TVarItemBase(wk, act, EKcDtInterface, name), TFuncLibItemBase(wk, act, name)
{
}

// 变量赋值
void KC::two_level::TVarInterface::SetVarValue(boost::any val)
{
    if (!val.empty() && val.type() == typeid(TKcWebInfVal))
        m_value = boost::any_cast<TKcWebInfVal>(val);
    else
        throw TIDNameWorkSrvException(TVarItemBase::GetAction().GetCurrLineID(), __FUNCTION__, (boost::format(TVarItemBase::m_work.getHint("Can_t_be_assigned_to_the_interface")) % val.type().name() % m_varName % TVarItemBase::GetAction().GetCurrLineID()).str(), TVarItemBase::m_work, m_varName);
}

// 获取变量的值
boost::any KC::two_level::TVarInterface::GetVarValue(void)
{
    return m_value;
}

// 获取函数定义
TKcFuncDefFL& KC::two_level::TVarInterface::GetFuncDef(string funcName, IFuncCallDyn& dyn)
{
    if (nullptr == m_value.pointer)
        throw TIDNameWorkSrvException(TVarItemBase::GetAction().GetCurrLineID(), __FUNCTION__, TVarItemBase::m_work.getHint("The_interface_is_nullptr_") + m_varName, TVarItemBase::m_work, m_value.name);
    // 获取接口继承列表
    vector<TKcInfFullFL*> vecInf;
    this->GetBaseToTheInf(vecInf, m_value.name);
    // 从根类开始循环每个接口
    int iBaseFuncCount = 0;
    for (auto it = vecInf.rbegin(); vecInf.rend() != it; ++it)
    {
        // 获取接口定义
        TKcInfFullFL& infFL = **it;
        // 获取函数定义列表
        std::vector<TKcFuncDefFL>& fdList = *(infFL.GetVal<1>().ValList.get());
        for (auto& fd: fdList)
            // 查找函数定义
            if (fd.GetVal<1>() == funcName)
            {
                // 添加虚函数
                dyn.SetVirtualFunc(m_value.pointer, fd.GetVal<3>() + iBaseFuncCount);
                return fd;
            }
        // 加基类函数数量
        iBaseFuncCount += infFL.GetVal<1>().Count();
    }
    // 没找到，报异常
    throw TIDNameWorkSrvException(TVarItemBase::GetAction().GetCurrLineID(), __FUNCTION__, "Function is not defined [#interface<" + m_value.name + ">." + funcName + "], at line " + lexical_cast<string>(TVarItemBase::GetAction().GetCurrLineID()), TVarItemBase::m_work, funcName);
}

// 获取继承的系列接口
void KC::two_level::TVarInterface::GetBaseToTheInf(vector<TKcInfFullFL*>& vec, string inf)
{
    // 获取接口定义
    TKcInfFullFL& infFL = TVarItemBase::GetAction().GetKcInfFullFL(inf);
    // 插入接口
    vec.push_back(&infFL);
    // 获取父接口
    string infBase = infFL.GetVal<0>().GetVal<1>().GetVal<0>();
    if (!infBase.empty()) this->GetBaseToTheInf(vec, infBase);
}
