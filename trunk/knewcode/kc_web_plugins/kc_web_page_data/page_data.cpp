#include "std.h"
#include "page_data.h"
#include "action_data.h"
#include "page_data_fct.h"

////////////////////////////////////////////////////////////////////////////////
// CWebPageData类
CWebPageData::CWebPageData(IWebPageDataFactory& fac, string fname, long id)
        : m_factory(fac), m_Content(fname), m_id(id)
        , m_mtxParse(m_factory.getContext().LockWork())
{
}

CWebPageData::CWebPageData(IWebPageDataFactory& fac, string name, string script, long id)
        : m_factory(fac), m_Content(name), m_script(script), m_id(id), m_ScriptTmp(true)
        , m_mtxParse(m_factory.getContext().LockWork())
{
}

CWebPageData::~CWebPageData(void)
{
}

// 获取编号
long CWebPageData::GetID(void) const
{
    return m_id;
}

// 添加语法项
void CWebPageData::AddSyntaxItem(const TKcSynBaseClass& syn)
{
    // 创建并添加语法项
    m_SyntaxItemList.push_back(this->getDataSyntaxInf().NewSyntaxItem(syn, *this, m_SyntaxItemList.size(), m_SyntaxItemStack));
}

// 设置表达式项运算符
void CWebPageData::SetExprOperator(TKcWebExpr& expr, const TKcSynBaseClass& syn)
{
    this->getDataExprInf().SetExprOperator(*this, expr, syn);
}

// 设置表达式项操作数
void CWebPageData::SetExprOperand(TKcWebExpr& opr, TKcWebExpr& exp)
{
    this->getDataExprInf().SetExprOperand(*this, opr, exp);
}

// 得到文件缓冲区
const TKcWebParseTextBuffer& CWebPageData::GetTextBuffer(void)
{
    if (nullptr == m_Content.step)
    {
        if (m_ScriptTmp) this->SetScriptBuf();
        else this->ReadPageFile();
    }
    return m_Content;
}

// 处理页数据
bool CWebPageData::ActionBase(IKCActionData& act)
{
    bool bResult = false;
    try
    {
        // todo: 需设置超时时间
        this->ActionBaseAct(act);
        bResult = true;
    }
    catch(TException& ex)
    {
        ex.CurrPosInfo() = dynamic_cast<CActionData&>(act).GetCurrPosInfo() + ex.CurrPosInfo();
        ex.ExceptType() = typeid(ex).name();
        throw ex;
    }
    catch(std::exception& e)
    {
        TException ex(act.GetCurrLineID(), __FUNCTION__, e.what());
        ex.CurrPosInfo() = dynamic_cast<CActionData&>(act).GetCurrPosInfo();
        ex.ExceptType() = typeid(e).name();
        throw ex;
    }
	catch(const char* e)
	{
        TException ex(act.GetCurrLineID(), __FUNCTION__, e);
        ex.CurrPosInfo() = dynamic_cast<CActionData&>(act).GetCurrPosInfo();
        ex.ExceptType() = "const char*";
        throw ex;
	}
	catch(int e)
	{
        TException ex(act.GetCurrLineID(), __FUNCTION__, lexical_cast<string>(e));
        ex.CurrPosInfo() = dynamic_cast<CActionData&>(act).GetCurrPosInfo();
        ex.ExceptType() = "int";
        throw ex;
	}
    catch(...)
    {
        TException ex(act.GetCurrLineID(), __FUNCTION__, m_factory.getHint("Unknown_exception"));
        ex.CurrPosInfo() = dynamic_cast<CActionData&>(act).GetCurrPosInfo();
        ex.ExceptType() = "Unknown_exception";
        throw ex;
    }
    return bResult;
}
void CWebPageData::ActionBaseAct(IKCActionData& act)
{
    // 处理页面
    const char* pos = __FUNCTION__;
    auto TryFunc = [&]()
    {
        m_factory.WriteLogDebug((m_factory.getHint("Action_page_") + m_Content.fileName + " [" + act.GetRequestRespond().GetUriFilename() + "]").c_str(),
                                pos, lexical_cast<string>(reinterpret_cast<unsigned int>(&act)).c_str());
        // 按顺序循环处理每个语法项
        for (ISyntaxItem* syn = this->GetSyntaxItem(0, act); nullptr != syn; syn = this->GetSyntaxItem(syn->GetNextIndex(act), act))
        {
            boost::format fmtDebugInfo = boost::format(m_factory.getHint("Action_syntax_item")) % syn->GetKeychar() % syn->GetLineID() % syn->GetIndex();
            m_factory.WriteLogTrace(fmtDebugInfo.str().c_str(), pos, syn->GetSyntaxContent().c_str());
            syn->ActionItem(act);
        }
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, act.GetRequestRespond(), ("ActionItem(" + m_Content.fileName + ")").c_str());
    act.GetRequestRespond().SignoCatch().SignoCatchPoint(sigTry);
}

