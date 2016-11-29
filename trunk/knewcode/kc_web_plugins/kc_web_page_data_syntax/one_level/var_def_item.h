#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CVarDefItem : public CSyntaxItemImpl
	{
	public:
		CVarDefItem(ISyntaxItemWork&, IWebPageData&, int, const TKcVarDef&);

        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcVarDef& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcVarDef m_syn;
	};
}}
