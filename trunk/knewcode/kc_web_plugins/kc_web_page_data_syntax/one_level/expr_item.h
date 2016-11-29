#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CExprItem : public CSyntaxItemImpl
	{
	public:
		CExprItem(ISyntaxItemWork&, IWebPageData&, int, const TKcExprWorkDef&);

        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcExprWorkDef& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcExprWorkDef m_syn;
	};
}}