bool CWebPageData::Action(IKCRequestRespond& re)
{
    CActionData act(re, *this);
    CAutoRelease _auto([&](){ re.SetActionData(&act); }, [&](){ re.SetActionData(); });
    CAutoRelease _autoDelay([&](){ act.ActionForDelay(); });
    return this->ActionBase(act);
}

bool CWebPageData::Action(IKCActionData& own, bool stt)
{
    for (IKCActionData* ia = dynamic_cast<IKCActionData*>(own.GetActOwn()); nullptr != ia; ia = dynamic_cast<IKCActionData*>(ia->GetActOwn()))
        if (this->GetID() == ia->GetWebPageData().GetID())
            throw TWebPageDataFactoryException(ia->GetCurrLineID(), __FUNCTION__, (boost::format(m_factory.getHint("Circular_reference")) % m_Content.fileName % ia->GetCurrLineID()).str(), m_factory);
    CActionData act(own, stt, *this);
    CAutoRelease _autoDelay([&](){ act.ActionForDelay(); });
    return this->ActionBase(act);
}

// 获取页面数据工厂
IWebPageDataFactory& CWebPageData::GetFactory(void)
{
    return m_factory;
}

// 添加动态库
TKLoadLibraryPtr CWebPageData::AddLoadLibrary(IKCActionData& act, string file)
{
    string sErr = "Load_library_failure";
    try
    {
        CWebPageDataFactory& fct = dynamic_cast<CWebPageDataFactory&>(m_factory);
        TKLoadLibraryPtr result = fct.AddLoadLibrary(act.GetRequestRespond(), file);
        if (nullptr == result.get())
            throw "The_load_library_interface_is_nullptr";
        return result;
    }
    catch (const char* ex)
    {
        sErr = ex;
    }
    catch (...) {}
    throw TWebPageDataFactoryException(this->GetCurrLineID(), __FUNCTION__, (boost::format(m_factory.getHint(sErr.c_str())) % file % this->GetCurrLineID()).str(), m_factory);
}

// 解析页面
void CWebPageData::Parse(IKCRequestRespond& reKc)
{
    // 锁
    CKcLock lck(m_mtxParse, "Web Page Parse - " + m_Content.fileName, __FUNCTION__);
    // 解析页面
    auto TryFunc = [&]()
    {
        // 文件发生变化重置
        this->FileChangedReset();
        // 判断是否已完成
        if (!this->IsParseDone())
            // 开始解析
            dynamic_cast<IServiceReferenceEx&>(dynamic_cast<CWebPageDataFactory&>(m_factory).getParseRef()).getServiceSafe<IParseWork>().Parse(*this);
    };
    // 信号捕捉
    TSignoTry<decltype(TryFunc)> sigTry(TryFunc, reKc, ("Parse(" + m_Content.fileName + ")").c_str());
    reKc.SignoCatch().SignoCatchPoint(sigTry);
}

// 判断文件是否发生变化
void CWebPageData::FileChangedReset(void)
{
    if (m_ScriptTmp) return;
    if (!filesystem::exists(m_Content.fileName))
        throw TWebPageDataFactoryException(0, __FUNCTION__, m_factory.getHint("Don_t_exists_file_") + m_Content.fileName, m_factory);
    // 文件属性
    filesystem::path fPath(m_Content.fileName);
    time_t fMtime = filesystem::last_write_time(fPath);
    int fSize = filesystem::file_size(fPath);
    // 判断文件是否发生变化
    if (m_Content.bufSize != fSize || m_Content.fileMtime != fMtime)
    {
        m_Content.bufSize = fSize;
        m_Content.fileMtime = fMtime;
        m_Content.first = m_Content.last = m_Content.step = nullptr;
        m_SyntaxItemList.clear();
        m_SyntaxItemStack.clear();
        m_CurrLineID = -1;
        this->ReadPageFile();
    }
}

// 是否解析完成
bool CWebPageData::IsParseDone(void)
{
    if (nullptr == m_Content.step)
    {
        if (m_ScriptTmp) this->SetScriptBuf();
        else this->ReadPageFile();
    }
    return m_Content.last == m_Content.step && m_Content.first != m_Content.last;
}

