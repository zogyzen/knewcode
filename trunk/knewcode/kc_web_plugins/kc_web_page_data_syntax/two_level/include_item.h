#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace two_level
{
	class CIncludeItem : public CSyntaxItemImpl
	{
	public:
		CIncludeItem(ISyntaxItemWork&, IWebPageData&, int, const TKcIncludeCL&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcIncludeCL& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcIncludeCL m_syn;
	};
}}
