#pragma once

namespace KC
{
    // 执行的数据
    class CActionData : public IKCActionData
    {
    public:
        CActionData(IKCRequestRespond&, IWebPageData&);
        CActionData(IKCActionData&, bool, IWebPageData&);
        virtual CALL_TYPE ~CActionData();

        // 得到页面请求和应答
        virtual IKCRequestRespond& CALL_TYPE GetRequestRespond(void);
        // 得到归属
        virtual IActionData* CALL_TYPE GetActOwn(void);
        virtual IActionData& CALL_TYPE GetActRoot(void);
        virtual IActionData& CALL_TYPE GetActNow(void);
        // 写日志
        virtual bool CALL_TYPE WriteLogTrace(const char*, const char*, const char* = "") const;
        virtual bool CALL_TYPE WriteLogDebug(const char*, const char*, const char* = "") const;
        virtual bool CALL_TYPE WriteLogInfo(const char*, const char*, const char* = "") const;
        virtual bool CALL_TYPE WriteLogError(const char*, const char*, const char* = "") const;
        // 解析并执行字符串脚本
        virtual bool CALL_TYPE ParseActionScript(const char*, const char*);
        // 创建、释放简单数据或接口
        virtual IKcWebDataVal& CALL_TYPE CreateDataVal(void);
        virtual bool CALL_TYPE ReleaseDataVal(IKcWebDataVal&);
        virtual IKcWebInfVal& CALL_TYPE CreateInfVal(void);
        virtual bool CALL_TYPE ReleaseInfVal(IKcWebInfVal&);
        // 标识符、内部变量是否存在
        virtual bool CALL_TYPE ExistsVar(const char*) const;
        virtual bool CALL_TYPE ExistsInnerVar(const char*, const char*) const;
        // 接口变量
        virtual IKcWebInfVal& CALL_TYPE GetInterfaceVar(const char*);
        virtual bool CALL_TYPE SetInterfaceVar(const char*, IKcWebInfVal&);
        // 简单类型变量值
        virtual IKcWebDataVal& CALL_TYPE GetVarValue(const char*);
        virtual bool CALL_TYPE SetVarValue(const char*, IKcWebDataVal&);
        // 内部变量值
        virtual IKcWebInfVal& CALL_TYPE GetInnerVarInf(const char*, const char*);
        virtual IKcWebDataVal& CALL_TYPE GetInnerVarVal(const char*, const char*);
        virtual bool CALL_TYPE SetInnerVar(const char*, const char*, IKcWebInfVal&);
        virtual bool CALL_TYPE SetInnerVar(const char*, const char*, IKcWebDataVal&);
        // 垃圾回收
        virtual void CALL_TYPE GC(void);
        // 获取本地化提示信息
        virtual const char* CALL_TYPE getHint(const char* key) const;
        // 抛一个字符串异常
        virtual void CALL_TYPE ThrowString(const char* ex) const;

        // 获取表达式的值
        virtual boost::any GetExprValue(const TKcWebExpr&);
        // 设置内部变量的值（参数依次为：变量名、下标、值）
        virtual void SetInnerVarValue(string, TAnyTypeValList&, boost::any);
        // 得到完整路径
        virtual string GetFullPath(string);
        // 获取标识符
        virtual IIDNameItem& GetIDName(string);
        // 添加标识符
        virtual bool AddIDName(string, TIDNameItermPtr, bool, bool, bool);
        // 获取接口定义
        virtual TKcInfFullFL& GetKcInfFullFL(string);
        // 添加接口定义
        virtual bool AddKcInfFullFL(const TKcInfFullFL&);
        // 得到当前行
        virtual int GetCurrLineID(void) const;
        // 得到详细当前位置
        virtual string GetCurrPosInfo(string = "\r\n") const;
        // 得到当前语法字符串
        virtual string GetCurrSynStr(void);
        // 非顺序语句的条件栈
        virtual TConditionStatck& ConditionStatck(void);
        // 得到页面接口
        virtual IWebPageData& GetWebPageData(void);
        // 延迟语法
        virtual int DelaySynNewIndex(void) const;
        virtual void AddDelaySyn(TSyntaxItemPtr);

    public:
        // 处理延迟语法列表
        void ActionForDelay(void);
        // 设置当前语法项
        void SetCurrSynItm(ISyntaxItem* CurrSynItm);

    private:
        // 处理延迟语法列表
        void ActionForDelayAct(void);

    protected:
        // 归属
        IKCActionData *m_own = nullptr, *m_now = nullptr;
        // 是否私有（标识符、接口等不向上层定义）
        bool m_private = false;
        // 请求应答接口
        IKCRequestRespond& m_re;
        // 页数据接口
        IWebPageData& m_pd;
        // 当前语法项
        ISyntaxItem* m_CurrSynItm = nullptr;
        // 标识符列表
        typedef map<string, TIDNameItermPtr> TIDNameItemList;
        TIDNameItemList m_IDNameItemList;
        // 静态标识符列表
        typedef set<string> TIDNameStaticList;
        TIDNameStaticList m_IDNameStaticList;
        // 接口列表
        typedef map<string, TKcInfFullFL> TInfFullFLList;
        TInfFullFLList m_InfFullFLList;
        // 非顺序语句的条件栈
        TConditionStatck m_ConditionStatck;
        // 延迟处理语法项队列
        TSyntaxItemList m_SyntaxItemListForDelay;

    private:
        // 临时简单类型数据
        typedef boost::shared_ptr<TKcWebDataVal> TKcWebDataValPtr;
        typedef map<void*, TKcWebDataValPtr> TKcWebDataValList;
        TKcWebDataValList m_KcWebDataValList;
        CKcMutex m_mtxTmpSimData;        // 临时接口数据
        typedef boost::shared_ptr<TKcWebInfVal> TKcWebInfValPtr;
        typedef map<void*, TKcWebInfValPtr> TKcWebInfValList;
        TKcWebInfValList m_KcWebInfValList;
        CKcMutex m_mtxTmpInfData;    };
}
