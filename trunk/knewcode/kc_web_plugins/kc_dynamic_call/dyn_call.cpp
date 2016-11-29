#include "std.h"
#include "dyn_call.h"

#include "parm_item/one_level/parm_bool.h"
#include "parm_item/one_level/parm_int.h"
#include "parm_item/one_level/parm_double.h"
#include "parm_item/one_level/parm_string.h"
#include "parm_item/two_level/parm_interface.h"

////////////////////////////////////////////////////////////////////////////////
// CDynamicCall类
CDynamicCall::CDynamicCall(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle)
{
}

CDynamicCall::~CDynamicCall()
{
}

// 得到服务特征码
const char* CDynamicCall::getGUID(void) const
{
    return c_DynamicCallSrvGUID;
}

// 对应的模块
const IBundle& CDynamicCall::getBundle(void) const
{
    return m_bundle;
}

// 创建函数调用接口
IFuncCallDyn& CDynamicCall::NewFuncCall(IKCActionData& act)
{
    return *new CFuncCall(*this, act);
}

// 释放函数调用接口
void CDynamicCall::FreeFuncCall(IFuncCallDyn& fc)
{
    delete dynamic_cast<CFuncCall*>(&fc);
}

////////////////////////////////////////////////////////////////////////////////
// CDynamicCall::CFuncCall类
CDynamicCall::CFuncCall::CFuncCall(CDynamicCall& fct, IKCActionData& act)
    : m_fct(fct), m_act(dynamic_cast<IKCActionData&>(act.GetActRoot()))
{
}

CDynamicCall::CFuncCall::~CFuncCall(void)
{
}

// 加实参
void CDynamicCall::CFuncCall::AddArg(boost::any& val, TKcParmFL& dtParm, TParmRefBackPtr RefPtr)
{
    IParmItem* pParmItem = nullptr;
    switch (dtParm.GetVal<0>().GetVal<0>())
    {
    case EKcDtBool:
        pParmItem = new parm_item::one_level::TParmBool(val, dtParm, RefPtr);
        break;
    case EKcDtInt:
        pParmItem = new parm_item::one_level::TParmInt(val, dtParm, RefPtr);
        break;
    case EKcDtDouble:
        pParmItem = new parm_item::one_level::TParmDouble(val, dtParm, RefPtr);
        break;
    case EKcDtStr:
        pParmItem = new parm_item::one_level::TParmString(val, dtParm, RefPtr);
        break;
    case EKcDtInterface:
        pParmItem = new parm_item::two_level::TParmInterfacer(val, dtParm, RefPtr);
        break;
    default:
        throw TDynamicCallException(m_act.GetCurrLineID(), __FUNCTION__, m_fct.getHint("Parameter_type_unknown_") + lexical_cast<string>((int)dtParm.GetVal<0>().GetVal<0>()), m_fct);
        break;
    }
    m_ParmItemPtrList.push_back(TParmItemPtr(pParmItem));
}

// 设置返回值类型
void CDynamicCall::CFuncCall::SetResult(TKcParmFL& dt)
{
    m_dtRes = dt;
    switch (dt.GetVal<0>().GetVal<0>())
    {
    case EKcDtBool:
        m_KCDyncall.SetResult(CKCFuncDyncall::DtBool);
        break;
    case EKcDtInt:
        m_KCDyncall.SetResult(CKCFuncDyncall::DtInt);
        break;
    case EKcDtDouble:
        m_KCDyncall.SetResult(CKCFuncDyncall::DtDouble);
        break;
    case EKcDtStr:
        m_KCDyncall.SetResult(CKCFuncDyncall::DtPointer);
        break;
    case EKcDtInterface:
        m_KCDyncall.SetResult(CKCFuncDyncall::DtInterface);
        break;
    default:
        m_KCDyncall.SetResult();
        break;
    }
}

// 设置函数指针
void CDynamicCall::CFuncCall::SetFunc(TFuncPointer pFunc)
{
    m_KCDyncall.SetFunc((CKCFuncDyncall::TFuncPointer)pFunc);
}

// 设置成员函数指针
void CDynamicCall::CFuncCall::SetMemberFunc(TObjectPointer pThis, TFuncPointer pFunc)
{
    m_KCDyncall.SetMemberFunc((CKCFuncDyncall::TObjectPointer)pThis, (CKCFuncDyncall::TFuncPointer)pFunc);
}

// 设置虚函数指针
void CDynamicCall::CFuncCall::SetVirtualFunc(TObjectPointer pThis, int FuncIndex)
{
    m_KCDyncall.SetVirtualFunc((CKCFuncDyncall::TObjectPointer)pThis, FuncIndex);
}

// 调用函数
boost::any CDynamicCall::CFuncCall::Call(string sInfo)
{
    boost::any result;
    try
    {
        // 添加实参
        for (TParmItemPtr parm: m_ParmItemPtrList) parm->AddArg(m_KCDyncall);
        // 调用函数
        void* pResult = nullptr;
        double dResult = 0;
        auto TryFunc = [&]()
        {
            m_KCDyncall.CallFunc(pResult, dResult);
        };
        // 信号捕捉
        TSignoTry<decltype(TryFunc)> sigTry(TryFunc, m_act.GetRequestRespond(), sInfo.c_str());
        m_act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
        // 处理返回值
        if (EKcDtBool == m_dtRes.GetVal<0>().GetVal<0>())
            // 布尔类型
            result = (bool)(int)pResult;
        else if (EKcDtInt == m_dtRes.GetVal<0>().GetVal<0>())
            // 整型
            result = (int)pResult;
        else if (EKcDtDouble == m_dtRes.GetVal<0>().GetVal<0>())
            // 浮点型
            result = dResult;
        else if (EKcDtStr == m_dtRes.GetVal<0>().GetVal<0>())
            // 字符串类型
            result = parm_item::one_level::TParmString::TranResult(pResult);
        else if (EKcDtInterface == m_dtRes.GetVal<0>().GetVal<0>())
            // 接口类型
            result = parm_item::two_level::TParmInterfacer::TranResult(pResult, m_dtRes.GetVal<1>().GetVal<0>());
        // 回调引用实参的值
        for (TParmItemPtr parm: m_ParmItemPtrList) parm->ParmRefBack();
    }
    catch(const char* ex)
    {
        throw TDynamicCallException(m_act.GetCurrLineID(), __FUNCTION__, ex, m_fct);
    }
    catch(int ex)
    {
        throw TDynamicCallException(m_act.GetCurrLineID(), __FUNCTION__, lexical_cast<string>(ex), m_fct);
    }
    catch(std::exception& ex)
    {
        throw TDynamicCallException(m_act.GetCurrLineID(), __FUNCTION__, ex.what() + string(" - ") + typeid(ex).name(), m_fct);
    }
    return result;
}
