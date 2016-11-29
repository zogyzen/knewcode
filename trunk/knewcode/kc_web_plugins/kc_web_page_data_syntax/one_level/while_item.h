#pragma once

#include "../syntax_item_if.h"

namespace KC { namespace one_level
{
	class CWhileItem : public CSyntaxItemImpl
	{
	public:
		CWhileItem(ISyntaxItemWork&, IWebPageData&, int, const TKcWhileSent&);
        // 获取语法关键字
        virtual string GetKeychar(void) const;
        // 下一相关语法项
        virtual int GetNextIndex(IKCActionData&);

    public:
        // 语法类型转换
        static const TKcWhileSent& DynCast(const TKcSynBaseClass&, IWebPageData&);

    private:
        TKcWhileSent m_syn;
	};
}}
