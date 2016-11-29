#include "std.h"
#include "page_data_fct.h"

////////////////////////////////////////////////////////////////////////////////
// CWebPageDataFactory类
CWebPageDataFactory::CWebPageDataFactory(const IBundle& bundle)
        : m_context(bundle.getContext()), m_bundle(bundle)
        , m_mtxPDs(m_context.LockWork()), m_mtxLib(m_context.LockWork())
{
}

// 得到服务特征码
const char* CWebPageDataFactory::getGUID(void) const
{
    return c_WebPageDataSrvGUID;
}

// 对应的模块
const IBundle& CWebPageDataFactory::getBundle(void) const
{
    return m_bundle;
}

// 解析页面文件
CWebPageData& CWebPageDataFactory::WebPageData(IKCRequestRespond& reKc, const char* fname)
{
    CWebPageData& pd = this->GetWebPageData(reKc, fname);
    pd.Parse(reKc);
    return pd;
}

// 添加页面
IWebPageData& CWebPageDataFactory::WebPageData(IKCRequestRespondData& reKc)
{
    return this->WebPageData(reKc, reKc.GetLocalFilename());
}

IWebPageData& CWebPageDataFactory::WebPageData(IKCActionData& act, const char* fname)
{
    return this->WebPageData(act.GetRequestRespond(), fname);
}

IWebPageData& CWebPageDataFactory::WebPageData(IKCActionData& act, const char* name, const char* script)
{
    CWebPageData& pd = this->GetWebPageData(act.GetRequestRespond(), name, script);
    pd.Parse(act.GetRequestRespond());
    return pd;
}

// 新建页面
CWebPageData& CWebPageDataFactory::GetWebPageData(IKCRequestRespond& reKc, const char* name, const char* script)
{
    CWebPageData* pPD = nullptr;
    if (nullptr == name)
        throw TWebPageDataFactoryException(0, __FUNCTION__, string(this->getHint("Don_t_create_the_page_")) + "NULL Name", *this);
    // 获取或新建页面
    {
        // 锁
        CKcLock lck(m_mtxPDs, string("Web Page Build - ") + name, __FUNCTION__);
        // 信号捕捉函数
        const char* pos = __FUNCTION__;
        auto TryFunc = [&]()
        {
            // 获取已有页面
            {
                TWebPageDataList::iterator iter = m_WebPageDataList.find(name);
                // 如果页面已存在
                if (m_WebPageDataList.end() != iter)
                {
                    pPD = iter->second.get();
                    // 如果文件发生变化，则需重新加载
                    if (nullptr != script || nullptr == pPD)
                    {
                        m_WebPageDataList.erase(iter);
                        pPD = nullptr;
                    }
                }
            }
            if (nullptr == pPD)
            {
                // 创建页面
                if (nullptr == script)
                {
                    this->WriteLogDebug((this->getHint("Create_page_") + string(name) + " [" + reKc.GetUriFilename() + "]").c_str(),
                                        pos, lexical_cast<string>(reinterpret_cast<unsigned int>(&reKc)).c_str());
                    pPD = new CWebPageData(*this, name, ++m_WebPageDataID);
                }
                else
                    pPD = new CWebPageData(*this, name, script, ++m_WebPageDataID);
                TWebPageDataPtr PDPtr(pPD);
                m_WebPageDataList.insert(make_pair(string(name), PDPtr));
            }
            // 设置最近访问时间
            else pPD->SetLastTime();
        };
        // 信号捕捉
        TSignoTry<decltype(TryFunc)> sigTry(TryFunc, reKc, (string("WebPageData(") + name + ")").c_str());
        reKc.SignoCatch().SignoCatchPoint(sigTry);
    }
    // 返回
    if (nullptr == pPD)
        throw TWebPageDataFactoryException(0, __FUNCTION__, string(this->getHint("Don_t_create_the_page_")) + name, *this);
    return *pPD;
}

