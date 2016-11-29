#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace two_level
{
	class CDelayItem : public CSyntaxItemImpl
	{
	public:
		CDelayItem(ISyntaxItemWork&, IWebPageData&, int, const TKcDelayDef&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcDelayDef& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcDelayDef m_syn;
	};
}}
