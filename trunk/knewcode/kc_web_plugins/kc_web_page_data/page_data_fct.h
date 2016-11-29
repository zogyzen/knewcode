#pragma once

#include "page_data.h"

namespace KC
{
    // 页面工厂
    class CWebPageDataFactory : public IWebPageDataFactory
    {
    public:
        CWebPageDataFactory(const IBundle& bundle);
        virtual CALL_TYPE ~CWebPageDataFactory(void) = default;

        // 得到服务特征码
        virtual const char* CALL_TYPE getGUID(void) const;
        // 对应的模块
        virtual const IBundle& CALL_TYPE getBundle(void) const;

        // 添加页面
        virtual IWebPageData& WebPageData(IKCRequestRespondData&);
        virtual IWebPageData& WebPageData(IKCActionData&, const char*);
        virtual IWebPageData& WebPageData(IKCActionData&, const char*, const char*);
        // 移除页面
        virtual bool RemoveWebPageData(const char*);
        // 垃圾回收
        virtual void GC(void);

    public:
        // 添加动态库
        TKLoadLibraryPtr AddLoadLibrary(IKCRequestRespond&, string);

        // 获取解析的应用
        IServiceReference& getParseRef(void);
        // 获取语法数据的应用
        IServiceReference& getDataSyntaxRef(void);
        // 获取表达式数据的应用
        IServiceReference& getDataExprRef(void);
        // 获取内部变量的应用
        IServiceReference& getInnerVarRef(void);
        // 获取标识符的应用
        IServiceReference& getIDNameRef(void);

    private:
        // 解析页面文件
        CWebPageData& WebPageData(IKCRequestRespond&, const char*);
        // 新建页面
        CWebPageData& GetWebPageData(IKCRequestRespond&, const char*, const char* = nullptr);

    private:
        IBundleContext& m_context;
        const IBundle& m_bundle;
        // 解析的应用
        IServiceReference* m_ParseRef = nullptr;
        // 语法数据的应用
        IServiceReference* m_DataSyntaxRef = nullptr;
        // 表达式数据的应用
        IServiceReference* m_DataExprRef = nullptr;
        // 内部变量的应用
        IServiceReference* m_InnerVarRef = nullptr;
        // 标识符的应用
        IServiceReference* m_IDNameRef = nullptr;
        // 页面列表
        typedef boost::shared_ptr<CWebPageData> TWebPageDataPtr;
        typedef map<string, TWebPageDataPtr> TWebPageDataList;
        TWebPageDataList m_WebPageDataList;
        long m_WebPageDataID = 0;
        CKcMutex m_mtxPDs;
        // 动态库列表
        typedef map<string, TKLoadLibraryPtr> TLibList;
        TLibList m_LibList;
        CKcMutex m_mtxLib;
    };
}
