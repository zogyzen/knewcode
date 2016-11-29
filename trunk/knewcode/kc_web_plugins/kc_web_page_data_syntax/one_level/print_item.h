#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CPrintItem : public CSyntaxItemImpl
	{
	public:
		CPrintItem(ISyntaxItemWork&, IWebPageData&, int, const TKcPrintSent&);

        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcPrintSent& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcPrintSent m_syn;
	};
}}
