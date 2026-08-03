#include "bundle.h"
#include "bundle_context.h"

//---------------------------------------------------------------------------
// TBundle类
CBundle::CBundle(const char* name, const char* path, const char* aliasList, CBundleContext& ct)
		: m_name(name), m_path(path), m_Context(ct), m_state(bsFree)
{
    // 模块路径转换为绝对路径
    if (!boost::filesystem::path(m_path).is_absolute())
        m_path = string(ct.getPath()) + "/" + m_path;
    // 格式化路径字符串
    m_path = CUtilFunc::FormatPath(m_path);
    // 别名列表
    string sAliases = CUtilFunc::PCharSafeToStr(aliasList);
    if (!sAliases.empty()) algorithm::split(m_aliases, aliasList, is_any_of(","));
}

CBundle::~CBundle()
{
	try
	{
		this->free();
	}
	CATCH_EXCEPTION_TO_WRITELOG(*this, this->getName())
}

// 注册服务
string CBundle::registerService(IService& srv, int aliasSort) const
{
    // 服务别名
    string sAlias;
    try
    {
        if (static_cast<int>(m_aliases.size()) > aliasSort)
        {
            sAlias = boost::algorithm::trim_copy(m_aliases[aliasSort]);
            if (!sAlias.empty())
            {
                // 设置服务别名
                IServiceEx *pSrvEx = dynamic_cast<IServiceEx*>(&srv);
                if (nullptr != pSrvEx) pSrvEx->SetAlias(sAlias);
            }
        }
        // 保存服务
        string sSrvAlias = CUtilFunc::PCharSafeToStr(srv.getAlias());
        auto it = m_mapSrv.find(sSrvAlias);
        if (m_mapSrv.end() == it) m_mapSrv.insert(make_pair(sSrvAlias, &srv));
    }
    catch (...) {}
    return sAlias;
}
bool CBundle::unregisterService(IService& srv) const
{
    bool bResult = false;
    try
    {
        string sSrvAlias = CUtilFunc::PCharSafeToStr(srv.getAlias());
        auto it = m_mapSrv.find(sSrvAlias);
        if (m_mapSrv.end() != it) m_mapSrv.erase(it);
        bResult = true;
    }
    catch (...) {}
    return bResult;
}

// 整个框架启动完成
bool CBundle::context_started(void)
{
    if (bsStarted == m_state && nullptr != m_actor)
        return m_actor->context_started();
    return false;
}
// 整个框架将要停止
bool CBundle::context_will_stop(void)
{
    if (bsStarted == m_state && nullptr != m_actor)
        return m_actor->context_will_stop();
    return false;
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
        string sDllPath = m_path + "/" + m_name + c_so_ext_name;
        // 加载动态库
        if (!m_lib.is_loaded())
        {
            system::error_code ec;
            m_lib.load(sDllPath, ec, dll::load_mode::rtld_now);
            if (ec)
                throw TFWBundleException(ec.value(), __CURR_CODE_PLACE_C__, "[" + m_name + c_so_ext_name + "] " + ec.message() + "\n" + KLoadInfo::_GetLoadDllError(), m_name, sDllPath);
        }
        // 初始化函数
        if (!m_lib.has(g_ModuleInitActor))
            throw TFWBundleException(2, __CURR_CODE_PLACE_C__, this->getContext().getHint("Can_t_get_the_function_") + m_name + "::InitActor", m_name, sDllPath);
        auto _Init = m_lib.get<IBundleActivator&(const IBundle&)>(g_ModuleInitActor);
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
        if (m_lib.is_loaded())
        {
            // 卸载函数
            if (m_lib.has(g_ModuleUninitActor))
            {
                auto _Uninit = m_lib.get<void(void)>(g_ModuleInitActor);
                _Uninit();
            }
            m_actor = nullptr;
            // 卸载动态库
            m_lib.unload();
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

// 得到首个服务的别名
const char* CBundle::getFirstServiceAlias(void) const
{
    return m_mapSrv.empty() ? nullptr : m_mapSrv.begin()->first.c_str();
}

// 写日志
bool CBundle::WriteLog(TLogInfo log) const
{
	try
	{
		if (EFrameworkType::eftContext == log.m_LogType) log.m_LogType = EFrameworkType::eftBundle;
		log.m_bundleName = m_name;
	}
	catch(...) {}
	return m_Context.WriteLog(log);
}
