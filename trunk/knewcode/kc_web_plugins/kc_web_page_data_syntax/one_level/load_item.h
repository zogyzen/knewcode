#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CLoadItem : public CSyntaxItemImpl
	{
	public:
		CLoadItem(ISyntaxItemWork&, IWebPageData&, int, const TKcLoadFullFL&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcLoadFullFL& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcLoadFullFL m_syn;
	};
}}
