#include "bundle_context.h"
#include "framework.h"
#include "service_registration.h"
#include "service_reference.h"
#include "bundle.h"

//---------------------------------------------------------------------------
// TBundleContext类
CBundleContext::CBundleContext(IKCStartWork& own, CFramework& fw, string pathModules, string sCfg)
    : m_own(own), m_Framework(fw), m_pathModules(pathModules), m_cfgFile(sCfg)
    , m_LockWork(*this), m_logThrd(*this)
{
    cout << "*[knewcode] CBundleContext::CBundleContext: " << m_pathModules << endl << "\t" << m_cfgFile << endl;
    if (boost::filesystem::exists(m_cfgFile))
        try
        {
            read_xml(m_cfgFile, m_cfgPt);
        }
        catch (std::exception &ex)
        {
            cout << "*[knewcode] CBundleContext::CBundleContext: XML Format Error - <" << typeid(ex).name() << "> " << ex.what() << endl;
        }
    else
        cout << "*[knewcode] CBundleContext::CBundleContext: Not Exists - " << m_cfgFile << endl;
    // 构建时间：Tue_Feb_6_01:27:06_2024
    string sDt(APP_COMPILE_DATETIME);
    cout << "*[knewcode] Build Datetime: " << sDt << endl;
    vector<string> vctDt;
    boost::split(vctDt, sDt, boost::is_any_of("_"));
    if (vctDt.size() != 5) throw std::runtime_error("Build Datetime Format Error - " + sDt);
    sDt = vctDt[4] + "-" + vctDt[1] + "-" + vctDt[2] + " " + vctDt[3];
    cout << "*[knewcode] Build Datetime: " << sDt << endl;
    auto dttm = boost::posix_time::time_from_string(sDt);
    auto dt = dttm.date();
    auto tm = dttm.time_of_day();
    int iYear = dt.year();
    m_buildDt = (boost::format("%04d.%02d.%02d.%02d%02d") % iYear % (int)(dt.month()) % (int)(dt.day()) % (int)(tm.hours()) % (int)(tm.minutes())).str();
    cout << "*[knewcode] Build Datetime: " << m_buildDt << endl;
}

CBundleContext::~CBundleContext()
{
	try
	{
        // 卸载漏掉的插件
        if (!m_BundleMap.empty())
        {
            WriteLogInfo("Unregister Miss Bundle", __CURR_CODE_PLACE_C__);
            boost::unique_lock<boost::mutex> lck(m_mtxBnd);
            m_BundleMap.clear();
        }
        // 注销漏掉的服务
        if (!m_SrvRegMap.empty())
        {
            WriteLogInfo("Unregister Miss Service", __CURR_CODE_PLACE_C__);
            boost::unique_lock<boost::shared_mutex> lck(m_mtxSrv);
            m_SrvRegMap.clear();
        }
        // 删除配置
        m_cfgPt.clear();
    }
	CATCH_EXCEPTION_TO_WRITELOG(*this, "")
}

// 得到主启动接口
IKCStartWork& CBundleContext::GetMain(void)
{
    //cout << hex << &m_own << "\t" << typeid(m_own).name() << endl;
    return m_own;
}

// 安装、卸载模块
IBundle* CBundleContext::installBundle(const char* sName, const char* sPath, IBundle::TBundleState iAct, const char* sSrvAliasList)
{
    CBundle* result = nullptr;
    try
	{
        cout << "...";
		// 创建模块代理
        CBundle* bundle = new CBundle(sName, sPath, sSrvAliasList, *this);
		TBundlePtr bundlePtr(bundle);
		// 加锁
        WriteLogInfo((string("###### ") + this->getHint("Install_Bundle_") + sName).c_str(), __CURR_CODE_PLACE_C__, sPath);
        boost::unique_lock<boost::mutex> lck(m_mtxBnd);
		// 检查是否重复注册
		TBundleMap::iterator iter = m_BundleMap.find(bundle->getName());
		if (m_BundleMap.end() != iter)
			throw TFWBundleException(1, __CURR_CODE_PLACE_C__, string(this->getHint("Repeat_to_install_")) + sName, bundle->getName());
		// 初始加载或启动
        bool bSucc = true;
        string sInstallType;
		switch (iAct)
		{
		case IBundle::bsLoaded:   	// 初始加载
            bSucc = bundle->load();
            sInstallType = "Load";
			break;
		case IBundle::bsStarted:	// 初始启动
            bSucc = bundle->start();
            sInstallType = "Start";
            break;
        case IBundle::bsFree:
            sInstallType = "Install";
            break;
		}
		// 保存模块
        m_BundleMap.insert(make_pair(string(bundle->getName()), bundlePtr));
        // 写安装日志
        string sLog = "";
        if (bSucc)
        {
            sLog = string() + this->getHint("Install_Bundle_Success_") + sName + "......" + sInstallType;
            bundle->WriteLogInfo(sLog.c_str(), __CURR_CODE_PLACE_C__);
        }
        else
        {
            sLog = string() + this->getHint("Install_Bundle_Fail_") + sName + "......" + sInstallType;
            bundle->WriteLogError(sLog.c_str(), __CURR_CODE_PLACE_C__);
        }
        // cout << CUtilFunc::Utf8ToGbk(sLog) << endl;
        cout << "\t" << (bSucc ? "***" : "---") << "[knewcode] Install Bundle " << (bSucc ? "Success" : "Failure") << " - " << sName << " \t\t" << sInstallType << endl;
        result = bundle;
	}
	CATCH_EXCEPTION_TO_WRITELOG(*this, sName)
	return result;
}