// 移除页面
bool CWebPageDataFactory::RemoveWebPageData(const char* name)
{
    // 锁
    CKcLock lck(m_mtxPDs, string("Web Page Build - ") + name, __FUNCTION__);
    // 删除页面
    TWebPageDataList::iterator iter = m_WebPageDataList.find(name);
    if (m_WebPageDataList.end() != iter) m_WebPageDataList.erase(iter);
    return true;
}

// 垃圾回收
void CWebPageDataFactory::GC(void)
{
    {
        // 页面锁
        CKcLock lck(m_mtxPDs, "Web Page Build - All", __FUNCTION__);
        // 释放页面
        for (auto it = m_WebPageDataList.begin(); m_WebPageDataList.end() != it; )
        {
            CWebPageData& pd = *it->second.get();
            if (std::difftime(std::time(0), pd.GetLastTime()) > 3600)
            {
                m_WebPageDataList.erase(it);
                it = m_WebPageDataList.begin();
            }
            else ++it;
        }
    }
    {
        // 动态库锁
        CKcLock lck(m_mtxLib, "Load DLL - All", __FUNCTION__);
        // 释放动态库
        for (auto it = m_LibList.begin(); m_LibList.end() != it; )
        {
            KLoadLibrary<>& lib = *it->second.get();
            if (std::difftime(std::time(0), lib.GetLastTime()) > 3600)
            {
                m_LibList.erase(it);
                it = m_LibList.begin();
            }
            else ++it;
        }
    }
}

// 添加动态库
TKLoadLibraryPtr CWebPageDataFactory::AddLoadLibrary(IKCRequestRespond& reKc, string file)
{
    // 锁
    CKcLock lck(m_mtxLib, "Load DLL - " + file, __FUNCTION__);
    // 查找已存在的动态库
    auto iter = m_LibList.find(file);
    if (m_LibList.end() == iter)
    {
        string sErr;
        TKLoadLibraryPtr DllPtr;
        auto TryFunc = [&]()
        {
            KLoadLibrary<> *pDll = new KLoadLibrary<>(file.c_str(), sErr);
            if (!sErr.empty()) throw sErr.c_str();
            if (nullptr == pDll) throw "The_load_library_interface_is_nullptr";
            DllPtr = TKLoadLibraryPtr(pDll);
            m_LibList.insert(make_pair(file, DllPtr));
        };
        // 信号捕捉
        TSignoTry<decltype(TryFunc)> sigTry(TryFunc, reKc, ("AddLoadLibrary(" + file + ")").c_str());
        reKc.SignoCatch().SignoCatchPoint(sigTry);
        return DllPtr;
    }
    else
    {
        iter->second->SetLastTime();
        return iter->second;
    }
}

// 获取解析的应用
IServiceReference& CWebPageDataFactory::getParseRef(void)
{
    if (nullptr == m_ParseRef)
        m_ParseRef = m_context.takeServiceReference(c_ParseWorkSrvGUID);
    return *m_ParseRef;
}

// 获取语法数据的应用
IServiceReference& CWebPageDataFactory::getDataSyntaxRef(void)
{
    if (nullptr == m_DataSyntaxRef)
        m_DataSyntaxRef = m_context.takeServiceReference(c_SyntaxWorkSrvGUID);
    return *m_DataSyntaxRef;
}

// 获取表达式数据的应用
IServiceReference& CWebPageDataFactory::getDataExprRef(void)
{
    if (nullptr == m_DataExprRef)
        m_DataExprRef = m_context.takeServiceReference(c_ExprTreeWorkSrvGUID);
    return *m_DataExprRef;
}

// 获取内部变量的应用
IServiceReference& CWebPageDataFactory::getInnerVarRef(void)
{
    if (nullptr == m_InnerVarRef)
        m_InnerVarRef = m_context.takeServiceReference(c_InnerVarSrvGUID);
    return *m_InnerVarRef;
}

// 获取标识符的应用
IServiceReference& CWebPageDataFactory::getIDNameRef(void)
{
    if (nullptr == m_IDNameRef)
        m_IDNameRef = m_context.takeServiceReference(c_IDNameWorkSrvGUID);
    return *m_IDNameRef;
}
