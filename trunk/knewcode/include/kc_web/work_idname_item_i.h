#pragma once

#include "framework_ex/service_ex_i.h"
#include "framework_ex/framework_exception.h"
#include "kc_web/dyn_call_i.h"

namespace KC
{
    class IKCActionData;

    // 标识符项
    class IIDNameItem
    {
    public:
        // 获取页面活动数据
        virtual IKCActionData& GetAction(void) = 0;

    public:
        virtual ~IIDNameItem() = default;
    };
    typedef boost::shared_ptr<IIDNameItem> TIDNameItermPtr;

    // 变量项
    class IVarItemBase : public IIDNameItem
    {
    public:
        // 获取变量的值
        virtual boost::any GetVarValue(void) = 0;
        // 变量赋值
        virtual void SetVarValue(boost::any) = 0;

    public:
        virtual ~IVarItemBase() = default;
    };

    // 函数组项
    class IFuncLibItemBase : public IIDNameItem
    {
    public:
        // 获取函数定义
        virtual TKcFuncDefFL& GetFuncDef(string, IFuncCallDyn&) = 0;

    public:
        virtual ~IFuncLibItemBase() = default;
    };

    // 标识符项管理接口
    class IIDNameItemWork : public IServiceEx
    {
    public:
        // 添加变量项
        virtual TIDNameItermPtr NewVarItem(IKCActionData&, EKcDataType, string, const TKcWebExpr&) = 0;
        // 添加函数组项
        virtual TIDNameItermPtr NewLibItem(IKCActionData&, string, const TKcLoadFullFL&) = 0;

    protected:
        virtual CALL_TYPE ~IIDNameItemWork(void) = default;
    };

    constexpr const char c_IDNameWorkSrvGUID[] = "IIDNameItemWork_3E1B1BD2-2144-7E85-240C-1FD39E00E6DE";

    class TIDNameWorkSrvException : public TFWSrvException
	{
	public:
		TIDNameWorkSrvException(int id, string place, string msg, string name, string IDName = "")
				: TFWSrvException(id, place, msg, name, c_IDNameWorkSrvGUID), m_IDName(IDName) {}
		TIDNameWorkSrvException(int id, string place, string msg, IIDNameItemWork& srv, string IDName = "")
				: TFWSrvException(id, place, msg, srv), m_IDName(IDName) {}
		virtual string error_info(void) const
		{
		    return TFWSrvException::error_info() + "\n" + m_IDName;
        }

	protected:
		string m_IDName = "";
	};
}
