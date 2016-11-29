#include "../std.h"
#include "load_item.h"

////////////////////////////////////////////////////////////////////////////////
// CLoadItem类
KC::one_level::CLoadItem::CLoadItem(ISyntaxItemWork& work, IWebPageData& pd, int i, const TKcLoadFullFL& syn)
        : CSyntaxItemImpl(work, pd, i, syn), m_syn(syn)
{
    // cout << "Load: " << m_syn.GetVal<0>().GetVal<0>().GetVal<0>().GetVal<0>() << endl;
}

// 获取语法关键字
string KC::one_level::CLoadItem::GetKeychar(void) const
{
    return Keychar::one_level::g_KeywordLoadMod;
}

// 语法类型转换
const TKcLoadFullFL& KC::one_level::CLoadItem::DynCast(const TKcSynBaseClass& syn, IWebPageData&)
{
    const TKcLoadFullFL& result = dynamic_cast<const TKcLoadFullFL&>(syn);
    result.SynPosAttrList->insert(result.SynPosAttrList->begin(), result.GetVal<0>().SynPosAttrList->front());
    result.SynPosAttrList->push_back(result.GetVal<0>().SynPosAttrList->back());
    return result;
}

// 处理页数据
void KC::one_level::CLoadItem::Action(IKCActionData& act)
{
    // 模块名称
    string sModName = m_syn.GetVal<0>().GetVal<0>().GetVal<0>().GetVal<0>();
    // 获取路径值
    boost::any val = act.GetExprValue(m_syn.GetVal<0>().GetVal<0>().GetVal<1>());
    if (val.type() != typeid(string))
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, m_work.getHint("The_path_must_be_string_type_") + lexical_cast<string>(this->GetLineID()), m_work, this->GetBeginPtr(), this->GetEndPtr());
    string sPath = boost::trim_copy(boost::any_cast<string>(val));
    // 获取绝对路径
    sPath = act.GetFullPath(sPath);
    if (!boost::filesystem::exists(sPath + c_LoadOutLibExtName))
        throw TSyntaxWorkSrvException(this->GetLineID(), __FUNCTION__, (boost::format(m_work.getHint("Don_t_exists_load_include_file")) % this->GetKeychar() % sPath % this->GetLineID()).str(), m_work, this->GetBeginPtr(), this->GetEndPtr());
    // 添加动态库
    m_syn.GetVal<0>().SetVal<1>(m_pd.AddLoadLibrary(act, sPath));
    // 添加新加载库标识符定义
    act.AddIDName(sModName, this->getIIDNameInf().NewLibItem(act, sModName, m_syn), m_syn.GetVal<0>().GetVal<2>(), false, false);
}
