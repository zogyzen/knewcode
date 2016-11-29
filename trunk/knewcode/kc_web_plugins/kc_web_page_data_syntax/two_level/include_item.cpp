#include "../std.h"
#include "include_item.h"

////////////////////////////////////////////////////////////////////////////////
// CIncludeItem类
KC::two_level::CIncludeItem::CIncludeItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcIncludeCL& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
    // cout << "Include: " << this->GetLineID() << "\t" << string(m_syn.GetVal<1>() ? Keychar::two_level::g_OptionPrivate : "") << endl;
}

// 获取语法关键字
string KC::two_level::CIncludeItem::GetKeychar(void) const
{
    return Keychar::two_level::g_KeywordInclude;
}

// 语法类型转换
const TKcIncludeCL& KC::two_level::CIncludeItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    return dynamic_cast<const TKcIncludeCL&>(syn);
}

// 处理页数据
void KC::two_level::CIncludeItem::Action(IKCActionData& act)
{
    // 获取路径值
    boost::any val = act.GetExprValue(m_syn.GetVal<0>());
    if (val.type() != typeid(string))
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, m_work.getHint("The_path_must_be_string_type_") + lexical_cast<string>(this->GetLineID()), m_work, this->GetBeginPtr(), this->GetEndPtr());
    string sPath = boost::trim_copy(boost::any_cast<string>(val));
    // 获取绝对路径
    sPath = act.GetFullPath(sPath);
    if (!boost::filesystem::exists(sPath))
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Don_t_exists_load_include_file")) % this->GetKeychar() % sPath % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 执行页面
    this->getPageFactoryRef().WebPageData(act, sPath.c_str()).Action(act, m_syn.GetVal<1>());
}
