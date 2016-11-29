#include "../std.h"
#include "delay_item.h"
#include "../work_syntax_item.h"

////////////////////////////////////////////////////////////////////////////////
// CDelayItem类
KC::two_level::CDelayItem::CDelayItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcDelayDef& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
}

// 获取语法关键字
string KC::two_level::CDelayItem::GetKeychar(void) const
{
    return Keychar::two_level::g_KeywordEvent;
}

// 语法类型转换
const TKcDelayDef& KC::two_level::CDelayItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcDelayDef&>(syn);
}

// 处理页数据
void KC::two_level::CDelayItem::Action(IKCActionData& act)
{
    // 根据延迟名称，选择活动接口
    auto FAct = [&]() -> IKCActionData&
    {
        string sType = m_syn.GetVal<0>().GetVal<0>();
        if (c_SyntaxDelayCPF == sType) return act;
        else if (c_SyntaxDelayTPF == sType)
            return dynamic_cast<IKCActionData&>(act.GetActRoot());
        else return act;
    };
    IKCActionData& actDst = FAct();
    // 创建延迟语法项
    CSyntaxItemWork& work = dynamic_cast<CSyntaxItemWork&>(m_work);
    TSyntaxItemPtr SynItm;
    switch (m_syn.GetVal<1>())
    {
    case 0:
        SynItm = work.NewSyntaxItem(m_syn.GetVal<2>(), actDst.GetWebPageData(), actDst.DelaySynNewIndex());
        break;
    case 1:
        SynItm = work.NewSyntaxItem(m_syn.GetVal<3>(), actDst.GetWebPageData(), actDst.DelaySynNewIndex());
        break;
    case 2:
        SynItm = work.NewSyntaxItem(m_syn.GetVal<4>(), actDst.GetWebPageData(), actDst.DelaySynNewIndex());
        break;
    default:
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(work.getHint("Unmatch_Syntax_Item")) % m_syn.GetVal<1>() % this->GetLineID()).str(), work, this->GetBeginPtr(), this->GetEndPtr());
        break;
    }
    // 添加到活动接口里
    actDst.AddDelaySyn(SynItm);
}