bool CBundleContext::uninstallBundle(IBundle*& bundle)
{
	try
	{
		if (nullptr != bundle)
		{
			// 加锁
            WriteLogInfo((string(this->getHint("Uninstall_Bundle_")) + bundle->getName()).c_str(), __CURR_CODE_PLACE_C__);
            boost::unique_lock<boost::mutex> lck(m_mtxBnd);
			// 判断模块是否存在
			TBundleMap::iterator iter = m_BundleMap.find(bundle->getName());
			if (m_BundleMap.end() != iter)
			{
				// 删除模块
                bundle->WriteLogInfo((this->getHint("Uninstall_Bundle_") + iter->first).c_str(), __CURR_CODE_PLACE_C__);
				m_BundleMap.erase(iter);
			}
			bundle = nullptr;
		}
	}
	CATCH_EXCEPTION_TO_WRITELOG(*dynamic_cast<IBundleEx*>(bundle), bundle->getName())
	return nullptr == bundle;
}

// 获取模块
bool CBundleContext::ExistsBundle(const char* name) const
{
	return m_BundleMap.end() != m_BundleMap.find(name);
}

// 获取模块
const IBundle& CBundleContext::getBundle(const char* name) const
{
	TBundleMap::const_iterator iter = m_BundleMap.find(name);
	if (m_BundleMap.end() == iter)
        throw TFWBundleException(1, __CURR_CODE_PLACE_C__, string(this->getHint("Can_t_find_the_bundle_")) + name, name);
	return *iter->second;
}

// 注册、注销服务
IServiceRegistration* CBundleContext::registerService(IService& srv, int aliasSort)
{
	CServiceRegistration *reg = nullptr;
	try
	{
	    // 得到服务唯一标识和类型名称
		string  symbolic = srv.getGUID(),
                sName = srv.getName();
        WriteLogInfo((string("====== ") + this->getHint("Register_Service_") + sName + "[" + symbolic + "]").c_str(), __CURR_CODE_PLACE_C__);
        // 服务特征码不能为空
		if (symbolic.empty())
            throw TFWSrvRegException(1, __CURR_CODE_PLACE_C__, this->getHint("The_service_GUID_is_null_") + sName, sName, symbolic);
		// 创建服务注册
		reg = new CServiceRegistration(srv, *this);
		TSrvRegPtr regPtr(reg);
		// 加锁
        boost::unique_lock<boost::shared_mutex> lck(m_mtxSrv);
		// 检查是否重复注册
		TSrvRegMap::iterator iter = m_SrvRegMap.find(symbolic);
		if (m_SrvRegMap.end() != iter)
            throw TFWSrvRegException(2, __CURR_CODE_PLACE_C__, this->getHint("Repeat_to_register_service_") + sName, sName, symbolic);
        // 保存服务到插件
        const CBundle& bund = dynamic_cast<const CBundle&>(srv.getBundle());
        string sAlias = bund.registerService(srv, aliasSort);
        // 服务别名
        if (!sAlias.empty() && m_CfgAlias.end() == m_CfgAlias.find(sAlias))
            m_CfgAlias.insert(make_pair(sAlias, srv.getGUID()));
		// 保存服务注册
        m_SrvRegMap.insert(make_pair(symbolic, regPtr));
        ((IServiceEx&)srv).WriteLogInfo((this->getHint("Register_Service_Success_") + sName).c_str(), __CURR_CODE_PLACE_C__);
	}
	CATCH_EXCEPTION_TO_WRITELOG((IServiceEx&)srv, srv.getGUID())
	return reg;
}

