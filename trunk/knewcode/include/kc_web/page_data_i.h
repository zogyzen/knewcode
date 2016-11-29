#pragma once

#include <csetjmp>

#include "for_user/page_interface.h"
#include "util/load_library.h"
#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/web_struct.h"
#include "kc_web/syntax_struct_two.h"
#include "kc_web/work_syntax_item_i.h"
#include "kc_web/work_idname_item_i.h"

namespace KC
{
    // �����Ӧ��ӿ�
    class IKCRequestRespondData : public IKCRequestRespond
    {
    protected:
        virtual CALL_TYPE ~IKCRequestRespondData() = default;
    };

    class IWebPageData;
    // ִ���е�ҳ������
    class IKCActionData : public IActionData
    {
    public:
        // ��ȡ���ʽ��ֵ
        virtual boost::any GetExprValue(const TKcWebExpr&) = 0;
        // �����ڲ�������ֵ����������Ϊ�����������±ꡢֵ��
        virtual void SetInnerVarValue(string, TAnyTypeValList&, boost::any) = 0;
        // �õ�����·��
        virtual string GetFullPath(string) = 0;
        // ��ȡ��ʶ��
        virtual IIDNameItem& GetIDName(string) = 0;
        // ��ӱ�ʶ��
        virtual bool AddIDName(string, TIDNameItermPtr, bool, bool, bool) = 0;
        // ��ȡ�ӿڶ���
        virtual TKcInfFullFL& GetKcInfFullFL(string) = 0;
        // ��ӽӿڶ���
        virtual bool AddKcInfFullFL(const TKcInfFullFL&) = 0;
        // �õ���ǰ��
        virtual int GetCurrLineID(void) const = 0;
        // �õ���ϸ��ǰλ��
        virtual string GetCurrPosInfo(string = "\r\n") const = 0;
        // �õ���ǰ�﷨�ַ���
        virtual string GetCurrSynStr(void) = 0;
        // ��˳����������ջ
        virtual TConditionStatck& ConditionStatck(void) = 0;
        // �õ�ҳ��ӿ�
        virtual IWebPageData& GetWebPageData(void) = 0;
        // �ӳ��﷨
        virtual int DelaySynNewIndex(void) const = 0;
        virtual void AddDelaySyn(TSyntaxItemPtr) = 0;

    protected:
        virtual CALL_TYPE ~IKCActionData() = default;
    };

    class IWebPageDataFactory;
    // ҳ��
    class IWebPageData
    {
    public:
        // ��ȡ���
        virtual long GetID(void) const = 0;
        // ����﷨��
        virtual void AddSyntaxItem(const TKcSynBaseClass&) = 0;
        // ���ñ��ʽ�������
        virtual void SetExprOperator(TKcWebExpr&, const TKcSynBaseClass&) = 0;
        // ���ñ��ʽ�������
        virtual void SetExprOperand(TKcWebExpr&, TKcWebExpr&) = 0;
        // �õ��ļ�������
        virtual const TKcWebParseTextBuffer& GetTextBuffer(void) = 0;
        // ����ҳ����
        virtual bool Action(IKCRequestRespond&) = 0;
        virtual bool Action(IKCActionData&, bool = false) = 0;
        // ��ȡҳ�����ݹ���
        virtual IWebPageDataFactory& GetFactory(void) = 0;
        // ��Ӷ�̬��
        virtual TKLoadLibraryPtr AddLoadLibrary(IKCActionData&, string) = 0;
        // �õ��﷨��
        virtual int GetLineID(const TKcSynBaseClass&) = 0;
        // �õ���ǰ��
        virtual int GetCurrLineID(void) = 0;
        // �õ���ǰ�﷨�ַ���
        virtual string GetCurrSynStr(const char*, const char* = nullptr) = 0;
        // ��ǰλ��
        virtual const TSynPosAttr& GetCurrPos(void) const = 0;
        virtual const TSynPosAttr& GetPrevPos(void) const = 0;
        virtual void SetCurrPos(const TSynPosAttr&) = 0;
        virtual void SetPrevPos(const TSynPosAttr&) = 0;

    protected:
        virtual ~IWebPageData(void) = default;
    };

    // ҳ�湤��
    class IWebPageDataFactory : public IServiceEx
    {
    public:
        // ���ҳ��
        virtual IWebPageData& WebPageData(IKCRequestRespondData&) = 0;
        virtual IWebPageData& WebPageData(IKCActionData&, const char*) = 0;
        virtual IWebPageData& WebPageData(IKCActionData&, const char*, const char*) = 0;
        // �Ƴ�ҳ��
        virtual bool RemoveWebPageData(const char*) = 0;
        // ��������
        virtual void GC(void) = 0;

    protected:
        virtual CALL_TYPE ~IWebPageDataFactory(void) = default;
    };

    constexpr const char c_WebPageDataSrvGUID[] = "IWebPageDataFactory_0E9C4A2F-7043-3312-771F-C9FB8DB146C3";

    class TWebPageDataFactoryException : public TFWSrvException
	{
	public:
		TWebPageDataFactoryException(int id, string place, string msg, string name)
				: TFWSrvException(id, place, msg, name, c_WebPageDataSrvGUID) {}
		TWebPageDataFactoryException(int id, string place, string msg, IWebPageDataFactory& srv)
				: TFWSrvException(id, place, msg, srv) {}
	};
}
