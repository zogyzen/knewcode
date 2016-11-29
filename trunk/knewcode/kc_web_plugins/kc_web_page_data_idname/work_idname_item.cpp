#include "std.h"
#include "work_idname_item.h"
#include "idname_item_if.h"

////////////////////////////////////////////////////////////////////////////////
// CIDNameItemWork类
CIDNameItemWork::CIDNameItemWork(const IBundle& bundle)
    : m_context(bundle.getContext()), m_bundle(bundle)
{
}

// 得到服务特征码
const char* CIDNameItemWork::getGUID(void) const
{
    return c_IDNameWorkSrvGUID;
}

// 对应的模块
const IBundle& CIDNameItemWork::getBundle(void) const
{
    return m_bundle;
}

// 添加变量项
TIDNameItermPtr CIDNameItemWork::NewVarItem(IKCActionData& act, EKcDataType dt, string n, const TKcWebExpr& val)
{
    return TVarItemBase::CreateVarItem(*this, act, dt, n, val);
}

// 添加函数组项
TIDNameItermPtr CIDNameItemWork::NewLibItem(IKCActionData& act, string n, const TKcLoadFullFL& ld)
{
    return TFuncLibItemBase::CreateLibItem(*this, act, n, ld);
}
