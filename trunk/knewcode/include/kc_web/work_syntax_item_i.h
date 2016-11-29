#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"

namespace KC
{
    // 语法项
    class IKCActionData;
    class ISyntaxItem
    {
    public:
        // 获取语法关键字
        virtual string GetKeychar(void) const = 0;
        // 得到序号
        virtual int GetIndex(void) const = 0;
        // 得到行号
        virtual int GetLineID(void) const = 0;
        // 获取语法起始位置指针
        virtual const char* GetBeginPtr(void) const = 0;
        virtual const char* GetEndPtr(void) const = 0;
        // 获取语法内容
        virtual string GetSyntaxContent(void) const = 0;
        // 解析数据
        virtual void ParseItem(void) = 0;
        // 下一相关语法项序号
        virtual int GetNextIndex(IKCActionData&) = 0;
        // 处理页数据
        virtual void ActionItem(IKCActionData&) = 0;

    public:
        virtual ~ISyntaxItem() {}
    };
    // 语法项列表
    typedef boost::shared_ptr<ISyntaxItem> TSyntaxItemPtr;
    typedef vector<TSyntaxItemPtr> TSyntaxItemList;
    typedef vector<ISyntaxItem*> TSyntaxItemStack;

    // 语法项管理接口
    class IWebPageData;
    class ISyntaxItemWork : public IServiceEx
    {
    public:
        // 添加语法项
        virtual TSyntaxItemPtr NewSyntaxItem(const TKcSynBaseClass&, IWebPageData&, int, TSyntaxItemStack&) = 0;

    protected:
        virtual CALL_TYPE ~ISyntaxItemWork(void) = default;
    };

    constexpr const char c_SyntaxWorkSrvGUID[] = "ISyntaxItemWork_B5FE913D-6D3B-303B-C1D0-2D42AA0BCF29";
    constexpr const char c_SyntaxDelayCPF[] = "CurrentPageFinish";
    constexpr const char c_SyntaxDelayTPF[] = "TotalPageFinish";

    class TSyntaxWorkSrvException : public TFWSrvException
	{
	public:
		TSyntaxWorkSrvException(int id, string place, string msg, string name, const char* pSentB = nullptr, const char* pSentE = nullptr)
				: TFWSrvException(id, place, msg, name, c_SyntaxWorkSrvGUID), m_pSentB(pSentB), m_pSentE(pSentE) {}
		TSyntaxWorkSrvException(int id, string place, string msg, ISyntaxItemWork& srv, const char* pSentB = nullptr, const char* pSentE = nullptr)
				: TFWSrvException(id, place, msg, srv), m_pSentB(pSentB), m_pSentE(pSentE) {}
		virtual string error_info(void) const
		{
		    string sSyn;
		    if (nullptr != m_pSentB) sSyn = "\n" + string(m_pSentB, m_pSentB + 30) + "\n";
		    return TFWSrvException::error_info() + sSyn;
        }

	protected:
		const char* m_pSentB = nullptr;
		const char* m_pSentE = nullptr;
	};
}
