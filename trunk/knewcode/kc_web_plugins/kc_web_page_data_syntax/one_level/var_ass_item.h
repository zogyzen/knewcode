#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CVarAssItem : public CSyntaxItemImpl
	{
	public:
		CVarAssItem(ISyntaxItemWork&, IWebPageData&, int, const TKcVarAssDef&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;
        // 执行赋值
        static void AssignValue(const TKcVarAssDef&, IKCActionData&);

    public:
        // 语法类型转换
        static const TKcVarAssDef& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcVarAssDef m_syn;
	};
}}
