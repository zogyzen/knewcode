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
    // 请求和应答接口
    class IKCRequestRespondData : public IKCRequestRespond
    {
    protected:
        virtual CALL_TYPE ~IKCRequestRespondData() = default;
    };

    class IWebPageData;
    // 执行中的页面数据
    class IKCActionData : public IActionData
    {
    public:
        // 获取表达式的值
        virtual boost::any GetExprValue(const TKcWebExpr&) = 0;
        // 设置内部变量的值（参数依次为：变量名、下标、值）
        virtual void SetInnerVarValue(string, TAnyTypeValList&, boost::any) = 0;
        // 得到完整路径
        virtual string GetFullPath(string) = 0;
        // 获取标识符
        virtual IIDNameItem& GetIDName(string) = 0;
        // 添加标识符
        virtual bool AddIDName(string, TIDNameItermPtr, bool, bool, bool) = 0;
        // 获取接口定义
        virtual TKcInfFullFL& GetKcInfFullFL(string) = 0;
        // 添加接口定义
        virtual bool AddKcInfFullFL(const TKcInfFullFL&) = 0;
        // 得到当前行
        virtual int GetCurrLineID(void) const = 0;
        // 得到详细当前位置
        virtual string GetCurrPosInfo(string = "\r\n") const = 0;
        // 得到当前语法字符串
        virtual string GetCurrSynStr(void) = 0;
        // 非顺序语句的条件栈
        virtual TConditionStatck& ConditionStatck(void) = 0;
        // 得到页面接口
        virtual IWebPageData& GetWebPageData(void) = 0;
        // 延迟语法
        virtual int DelaySynNewIndex(void) const = 0;
        virtual void AddDelaySyn(TSyntaxItemPtr) = 0;

    protected:
        virtual CALL_TYPE ~IKCActionData() = default;
    };

    class IWebPageDataFactory;
    // 页面
    class IWebPageData
    {
    public:
        // 获取编号
        virtual long GetID(void) const = 0;
        // 添加语法项
        virtual void AddSyntaxItem(const TKcSynBaseClass&) = 0;
        // 设置表达式项运算符
        virtual void SetExprOperator(TKcWebExpr&, const TKcSynBaseClass&) = 0;
        // 设置表达式项操作数
        virtual void SetExprOperand(TKcWebExpr&, TKcWebExpr&) = 0;
        // 得到文件缓冲区
        virtual const TKcWebParseTextBuffer& GetTextBuffer(void) = 0;
        // 处理页数据
        virtual bool Action(IKCRequestRespond&) = 0;
        virtual bool Action(IKCActionData&, bool = false) = 0;
        // 获取页面数据工厂
        virtual IWebPageDataFactory& GetFactory(void) = 0;
        // 添加动态库
        virtual TKLoadLibraryPtr AddLoadLibrary(IKCActionData&, string) = 0;
        // 得到语法行
        virtual int GetLineID(const TKcSynBaseClass&) = 0;
        // 得到当前行
        virtual int GetCurrLineID(void) = 0;
        // 得到当前语法字符串
        virtual string GetCurrSynStr(const char*, const char* = nullptr) = 0;
        // 当前位置
        virtual const TSynPosAttr& GetCurrPos(void) const = 0;
        virtual const TSynPosAttr& GetPrevPos(void) const = 0;
        virtual void SetCurrPos(const TSynPosAttr&) = 0;
        virtual void SetPrevPos(const TSynPosAttr&) = 0;

    protected:
        virtual ~IWebPageData(void) = default;
    };

    // 页面工厂
    class IWebPageDataFactory : public IServiceEx
    {
    public:
        // 添加页面
        virtual IWebPageData& WebPageData(IKCRequestRespondData&) = 0;
        virtual IWebPageData& WebPageData(IKCActionData&, const char*) = 0;
        virtual IWebPageData& WebPageData(IKCActionData&, const char*, const char*) = 0;
        // 移除页面
        virtual bool RemoveWebPageData(const char*) = 0;
        // 垃圾回收
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