// 注销服务
bool CBundleContext::unregisterService(IServiceRegistration*& reg)
{
    if (nullptr != reg && this->unregisterService(reg->getGUID()))
        reg = nullptr;
	return nullptr == reg;
}
bool CBundleContext::unregisterService(const char* symbolic)
{
    bool bResult = false;
	try
	{
		// 加锁
        WriteLogInfo((this->getHint("Unregister_Service_") + string("[") + symbolic + "]").c_str(), __CURR_CODE_PLACE_C__);
        boost::unique_lock<boost::shared_mutex> lck(m_mtxSrv);
		// 判断服务是否存在
		TSrvRegMap::iterator iter = m_SrvRegMap.find(symbolic);
		if (m_SrvRegMap.end() != iter)
		{
            // 从插件中删除
            auto &srv = iter->second->getService();
            const CBundle& bund = dynamic_cast<const CBundle&>(srv.getBundle());
            bund.unregisterService(srv);
            // 删除注册
            ((IServiceEx&)srv).WriteLogInfo((string(this->getHint("Unregister_Service_")) + srv.getName()).c_str(), __CURR_CODE_PLACE_C__);
			m_SrvRegMap.erase(iter);
		}
		bResult = true;
	}
	CATCH_EXCEPTION_TO_WRITELOG(*this, symbolic)
	return bResult;
}

// 获取服务的注册
boost::weak_ptr<IServiceRegistration> CBundleContext::getSrvReg(string symbolic)
{
    WriteLogTrace((this->getHint("Take_Reference_Service") + string(" - [") + symbolic + "]").c_str(), __CURR_CODE_PLACE_C__);
    boost::shared_lock<boost::shared_mutex> lck(m_mtxSrv);
    TSrvRegMap::iterator iter = m_SrvRegMap.find(symbolic);
    if (m_SrvRegMap.end() == iter)
        throw TFWException(1, __CURR_CODE_PLACE_C__, this->getHint("The_service_don_t_exist_") + symbolic);
    return boost::weak_ptr<IServiceRegistration>(iter->second);
}

// 服务的引用、释放
IServiceReference* CBundleContext::takeServiceReference(const char* symbolic)
{
	CServiceReference *refSrv = nullptr;
	try
	{
        string sSymbolic = CUtilFunc::PCharSafeToStr(symbolic);
        if (sSymbolic.empty())
            throw TFWException(1, __CURR_CODE_PLACE_C__, this->getHint("The_service_don_t_exist_") + string("[null]"));
        // 创建服务引用
        static std::atomic_ullong iRefID(0);
        refSrv = new CServiceReference(symbolic, ++iRefID, *this);
        refSrv->WriteLogTrace((this->getHint("Take_Reference_Service") + string(" - [") + symbolic + "]").c_str(), __CURR_CODE_PLACE_C__);
    }
	CATCH_EXCEPTION_TO_WRITELOG(*this, symbolic)
	return refSrv;
}

// 释放引用
bool CBundleContext::freeServiceReference(IServiceReference*& refSrv)
{
	try
	{
	    if (nullptr != refSrv)
        {
            refSrv->WriteLogTrace((this->getHint("Free_Reference_Service") + string(" - [") + refSrv->getGUID() + "]").c_str(), __CURR_CODE_PLACE_C__);
            delete dynamic_cast<CServiceReference*>(refSrv);
            refSrv = nullptr;
        }
	}
	CATCH_EXCEPTION_TO_WRITELOG(*dynamic_cast<IServiceReferenceEx*>(refSrv), refSrv->getGUID())
	return nullptr == refSrv;
}

// 得到框架模块目录
const char* CBundleContext::getPath(void) const
{
	return m_pathModules.c_str();
}

// 得到网站或应用根路径
const char* CBundleContext::getWebsiteRootPath(void) const
{
    return m_own.GetWebsiteRootPath();
}

