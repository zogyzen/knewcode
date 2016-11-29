#include "std.h"
#include "kc_signo_catch.h"
#include "bundle_context.h"

////////////////////////////////////////////////////////////////////////////////
// 全局上下文
extern CBundleContext *g_context;

// 异常重入标志
thread_local static bool g_SignalFuncReEntry = false;

// 需要注册的信号值
constexpr int c_SignalVals[] = {4, 7, 8, 10, 11, 12, 13, 16, 24, 25, 31};

// 全局线程信号捕捉变量
class TSignoCatchThreadVar
{
public:
    ~TSignoCatchThreadVar()
    {
        RemoveSignoCatch();
    };

    unsigned int GetSigCatchThrdVarID(void)
    {
        return reinterpret_cast<unsigned int>(m_catch);
    }

    CSignoCatch& GetSignoCatch(void)
    {
        if (nullptr == m_catch)
        {
            m_catch = new CSignoCatch;
            if (nullptr != g_context && g_context->GetCfgLogLevel() <= (int)log::trivial::debug)
                g_context->WriteLogDebug(("Begin Jmp Point[" + lexical_cast<string>(GetSigCatchThrdVarID()) + "]").c_str(), __FUNCTION__);
        }
        return *m_catch;
    }

    void RemoveSignoCatch(void)
    {
        if (nullptr == m_catch)
        {
            if (nullptr != g_context && g_context->GetCfgLogLevel() <= (int)log::trivial::debug)
                g_context->WriteLogDebug(("End Jmp Point[" + lexical_cast<string>(GetSigCatchThrdVarID()) + "]").c_str(), __FUNCTION__);
            delete m_catch;
            m_catch = nullptr;
        }
    }

private:
    CSignoCatch* m_catch = nullptr;
};
thread_local static TSignoCatchThreadVar g_SigCatchThrdVar;
CSignoCatch& GetSignoCatch(void)
{
    return g_SigCatchThrdVar.GetSignoCatch();
}

////////////////////////////////////////////////////////////////////////////////
// CSignoCatch类
CSignoCatch::CSignoCatch(void) : m_threadID(pthread_self())
{
    CSignoCatch::Signal();
}
CSignoCatch::~CSignoCatch(void)
{
    if (!m_JmpVec.empty()) m_JmpVec.clear();
}

// 注册信号处理函数
void CSignoCatch::Signal(void)
{
    for(int i: c_SignalVals) CRegisterSignal::Signal(i, CSignoCatch::SignalFunc);
}

// 异常信号处理函数
void CSignoCatch::SignalFunc(int signo)
{
    // 重新注册信号
    CRegisterSignal::ReSignal(signo, CSignoCatch::SignalFunc);
    // 线程递归进入，程序退出
    if (g_SignalFuncReEntry)
    {
        CTempLog::Write("SignalFunc Reentry, Process Exit. Signo[" + lexical_cast<string>(signo) + "]", __FUNCTION__);
        exit(pthread_self());
    }
    // 记录线程进入，避免递归进入
    else g_SignalFuncReEntry = true;
    try
    {
        if (nullptr != g_context)
        {
            // 开始日志
            g_context->WriteLogFatal(("SignalFunc Begin. Signo[" + lexical_cast<string>(signo) + "]").c_str(), __FUNCTION__);
            // 段间跳转
            GetSignoCatch().SignalLongjmp(signo);
            // 跳转失败日志
            g_context->WriteLogFatal(("SignalFunc Fail, not longjmp, Process Exit. Signo[" + lexical_cast<string>(signo) + "]").c_str(), __FUNCTION__);
        }
        else
            CTempLog::Write("SignalFunc Fail, NULL Context, Process Exit. Signo[" + lexical_cast<string>(signo) + "]", __FUNCTION__);
    }
    catch (...) {}
    // 进程退出
    //pthread_exit(nullptr);
    exit(pthread_self());
}
void CSignoCatch::SignalLongjmp(int signo)
{
    try
    {
        // 日志
        if (this->ExistsJmpPoint())
        {
            TJmpBuf& jp = this->TopJmpPoint();
            this->m_JmpPointName = jp.name;
            this->m_SignoCurrPosInfo = jp.it.Detail(this->m_SignoCurrLine);
        }
        else
        {
            this->m_JmpPointName = "NoJmpPoint";
            this->m_SignoCurrPosInfo = "@";
        }
        this->m_BackTrace = CBacktraceSymbols::Get();
        string sInfo = "SignalFunc End. longjmp [" + this->m_JmpPointName + "] Signo[" + lexical_cast<string>(signo) + "]" + this->m_BackTrace + this->m_SignoCurrPosInfo;
        if (nullptr != g_context) g_context->WriteLogFatal(sInfo.c_str(), __FUNCTION__, this->m_JmpPointName.c_str());
    }
    catch (...) {}
    try
    {
        if (this->ExistsJmpPoint())
        {
            // 跳转到上一个注册点
            g_SignalFuncReEntry = false;
            std::longjmp(this->TopJmpPoint().env, signo);
        }
    }
    catch (...) {}
}

