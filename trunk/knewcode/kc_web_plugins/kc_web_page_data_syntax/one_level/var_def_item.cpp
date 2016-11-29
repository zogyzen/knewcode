#include "../std.h"
#include "var_def_item.h"

////////////////////////////////////////////////////////////////////////////////
// CVarDefItem类
KC::one_level::CVarDefItem::CVarDefItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcVarDef& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::one_level::CVarDefItem::GetKeychar(void) const
{
    return "VARIABLE_DEFINE";
}

// 语法类型转换
const TKcVarDef& KC::one_level::CVarDefItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    const TKcVarDef& result = dynamic_cast<const TKcVarDef&>(syn);
    result.SynPosAttrList->insert(result.SynPosAttrList->begin(), result.GetVal<0>().SynPosAttrList->front());
    return result;
}

// 处理页数据
void KC::one_level::CVarDefItem::Action(IKCActionData& act)
{
    // 变量的类型
    EKcDataType dt = m_syn.GetVal<0>().GetVal<0>();
    if (EKcDtVoid == dt)
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, m_work.getHint("Can_t_define_void_variable") + lexical_cast<string>(this->GetLineID()), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 循环添加每个变量定义
    for (TKcVarAssDef& var: *(m_syn.GetVal<1>().ValList.get()))
        act.AddIDName(var.GetVal<0>().GetVal<0>(), this->getIIDNameInf().NewVarItem(act, dt, var.GetVal<0>().GetVal<0>(), var.GetVal<1>()), m_syn.GetVal<2>(), m_syn.GetVal<3>(), m_syn.GetVal<4>());
}