// 配置路径转换到本地完整路径
const char* CBundleContext::transCfgPathToFullPath(const char* sCfgPath) const
{
    static thread_local string sResult;
    sResult = m_pathModules;
    string strCfgPath = boost::algorithm::trim_copy(CUtilFunc::PCharSafeToStr(sCfgPath));
    if (!strCfgPath.empty())
    {
        // 以框架目录为基准根路径
        if (strCfgPath[0] == '>')
            sResult = CUtilFunc::ToAbsPath(strCfgPath.substr(1), m_pathModules);
        // 相对路径，以网站或应用（前后端）为基准根路径
        else if (!boost::filesystem::path(strCfgPath).is_absolute())
            sResult = CUtilFunc::ToAbsPath(strCfgPath, CUtilFunc::PCharSafeToStr(getWebsiteRootPath()));
        // 绝对路径
        else sResult =  strCfgPath;
        // 去掉..和.
        if (boost::filesystem::exists(sResult))
            sResult = boost::filesystem::canonical(sResult).string();
    }
    return sResult.c_str();
}

// 得到配置的目录
const char* CBundleContext::getPathCfg(const char* cfg) const
{
    auto iter = m_CfgDirs.find(cfg);
    if (m_CfgDirs.end() == iter)
        throw TFWException(1, __CURR_CODE_PLACE_C__, string(this->getHint("The_path_config_don_t_exist_")) + cfg);
    return iter->second.c_str();
}

// 获取本地化语言选项
const char* CBundleContext::getSelLang(void) const
{
    return m_selLang.c_str();
}

// 获取本地化提示信息
const char* CBundleContext::getHint(const char* key, const char* def) const
{
    auto iter = m_ptHints.find(key);
    if (m_ptHints.end() != iter)
        return iter->second.c_str();
    return CUtilFunc::PCharSafeToPChar(def, key);
}

// 得到配置的日志等级
int CBundleContext::GetCfgLogLevel(void) const
{
    return (int)m_logThrd.GetCfgLogLevel();
}

// 写日志
bool CBundleContext::WriteLog(TLogInfo log) const
{
    log.m_ProcessID = boost::this_process::get_id();
    log.m_threadID = CUtilFunc::CurrThreadID();
    m_logThrd.AppendLog(log);
	return true;
}

IKcLockWork& CBundleContext::LockWork(void)
{
    return m_LockWork;
}

int CBundleContext::GetTimeOutSeconds(void) const
{
    return m_timeout_seconds;
}

// 得到服务器编号
unsigned CBundleContext::GetSrvID(const char* sAttr) const
{
    return atoi(GetCfgInfo("Config.Parameters.srvID", sAttr, "1"));
}

// 得到系统标志信息
const char* CBundleContext::GetSysFlag(const char* sAttr, const char* sDefault) const
{
    return GetCfgInfo("Config.Parameters.sys_flag", sAttr, sDefault);
}

// 得到配置信息
const char* CBundleContext::GetCfgInfo(const char* sNode, const char* sAttr, const char* sDefault) const
{
    static thread_local string sResult;
    sResult.clear();
    sResult = CUtilFunc::PCharSafeToStr(sDefault);
    string strNode = CUtilFunc::PCharSafeToStr(sNode);
    string strAttr = CUtilFunc::PCharSafeToStr(sAttr);
    if (!strNode.empty() && !m_cfgPt.empty())
    {
        string strNodeAttr = strNode;
        if (!strAttr.empty())
            strNodeAttr = strNode + ".<xmlattr>." + strAttr;
        if (m_cfgPt.get_child_optional(strNodeAttr))
            sResult = m_cfgPt.get<string>(strNodeAttr);
    }
    boost::algorithm::trim(sResult);
    return sResult.c_str();
}

// 通过别名得到服务标识
const char* CBundleContext::GetSrvGUIDByAlias(const char* alias) const
{
    auto it = m_CfgAlias.find(alias);
    if (m_CfgAlias.end() != it)
        return it->second.c_str();
    return alias;
}

