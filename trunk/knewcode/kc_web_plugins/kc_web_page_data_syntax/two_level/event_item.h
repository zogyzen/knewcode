#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace two_level
{
	class CEventItem : public CSyntaxItemImpl
	{
	public:
		CEventItem(ISyntaxItemWork&, IWebPageData&, int, const TKcEventDef&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcEventDef& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcEventDef m_syn;
	};
}}
