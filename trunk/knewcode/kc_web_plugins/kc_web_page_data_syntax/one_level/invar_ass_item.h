#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CInnerVarAssItem : public CSyntaxItemImpl
	{
	public:
		CInnerVarAssItem(ISyntaxItemWork&, IWebPageData&, int, const TKcInnerVarAssDef&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;
        // 执行赋值
        static void AssignValue(TKcInnerVarAssDef&, IKCActionData&);

    public:
        // 语法类型转换
        static const TKcInnerVarAssDef& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcInnerVarAssDef m_syn;
	};
}}