// 子配置项
int CBundleContext::GetCfgSubCount(const char* sNode) const
{
    int iResult = 0;
    string strNode = CUtilFunc::PCharSafeToStr(sNode);
    if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
        iResult = static_cast<int>(m_cfgPt.get_child(strNode).size());
    return iResult;
}
bool CBundleContext::IsCfgSubValid(const char* sNode, int id) const
{
    bool bResult = false;
    string strNode = CUtilFunc::PCharSafeToStr(sNode);
    if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
    {
        auto ptNode = m_cfgPt.get_child(strNode);
        auto iter = ptNode.begin();
        for (; ptNode.end() != iter && id > 0; ++iter, --id);
        if (ptNode.end() != iter && 0 == id)
            bResult = c_RESTful_xmlcomment != iter->first;
    }
    return bResult;
}
const char* CBundleContext::GetCfgSubName(const char* sNode, int id) const
{
    static thread_local string sResult;
    sResult.clear();
    string strNode = CUtilFunc::PCharSafeToStr(sNode);
    if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
    {
        auto ptNode = m_cfgPt.get_child(strNode);
        auto iter = ptNode.begin();
        for (; ptNode.end() != iter && id > 0; ++iter, --id);
        if (ptNode.end() != iter && 0 == id && c_RESTful_xmlcomment != iter->first)
            sResult = iter->first;
    }
    return sResult.c_str();
}
const char* CBundleContext::GetCfgSubInfo(const char* sNode, int id, const char* sAttr, const char* sDefault) const
{
    static thread_local string sResult;
    sResult.clear();
    sResult = CUtilFunc::PCharSafeToStr(sDefault);
    string strNode = CUtilFunc::PCharSafeToStr(sNode);
    string strAttr = CUtilFunc::PCharSafeToStr(sAttr);
    if (!strNode.empty() && !m_cfgPt.empty() && m_cfgPt.get_child_optional(strNode))
    {
        auto ptNode = m_cfgPt.get_child(strNode);
        auto iter = ptNode.begin();
        for (; ptNode.end() != iter && id > 0; ++iter, --id);
        if (ptNode.end() != iter && 0 == id && c_RESTful_xmlcomment != iter->first)
        {
            if (strAttr.empty())
                sResult = iter->second.get_value<string>();
            else
            {
                auto ptAttr = iter->second.get_child_optional("<xmlattr>." + strAttr);
                if (ptAttr) sResult = ptAttr.get().get_value<string>();
            }

        }
    }
    return sResult.c_str();
}

// 得到配置文件
const char* CBundleContext::GetCfgFile(void) const
{
    if (!boost::filesystem::exists(m_cfgFile))
        throw TFWException(1, __CURR_CODE_PLACE_C__, "Don't exists config file [" + m_cfgFile + "].");
    return m_cfgFile.c_str();
}