// 读文件
void CWebPageData::ReadPageFile(void)
{
    m_factory.WriteLogDebug((m_factory.getHint("Read_Page_File_") + m_Content.fileName).c_str(), __FUNCTION__,
                             lexical_cast<string>(reinterpret_cast<unsigned int>(this)).c_str());
    if (!filesystem::exists(m_Content.fileName))
        throw TWebPageDataFactoryException(0, __FUNCTION__, m_factory.getHint("Don_t_exists_file_") + m_Content.fileName, m_factory);
    // 文件属性
    filesystem::path fPath(m_Content.fileName);
    m_Content.bufSize = filesystem::file_size(fPath);
    m_Content.fileMtime = filesystem::last_write_time(fPath);
    // 打开文件
    ifstream kcFile;
    kcFile.open(m_Content.fileName.c_str());
    CAutoRelease _auto(boost::bind(&ifstream::close, &kcFile));
    // 读取文件内容
    stringstream bufStream;
    bufStream << kcFile.rdbuf();
    m_script = bufStream.str();
    if (m_script.size() == 0 && m_Content.bufSize > 0)
        throw TWebPageDataFactoryException(0, __FUNCTION__, m_factory.getHint("Read_file_failure_") + m_Content.fileName, m_factory);
    // 设置缓冲区
    this->SetScriptBuf();
}

// 设置脚本缓冲区
void CWebPageData::SetScriptBuf(void)
{
    // 创建内容缓存区
    int bufSize = m_script.size();
    char* bufContent = new char[bufSize + 1];
    m_Content.buffer = TKcWebParseTextBuffer::CContentBuffer(bufContent);
    memset(bufContent, 0, bufSize + 1);
    // 设置内容缓存区
    m_Content.first = m_Content.step = bufContent;
    m_Content.last = bufContent + bufSize;
    m_CurrPos = m_PrevPos = TSynPosAttr(bufContent, bufContent);
    memcpy(bufContent, m_script.c_str(), m_script.size());
    m_script = "";
}

// 最近访问时间
void CWebPageData::SetLastTime(void)
{
    m_lastTime = std::time(0);
}
time_t CWebPageData::GetLastTime(void)
{
    return m_lastTime;
}

// 得到语法数据接口
ISyntaxItemWork& CWebPageData::getDataSyntaxInf(void)
{
    CWebPageDataFactory& fct = dynamic_cast<CWebPageDataFactory&>(m_factory);
    return dynamic_cast<IServiceReferenceEx&>(fct.getDataSyntaxRef()).getServiceSafe<ISyntaxItemWork>();
}

// 获取语法项
ISyntaxItem* CWebPageData::GetSyntaxItem(int i, IKCActionData& act)
{
    ISyntaxItem* result = nullptr;
    if ((int)m_SyntaxItemList.size() > i && i >= 0) result = m_SyntaxItemList[i].get();
    dynamic_cast<CActionData&>(act).SetCurrSynItm(result);
    return result;
}

// 得到表达式数据接口
IExprTreeWork& CWebPageData::getDataExprInf(void)
{
    CWebPageDataFactory& fct = dynamic_cast<CWebPageDataFactory&>(m_factory);
    return dynamic_cast<IServiceReferenceEx&>(fct.getDataExprRef()).getServiceSafe<IExprTreeWork>();
}

// 得到内部变量接口
IInnerVar& CWebPageData::getInnerVarInf(void)
{
    CWebPageDataFactory& fct = dynamic_cast<CWebPageDataFactory&>(m_factory);
    return dynamic_cast<IServiceReferenceEx&>(fct.getInnerVarRef()).getServiceSafe<IInnerVar>();
}

// 获取标识符的应用
IIDNameItemWork& CWebPageData::getIIDNameInf(void)
{
    CWebPageDataFactory& fct = dynamic_cast<CWebPageDataFactory&>(m_factory);
    return dynamic_cast<IServiceReferenceEx&>(fct.getIDNameRef()).getServiceSafe<IIDNameItemWork>();
}

// 得到语法行
int CWebPageData::GetLineID(const TKcSynBaseClass& syn)
{
    const TKcWebParseTextBuffer& buf = this->GetTextBuffer();
    const char* pBeginPtr = syn.GetBeginPtr();
    if (nullptr != pBeginPtr)
        m_CurrLineID = std::count((const char*)buf.first, pBeginPtr, '\n') + 1;
    return m_CurrLineID;
}

// 得到当前行
int CWebPageData::GetCurrLineID(void)
{
    return m_CurrLineID;
}

// 得到当前语法字符串
string CWebPageData::GetCurrSynStr(const char* BeginPtr, const char* EndPtr)
{
    string sResult = "";
    if (nullptr != BeginPtr && nullptr != EndPtr)
        sResult = string(BeginPtr, EndPtr);
    else if (nullptr != BeginPtr)
        sResult = string(BeginPtr, BeginPtr + std::min(m_Content.last - BeginPtr, 50));
    return sResult;
}

// 当前位置
const TSynPosAttr& CWebPageData::GetCurrPos(void) const
{
    return m_CurrPos;
}

const TSynPosAttr& CWebPageData::GetPrevPos(void) const
{
    return m_PrevPos;
}

void CWebPageData::SetCurrPos(const TSynPosAttr& pos)
{
    m_CurrPos = pos;
}

void CWebPageData::SetPrevPos(const TSynPosAttr& pos)
{
    m_PrevPos = pos;
}
