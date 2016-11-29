#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CExecItem : public CSyntaxItemImpl
	{
	public:
		CExecItem(ISyntaxItemWork&, IWebPageData&, int, const TKcExecSent&);

        // 获取语法关键字
        virtual string GetKeychar(void) const;

    public:
        // 语法类型转换
        static const TKcExecSent& DynCast(const TKcSynBaseClass&, IWebPageData&);

    protected:
        // 处理页数据
        virtual void Action(IKCActionData&);

    private:
        TKcExecSent m_syn;
	};
}}