// 启动
void CBundleContext::startup(void)
{
    bool bLogInit = false;
	try
	{
        if (!boost::filesystem::exists(m_cfgFile))
            throw TFWException(1, __CURR_CODE_PLACE_C__, "Don't exists config file [" + m_cfgFile + "].");
        if (m_cfgPt.empty())
            throw TFWException(1, __CURR_CODE_PLACE_C__, "The config file [" + m_cfgFile + "] is empty.");
        // 初始化信息提示信息
        this->init_hint();
        // 初始化日志
        this->init_log();
        bLogInit = true;
        // 初始化超时时间
        this->init_timeout();
        // 循环所有的目录
        if (m_cfgPt.get_child_optional("Config.Directories"))
        {
            BOOST_FOREACH(property_tree::ptree::value_type &v, m_cfgPt.get_child("Config.Directories"))
            {
                m_CfgDirs[v.first] = this->GetFullPath(v.second.get<string>("<xmlattr>.Path"));
            }
        }
        // 当前路径
        boost::filesystem::path curPath = boost::filesystem::current_path();
        boost::filesystem::current_path(getPath());
        CAutoRelease _auto([&](){ boost::filesystem::current_path(curPath); });
        // 循环所有的模块
        if (m_cfgPt.get_child_optional("Config.Modules"))
        {
            BOOST_FOREACH(property_tree::ptree::value_type &v, m_cfgPt.get_child("Config.Modules"))
            {
                if (c_RESTful_xmlcomment != v.first)
                {
                    // 活动状态
                    IBundle::TBundleState bdlStt = IBundle::bsStarted;
                    string sAct = v.second.get<string>("<xmlattr>.Activation");
                    trim(sAct);
                    if (sAct == "0") bdlStt = IBundle::bsFree;
                    else if (sAct == "1") bdlStt = IBundle::bsLoaded;
                    else bdlStt = IBundle::bsStarted;
                    // 路径
                    string sDllPath = "";
                    if (v.second.get_child_optional("<xmlattr>.Path"))
                        sDllPath = v.second.get<string>("<xmlattr>.Path");
                    // 服务别名列表（逗号分隔）
                    string sSrvAliasList = "";
                    if (v.second.get_child_optional("<xmlattr>.srv"))
                        sSrvAliasList = v.second.get<string>("<xmlattr>.srv");
                    // 安装插件
                    this->installBundle(v.first.c_str(), sDllPath.c_str(), bdlStt, sSrvAliasList.c_str());
                }
            }
            // 所有插件启动完成
            for (auto &bundle : m_BundleMap) bundle.second->context_started();
        }
    }
	catch(TException& ex)
	{
        cout << "*[knewcode] CBundleContext::startup Failed: " << bLogInit << " [" << typeid(ex).name() << "] " << ex.error_info() << endl;
        if (bLogInit)
        {
            TLogInfo log(ex.error_info(), __CURR_CODE_PLACE_C__, IFuncLog::lglvError);
            log.m_excpType = typeid(ex).name();
            log.m_excpID = ex.error_id();
            log.m_lineCode = __LINE__;
            this->WriteLog(log);
        }
        throw;
	}
	catch(std::exception& ex)
	{
        cout << "*[knewcode] CBundleContext::startup Failed: " << bLogInit << " [" << typeid(ex).name() << "] " << ex.what() << endl;
        if (bLogInit)
        {
            TLogInfo log(ex.what(), __CURR_CODE_PLACE_C__, IFuncLog::lglvFatal);
            log.m_excpType = typeid(ex).name();
            log.m_lineCode = __LINE__;
            this->WriteLog(log);
        }
        throw;
	}
	catch(const char* ex)
	{
        cout << "*[knewcode] CBundleContext::startup Failed: " << bLogInit << " " << ex << endl;
        if (bLogInit)
        {
            TLogInfo log(ex, __CURR_CODE_PLACE_C__, IFuncLog::lglvFatal);
            log.m_excpType = "const char*";
            log.m_lineCode = __LINE__;
            this->WriteLog(log);
        }
		throw;
	}
	catch(...)
	{
        cout << "*[knewcode] CBundleContext::startup Failed: " << bLogInit << endl;
        if (bLogInit)
        {
            TLogInfo log(this->getHint("Unknown_exception"), __CURR_CODE_PLACE_C__, IFuncLog::lglvFatal);
            log.m_lineCode = __LINE__;
            this->WriteLog(log);
        }
		throw;
	}
}

// 关闭上下文
void CBundleContext::shutoff(void)
{
    // 停止插件
    for (auto &bundle : m_BundleMap) bundle.second->context_will_stop();
    for(TBundleMap::iterator it = m_BundleMap.begin(); m_BundleMap.end() != it; ++it)
        it->second->stop();
    // 卸载插件
    vector<IBundle*> bundleList;
    for(TBundleMap::iterator it = m_BundleMap.begin(); m_BundleMap.end() != it; ++it)
        bundleList.push_back(it->second.get());
    for(vector<IBundle*>::iterator it = bundleList.begin(); bundleList.end() != it; ++it)
        this->uninstallBundle(*it);
    // 写结束日志
    this->WriteLogInfo(((boost::format(this->getHint("End_framework")) % VersionInfo() % m_cfgFile).str()).c_str(), __CURR_CODE_PLACE_C__);
    // 结束写日志线程
    this->m_logThrd.Stop();
}

// 初始化日志
void CBundleContext::init_log(void)
{
    // 启动写日志线程
    m_logThrd.Start(CUtilFunc::PCharSafeToStr(m_own.OwnName()));
    // 写开始日志
    string sVer = m_own.OwnVersion() + string(" | ") + VersionInfo();
    string sLog = (format(this->getHint("Start_framework", "***** Start %1% ***** - %2%")) % sVer % m_cfgFile).str();
    this->WriteLogInfo(sLog.c_str(), __CURR_CODE_PLACE_C__, (format("max size = %dM") % (string().max_size() / 1024.0 / 1024)).str().c_str());
    // cout << CUtilFunc::Utf8ToGbk(sLog) << endl;
    cout << "*[knewcode] Start " << sVer << " - " << m_cfgFile << endl << endl;
}

