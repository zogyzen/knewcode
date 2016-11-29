#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace two_level
{
	class CInterfaceItem : public CSyntaxItemImpl
	{
	public:
		CInterfaceItem(ISyntaxItemWork&, IWebPageData&, int, const TKcInfFullFL&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcInfFullFL& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcInfFullFL m_syn;
	};
}}
