#include "std.h"
#include "bundle.h"
#include "bundle_context.h"

//---------------------------------------------------------------------------
// TBundle类
CBundle::CBundle(const char* name, const char* path, CBundleContext& ct)
		: m_name(name), m_path(path), m_Context(ct), m_state(bsFree)
{
    // 模块路径转换为绝对路径
    if (!boost::filesystem::path(m_path).is_complete())
        m_path = string(ct.getPath()) + "/" + m_path;
    // 格式化路径字符串
    m_path = CUtilFunc::FormatPath(m_path);
}

CBundle::~CBundle()
{
	try
	{
		this->free();
	}
	CATCH_EXCEPTION_TO_WRITELOG(*this, this->getName())
}

// 得到模块名称（子目录）
const char* CBundle::getName(void) const
{
	return m_name.c_str();
}

// 得到模块目前状态
CBundle::TBundleState CBundle::getState(void) const
{
	return m_state;
}

// 得到所在目录
const char* CBundle::getPath(void) const
{
	return m_path.c_str();
}

// 加载DLL
bool CBundle::load(void) const
{
	if (bsFree == m_state)
	{
        // 加载动态库
        string sError;
        if (nullptr == m_libPtr)
            m_libPtr = new KLoadLibrary<true>((m_path + "/" + m_name).c_str(), sError);
        if (sError.length() > 0)
            throw TFWBundleException(1, __FUNCTION__, sError, m_name);
        // 初始化函数
        typedef IBundleActivator& CALL_TYPE (*TFuncInitActor)(const IBundle&);
        TFuncInitActor _Init = m_libPtr->GetLibFunc<TFuncInitActor>(g_ModuleInitActor, sError);
        if (sError.length() > 0)
            throw TFWBundleException(2, __FUNCTION__, sError, m_name);
        if (nullptr == _Init)
            throw TFWBundleException(3, __FUNCTION__, this->getContext().getHint("Can_t_get_the_function_") + m_name + "::InitActor", m_name);
        m_actor = &_Init(*this);
        if (nullptr != m_actor) m_state = bsLoaded;
	}
	return bsFree != m_state;
}

// 释放DLL
bool CBundle::free(void) const
{
	if (bsStarted == m_state)
		this->stop();
	if (bsLoaded == m_state)
    {
        if (nullptr != m_libPtr)
        {
            // 卸载函数
            typedef void CALL_TYPE (*TFuncUninitActor)(void);
            TFuncUninitActor _Uninit = m_libPtr->GetLibFunc<TFuncUninitActor>(g_ModuleUninitActor);
            _Uninit();
            m_actor = nullptr;
            // 卸载动态库
            delete m_libPtr;
            m_libPtr = nullptr;
        }
		m_state = bsFree;
    }
	return bsFree == m_state;
}

// 启动模块
bool CBundle::start(void) const
{
	try
	{
		if (bsFree == m_state)
			this->load();
		if (bsLoaded == m_state && nullptr != m_actor && m_actor->start())
			m_state = bsStarted;
	}
	CATCH_EXCEPTION_TO_WRITELOG(*this, this->getName())
	return bsStarted == m_state;
}

// 停止模块
bool CBundle::stop(void) const
{
	try
	{
		if (bsStarted == m_state && nullptr != m_actor && m_actor->stop())
			m_state = bsLoaded;
		if (nullptr == m_actor)
			m_state = bsFree;
	}
	CATCH_EXCEPTION_TO_WRITELOG(*this, this->getName())
	return bsStarted != m_state;
}

// 得到模块上下文
IBundleContext& CBundle::getContext(void) const
{
    return m_Context;
}

// 写日志
bool CBundle::WriteLog(TLogInfo log) const
{
	try
	{
		if (0 == log.m_LogType) log.m_LogType = 1;
		log.m_bundleName = m_name;
	}
	catch(...) {}
	return m_Context.WriteLog(log);
}