// 压入和弹出栈跳转点
void CSignoCatch::PushJmpPoint(string n, jmp_buf& env, ISignoTry& it)
{
    m_JmpVec.push_back(TJmpBuf(n, env, it));
}
void CSignoCatch::PopJmpPoint(void)
{
    if (!m_JmpVec.empty()) m_JmpVec.pop_back();
}
bool CSignoCatch::ExistsJmpPoint(void)
{
    return !m_JmpVec.empty();
}

// 获取栈顶跳转点
CSignoCatch::TJmpBuf& CSignoCatch::TopJmpPoint(void)
{
    if (m_JmpVec.empty())
        throw TFWException(0, __FUNCTION__, "Array index out of bounds");
    return m_JmpVec.back();
}

// 信号结果处理
void CSignoCatch::SignoResult(int sig, string n)
{
    if (0 != sig)
    {
        string sErrInfo = "";
        if (SIGFPE == sig)
            sErrInfo = "SIGFPE Signo";
        else if (SIGSEGV == sig)
            sErrInfo = "SIGSEGV Signo";
        else
            sErrInfo = "Exception Signo - " + lexical_cast<string>(sig);
        TFWException ex(m_SignoCurrLine, __FUNCTION__, sErrInfo + " [" + n + "]");
        ex.CurrPosInfo() = this->m_SignoCurrPosInfo;
        ex.ExceptType() = sErrInfo;
        ex.BackTrace() = this->m_BackTrace;
        throw ex;
    }
}

// 信号捕捉
int CSignoCatch::SignoTryPoint(ISignoCatch::ISignoTry& it, jmp_buf& env)
{
    int signo = setjmp(env);
    if (0 == signo) it.Try();
    return signo;
}

// 信号捕捉函数
void CSignoCatch::SignoCatchPoint(ISignoTry& it)
{
    this->SignoResult(this->SignoCatchPointNoThrow(it), it.Name());
}
int CSignoCatch::SignoCatchPointNoThrow(ISignoTry& it)
{
    // 添加栈跳转点
    jmp_buf env = {0};
    auto FBegin = [&]()
    {
        if (nullptr != g_context && g_context->GetCfgLogLevel() <= (int)log::trivial::debug)
            g_context->WriteLogDebug((string(g_context->getHint("Push_Jmp_Point_")) + " [" + lexical_cast<string>(g_SigCatchThrdVar.GetSigCatchThrdVarID()) + "] [" + it.Name() + "] ").c_str(), it.Info());
        this->PushJmpPoint(it.Name() + string(" @ ") + it.Info(), env, it);
    };
    auto FEnd = [&]()
    {
        this->PopJmpPoint();
        if (nullptr != g_context && g_context->GetCfgLogLevel() <= (int)log::trivial::debug)
            g_context->WriteLogDebug((string(g_context->getHint("Pop_Jmp_Point_")) + " [" + lexical_cast<string>(g_SigCatchThrdVar.GetSigCatchThrdVarID()) + "] [" + it.Name() + "] ").c_str(), it.Info());
    };
    CAutoRelease _autoPop(FBegin, FEnd);
    // 信号捕捉
    return this->SignoTryPoint(it, env);
}
