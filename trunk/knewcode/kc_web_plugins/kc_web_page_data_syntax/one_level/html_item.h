#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CHtmlItem : public CSyntaxItemImpl
	{
	public:
		CHtmlItem(ISyntaxItemWork&, IWebPageData&, int, const TKcHtmlSyn&);

        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcHtmlSyn& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcHtmlSyn m_syn;
	};
}}