// 初始化信息提示信息
void CBundleContext::init_hint(void)
{
    if (!m_cfgPt.empty())
    {
        // 多语言配置
        string sCfgLoc = "cn";
        if (m_cfgPt.get_child_optional("Config.Parameters.hint_file"))
            sCfgLoc = m_cfgPt.get<string>( "Config.Parameters.hint_file.<xmlattr>.File" );
        string sHintFile = CUtilFunc::ToAbsPath("hint_file/" + sCfgLoc + ".xml", m_pathModules);
        // 直接配置的文件
        if (!boost::filesystem::exists(sHintFile))
            sHintFile = this->transCfgPathToFullPath(sCfgLoc.c_str());
        cout << "*[knewcode] CBundleContext::init_hint: " << sHintFile << endl;
        // 读取信息提示文件
        if (boost::filesystem::exists(sHintFile))
        {
            m_selLang = boost::filesystem::path(sHintFile).stem().string();
            boost::property_tree::ptree ptHint;
            read_xml(sHintFile, ptHint);
            // 循环所有的信息提示
            if (ptHint.get_child_optional("Hint"))
            {
                BOOST_FOREACH(property_tree::ptree::value_type &v, ptHint.get_child("Hint"))
                {
                    m_ptHints[v.first] = v.second.get<string>("<xmlattr>.String");
                }
            }
        }
    }
}

// 初始化超时时间
void CBundleContext::init_timeout(void)
{
    if (!m_cfgPt.empty())
    {
        if (m_cfgPt.get_child_optional("Config.Parameters.timeout"))
        {
            string sSeconds = m_cfgPt.get<string>( "Config.Parameters.timeout.<xmlattr>.seconds" );
            m_timeout_seconds = lexical_cast<int>(sSeconds);
        }
    }
}

// 版本信息
const char* CBundleContext::VersionInfo(void) const
{
    static thread_local const string sResult = CUtilFunc::KcVersionForFullInfo() + "." + BuildDatetime();
    return sResult.c_str();
}
const char* CBundleContext::BuildDatetime(void) const
{
    return m_buildDt.c_str();
}

// 获取绝对路径
string CBundleContext::GetFullPath(string sPath)
{
    string sResult = sPath;
    if (!boost::filesystem::path(sPath).is_absolute())
        sResult = m_pathModules + "/" + sPath;
    return CUtilFunc::FormatPath(sResult);
}

// 加解密
const char* CBundleContext::Encrypted(const char* src, unsigned long len, const char* key) const
{
    static thread_local string sResult;
    if (nullptr != src && len > 0)
    {
        sResult = string(src, len);
        try
        {
            const std::string raw_data(src, len);
            const char keyArr[33] = { 0 };
            memcpy((void*)keyArr, key, std::min((size_t)32, strlen(key)));
            const std::vector<unsigned char> keyBytes = plusaes::key_from_string(&keyArr);
            const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(raw_data.size());
            std::vector<unsigned char> encrypted(encrypted_size);
            memset(encrypted.data(), 0, encrypted.size());
            if (plusaes::encrypt_cbc((unsigned char*)raw_data.data(), raw_data.size(), &keyBytes[0], keyBytes.size(), &c_plusAesIV, &encrypted[0], encrypted.size(), true) == plusaes::kErrorOk)
                sResult = CUtilFunc::Base64Encode(encrypted.data(), encrypted_size);
        }
        catch (...) {}
    }
    return sResult.c_str();
}
const char* CBundleContext::Decrypted(const char* src, unsigned long& len, const char* key) const
{
    static thread_local string sResult;
    if (nullptr != src && len > 0)
    {
        sResult = string(src, len);
        try
        {
            std::string raw_data(src, len);
            raw_data = CUtilFunc::Base64Decode((unsigned char*)raw_data.data(), raw_data.size());
            len = raw_data.size();
            const char keyArr[33] = { 0 };
            memcpy((void*)keyArr, key, std::min((size_t)32, strlen(key)));
            const std::vector<unsigned char> keyBytes = plusaes::key_from_string(&keyArr);
            std::vector<unsigned char> decrypted(len + 1);
            memset(decrypted.data(), 0, decrypted.size());
            unsigned long padded_size = 0;
            if (plusaes::decrypt_cbc((unsigned char*)&raw_data[0], raw_data.size(), &keyBytes[0], keyBytes.size(), &c_plusAesIV, &decrypted[0], decrypted.size(), &padded_size) == plusaes::kErrorOk)
            {
                sResult = string((const char*)decrypted.data(), len);
                len = len - padded_size;
            }
        }
        catch (...) {}
    }
    return sResult.c_str();
}
