#include "../std.h"
#include "operand_func.h"

#include "../work_expr_tree.h"
#include "operand_var.h"
#include "operand_invar.h"

////////////////////////////////////////////////////////////////////////////////
// 动态参数回调接口
template<typename TVAR>
class CParmRefBack : public IFuncCallDyn::IParmRefBack
{
public:
    CParmRefBack(IKCActionData& act, TVAR *pVarNode) : m_act(act), m_pVarNode(pVarNode) {}
    virtual ~CParmRefBack() = default;

    // 设置引用的返回值
    virtual void SetValue(boost::any& val)
    {
        if (nullptr != m_pVarNode)
            m_pVarNode->SetValue(m_act, val);
    }

private:
    IKCActionData& m_act;
    TVAR *m_pVarNode = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// TFuncallNode类
KC::one_level::TFuncallNode::TFuncallNode(IExprTreeWork& wk, IWebPageData& pd, const TKcFuncCallSyn& syn)
    : TOperandNode(wk, pd, syn), m_syn(syn)
{
}

// 获取操作数的值
boost::any KC::one_level::TFuncallNode::GetValue(IKCActionData& act)
{
    boost::any resVal;
    // 标识符命名接口
    IIDNameItemWork& idWk = dynamic_cast<CExprTreeWork&>(m_work).getIDNameInf();
    // 正常调用的接口变量名称
    string sLibName = trim_copy(m_syn.GetVal<0>().GetVal<0>());
    // 循环每个函数体
    for (int i = 0, j = (int)m_syn.GetVal<2>().ValList->size(); i < j; ++i)
    {
        // 函数体语法
        TKcFuncCallBodySyn& syn = (*m_syn.GetVal<2>().ValList)[i];
        // 得到函数组标识符（库、接口）
        IFuncLibItemBase* pLib = nullptr;
        TIDNameItermPtr LibPtr;
        // 标准调用方式
        if (0 == i && sLibName.size() > 0)
            pLib = dynamic_cast<IFuncLibItemBase*>(&act.GetIDName(sLibName));
        // 匿名调用方式（内部变量调用或函数连续调用）
        else
        {
            // 匿名接口变量名
            if (0 == i)
                sLibName = "AnonymInfVar" + posix_time::to_iso_string(posix_time::second_clock::local_time()) + "R" + lexical_cast<string>(rand());
            // 创建匿名接口
            if (0 == i)
                LibPtr = idWk.NewVarItem(act, EKcDtInterface, sLibName, m_syn.GetVal<1>());
            else
            {
                TIDNameItermPtr NewLibPtr = idWk.NewVarItem(act, EKcDtInterface, sLibName, TKcWebExpr());
                IVarItemBase* pVar = dynamic_cast<IVarItemBase*>(NewLibPtr.get());
                if (nullptr != pVar && !resVal.empty())
                {
                    pVar->SetVarValue(resVal);
                    LibPtr = NewLibPtr;
                }
            }
            pLib = dynamic_cast<IFuncLibItemBase*>(LibPtr.get());
        }
        // 调用函数
        resVal = this->RunFunc(act, pLib, sLibName, syn);
        sLibName += "0F" + lexical_cast<string>(i) + syn.GetVal<0>().GetVal<0>();
    }
    return resVal;
}

// 执行函数
boost::any KC::one_level::TFuncallNode::RunFunc(IKCActionData& act, IFuncLibItemBase* pLib, string sLibName, TKcFuncCallBodySyn& syn)
{
    if (nullptr == pLib)
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Don_t_exists_function_library_or_interface")) % sLibName % this->GetLineID()).str(), m_work);
    // 创建动态调用函数对象
    IDynamicCall& dynFct = dynamic_cast<CExprTreeWork&>(m_work).getDynamicCallInf();
    IFuncCallDyn& dyn = dynFct.NewFuncCall(act);
    CAutoRelease _autoFreeFunc(boost::bind(&IDynamicCall::FreeFuncCall, &dynFct, boost::ref(dyn)));
    // 得到函数名
    string sFuncName = syn.GetVal<0>().GetVal<0>();
    // 得到函数定义
    TKcFuncDefFL& funcDef = pLib->GetFuncDef(sFuncName, dyn);
    // 判断形参和实参是否数量一致
    if (funcDef.GetVal<2>().ValList->size() != syn.GetVal<1>().ValList->size())
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Inconsistent_number_of_formal_parameters_and_arguments")) % (sLibName + "." + sFuncName) % this->GetLineID()).str(), m_work);
    // 设置返回值
    TKcParmFL dtRes = funcDef.GetVal<0>();
    dyn.SetResult(dtRes);
    // 循环添加每个实参
    for (int i = 0, j = (int)syn.GetVal<1>().ValList->size(); i < j; ++i)
    {
        // 形参类型
        TKcParmFL& dtParm = (*funcDef.GetVal<2>().ValList)[i];
        if (EKcDtVoid == dtParm.GetVal<0>().GetVal<0>())
            throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Parameter_can_t_be_a_void_type")) % i % (sLibName + "." + sFuncName) % this->GetLineID()).str(), m_work);
        // 实参的值
        IOperandNode *pOperandL = dynamic_cast<IOperandNode*>((*syn.GetVal<1>().ValList)[i].exprTreeNodePtr.get());
        if (nullptr == pOperandL)
            throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Argument_can_t_be_a_void_type")) % i % (sLibName + "." + sFuncName) % this->GetLineID()).str(), m_work);
        boost::any val = pOperandL->GetValue(act);
        // 动态参数回调接口
        IFuncCallDyn::TParmRefBackPtr refBack;
        // 变量（或内部变量）引用，需要回调
        if (dtParm.GetVal<2>())
        {
            TVarNode *pVarNode = dynamic_cast<TVarNode*>(pOperandL);
            TInnerVarNode *pInVarNode = dynamic_cast<TInnerVarNode*>(pOperandL);
            if (nullptr != pVarNode)
                refBack = IFuncCallDyn::TParmRefBackPtr(new CParmRefBack<TVarNode>(act, pVarNode));
            else if (nullptr != pInVarNode)
                refBack = IFuncCallDyn::TParmRefBackPtr(new CParmRefBack<TInnerVarNode>(act, pInVarNode));
        }
        // 添加参数
        dyn.AddArg(val, dtParm, refBack);
    }
    // 调用函数
    boost::any resVal;
    try
    {
        resVal = dyn.Call(sLibName + "." + sFuncName);
        m_work.WriteLogTrace(("call function end: " + sLibName + "." + sFuncName).c_str(), __FUNCTION__, act.GetRequestRespond().GetLocalFilename());
    }
    catch(TDynamicCallException& e)
    {
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, e.error_info() + " [" + sLibName + "." + sFuncName + "], at line " + lexical_cast<string>(this->GetLineID()), m_work);
    }
    catch(boost::bad_lexical_cast& e)
    {
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, string(e.what()) + " [" + sLibName + "." + sFuncName + "], at line " + lexical_cast<string>(this->GetLineID()), m_work);
    }
    catch(std::exception& e)
    {
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, string(e.what()) + " [" + sLibName + "." + sFuncName + "], at line " + lexical_cast<string>(this->GetLineID()), m_work);
    }
    catch(...)
    {
        throw TExprTreeWorkSrvException(this->GetLineID(), __FUNCTION__, string(m_work.getHint("Unknown_exception")) + " [" + sLibName + "." + sFuncName + "], at line " + lexical_cast<string>(this->GetLineID()), m_work);
    }
    return resVal;
}
