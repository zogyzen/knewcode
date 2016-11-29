#include "std.h"
#include "idname_item_if.h"

#include "one_level/var_bool.h"
#include "one_level/var_double.h"
#include "one_level/var_int.h"
#include "one_level/var_string.h"
#include "one_level/func_lib.h"
#include "two_level/var_interface.h"

////////////////////////////////////////////////////////////////////////////////
// TVarItemBase类
KC::TVarItemBase::TVarItemBase(IIDNameItemWork& wk, IKCActionData& act, EKcDataType dt, string name)
    : m_work(wk), m_act(dynamic_cast<IKCActionData&>(act.GetActRoot())), m_dt(dt), m_varName(name)
{
}

// 获取页面活动数据
IKCActionData& KC::TVarItemBase::GetAction(void)
{
    return m_act;
}

// 创建变量
KC::TIDNameItermPtr KC::TVarItemBase::CreateVarItem(IIDNameItemWork& wk, IKCActionData& act, EKcDataType dt, string n, const TKcWebExpr& val)
{
    // 创建变量标识符
    TVarItemBase* pVar = nullptr;
    switch (dt)
    {
    case EKcDtBool:
        pVar = new one_level::TVarBool(wk, act, n);
        break;
    case EKcDtInt:
        pVar = new one_level::TVarInt(wk, act, n);
        break;
    case EKcDtDouble:
        pVar = new one_level::TVarDouble(wk, act, n);
        break;
    case EKcDtStr:
        pVar = new one_level::TVarString(wk, act, n);
        break;
    case EKcDtInterface:
        pVar = new two_level::TVarInterface(wk, act, n);
        break;
    default:
        throw TIDNameWorkSrvException(act.GetCurrLineID(), __FUNCTION__, "Unknow Type. Variable [" + n + "], at line " + lexical_cast<string>(act.GetCurrLineID()), wk, n);
        break;
    }
    // 赋值
    IOperandNode *valExpr = dynamic_cast<IOperandNode*>(val.exprTreeNodePtr.get());
    if (nullptr != valExpr)
        pVar->SetVarValue(valExpr->GetValue(act));
    return TIDNameItermPtr(pVar);
}

////////////////////////////////////////////////////////////////////////////////
// TFuncLibItemBase类
KC::TFuncLibItemBase::TFuncLibItemBase(IIDNameItemWork& wk, IKCActionData& act, string name)
    : m_work(wk), m_act(dynamic_cast<IKCActionData&>(act.GetActRoot())), m_libName(name)
{
}

// 获取页面活动数据
IKCActionData& KC::TFuncLibItemBase::GetAction(void)
{
    return m_act;
}

// 创建函数组
KC::TIDNameItermPtr KC::TFuncLibItemBase::CreateLibItem(IIDNameItemWork& wk, IKCActionData& act, string n, const TKcLoadFullFL& ld)
{
    if (nullptr == ld.GetVal<0>().GetVal<1>().get())
        throw TIDNameWorkSrvException(act.GetCurrLineID(), __FUNCTION__, (boost::format(wk.getHint("Not_load_dynamic_libraries")) % n % act.GetCurrLineID()).str(), wk, n);
    return TIDNameItermPtr(new one_level::TFuncLib(wk, act, n, ld));
}
