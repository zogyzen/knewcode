#pragma once

namespace KC
{
    // 页面
    class CWebPageData : public IWebPageData
    {
    public:
        CWebPageData(IWebPageDataFactory&, string, long);
        CWebPageData(IWebPageDataFactory&, string, string, long);
        virtual ~CWebPageData(void);

        // 获取编号
        virtual long GetID(void) const;
        // 添加语法项
        virtual void AddSyntaxItem(const TKcSynBaseClass&);
        // 设置表达式项运算符
        virtual void SetExprOperator(TKcWebExpr&, const TKcSynBaseClass&);
        // 设置表达式项操作数
        virtual void SetExprOperand(TKcWebExpr&, TKcWebExpr&);
        // 得到文件缓冲区
        virtual const TKcWebParseTextBuffer& GetTextBuffer(void);
        // 处理页数据
        virtual bool Action(IKCRequestRespond&);
        virtual bool Action(IKCActionData&, bool);
        // 获取页面数据工厂
        virtual IWebPageDataFactory& GetFactory(void);
        // 添加动态库
        virtual TKLoadLibraryPtr AddLoadLibrary(IKCActionData&, string);
        // 得到语法行
        virtual int GetLineID(const TKcSynBaseClass&);
        // 得到当前行
        virtual int GetCurrLineID(void);
        // 得到当前语法字符串
        virtual string GetCurrSynStr(const char*, const char* = nullptr);
        // 当前位置
        virtual const TSynPosAttr& GetCurrPos(void) const;
        virtual const TSynPosAttr& GetPrevPos(void) const;
        virtual void SetCurrPos(const TSynPosAttr&);
        virtual void SetPrevPos(const TSynPosAttr&);

    public:
        // 最近访问时间
        void SetLastTime(void);
        time_t GetLastTime(void);
        // 解析页面
        void Parse(IKCRequestRespond&);
        // 得到表达式数据接口
        IExprTreeWork& getDataExprInf(void);
        // 获取内部变量的应用
        IInnerVar& getInnerVarInf(void);
        // 获取标识符的应用
        IIDNameItemWork& getIIDNameInf(void);

    private:
        // 读文件
        void ReadPageFile(void);
        // 设置脚本缓冲区
        void SetScriptBuf(void);
        // 文件发生变化重置
        void FileChangedReset(void);
        // 是否解析完成
        bool IsParseDone(void);
        // 得到语法数据接口
        ISyntaxItemWork& getDataSyntaxInf(void);
        // 获取语法项
        ISyntaxItem* GetSyntaxItem(int, IKCActionData&);
        // 处理页数据
        bool ActionBase(IKCActionData&);
        void ActionBaseAct(IKCActionData&);

    private:
        IWebPageDataFactory& m_factory;
        // 文件内容
        TKcWebParseTextBuffer m_Content;
        // 临时脚本
        string m_script;
        // 编号
        long m_id = 0;
        // 是否脚本（临时）
        bool m_ScriptTmp = false;
        // 锁（保证不同时解析）
        CKcMutex m_mtxParse;
        // 最近访问时间
        time_t m_lastTime = std::time(0);
        // 语法项队列
        TSyntaxItemList m_SyntaxItemList;
        // 条件和循环语句栈（用于配对）
        TSyntaxItemStack m_SyntaxItemStack;
        // 当前行
        int m_CurrLineID = -1;
        // 当前位置
        TSynPosAttr m_CurrPos, m_PrevPos;
    };
}
