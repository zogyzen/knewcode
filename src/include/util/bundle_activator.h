#pragma once

#include <boost/property_tree/ptree.hpp>

#include "framework/bundle_activator_i.h"
#include "for_user/kc_controller_i.h"
#include "util/ctrl_common.h"
#include "util/backtrace_sigsegv.h"

namespace KC
{
    // 插件包含的控制器
    template<typename ICtrlWork>
    class TKCCtrlWork : public ICtrlWork
    {
    public:
        TKCCtrlWork(const IBundle& bundle) : m_context(bundle.getContext()), m_bundle(bundle)
        {
        }
        ~TKCCtrlWork() override = default;

        // 对应的模块
        const IBundle& CALL_TYPE getBundle(void) const override
        {
            return this->m_bundle;
        }

        // 控制器接口
        virtual void initAllCtrl(void) = 0;
        bool CALL_TYPE hasCtrl(const char* pName = nullptr) const override
        {
            return CCtrlCommon::hasCtrl(this->m_ctrls, pName);
        }
        IKCController& CALL_TYPE getCtrl(const char* pName = "") override
        {
            return CCtrlCommon::getCtrl<typename ICtrlWork::TCtrlException>(*this, this->m_ctrls, pName);
        }

        // 初始化/释放
        virtual bool init(void)
        {
            this->initAllCtrl();
            return true;
        }
        virtual bool free(void)
        {
            this->m_ctrls.clear();
            return true;
        }

        // 开始/停止
        virtual bool start(void)
        {
            bool bResult = true;
            for (auto &ctrl : m_ctrls) bResult = ctrl.second->start() && bResult;
            return bResult;
        }
        virtual bool stop(void)
        {
            bool bResult = true;
            for (auto &ctrl : m_ctrls) bResult = ctrl.second->stop() && bResult;
            return bResult;
        }

        // 服务别名列表
        std::string srvAliasList(void) { return m_srvAliasList; }

    protected:
        // 插件及上下文
        IBundleContext& m_context;
        const IBundle& m_bundle;
        // 控制器列表
        CCtrlCommon::TKCControllerMap m_ctrls;
        // 服务别名列表
        std::string m_srvAliasList;
    };
    template<typename ICtrlWork, typename TCtrl>
    class TKCCtrlWorkCommon : public TKCCtrlWork<ICtrlWork>
    {
    public:
        using TKCCtrlWork<ICtrlWork>::TKCCtrlWork;

        // 初始化控制器
        void initAllCtrl(void) override
        {
            this->m_srvAliasList = CCtrlCommon::GetAllCtrl(*this, m_pt, this->m_context.GetCfgFile(), this->m_ctrls,
                [&](string sName, boost::property_tree::ptree& v) { return new TCtrl(*this, sName, v); });
        }

    protected:
        // 插件配置
        boost::property_tree::ptree m_pt;
    };

    // 插件自加载类
    template<typename TKCSrv>
    class TActivator : public IBundleActivator
    {
    public:
        TActivator(const IBundle& bundle) : m_context(bundle.getContext()), m_bundle(bundle) {}
        ~TActivator()  override = default;

        // 启动模块（注册服务等）
        bool CALL_TYPE start(void) override
        {
            if (nullptr == m_srv)
                m_srv = new TKCSrv(m_bundle);
            if (nullptr == m_sreg)
                m_sreg = m_context.registerService(*m_srv, 0);
            return m_srv->init();
        }
        // 停止模块（注销服务等）
        bool CALL_TYPE stop(void) override
        {
            m_context.unregisterService(m_sreg);
            m_sreg = nullptr;
            bool bResult = m_srv->free();
            delete m_srv;
            m_srv = nullptr;
            return bResult;
        }

        // 整个框架启动完成
        bool CALL_TYPE context_started(void) override
        {
            return nullptr != m_srv ? m_srv->start() : true;
        }
        // 整个框架将要停止
        bool CALL_TYPE context_will_stop(void) override
        {
            return nullptr != m_srv ? m_srv->stop() : true;
        }

    private:
        // 插件上下文
        IBundleContext& m_context;
        const IBundle& m_bundle;

        // 服务层
        TKCSrv *m_srv = nullptr;
        IServiceRegistration *m_sreg = nullptr;
    };
}

#define KC_SET_ACTIVATOR(TActor) \
    static TActor* g_actor = nullptr; \
    extern "C" \
    { \
        KC::IBundleActivator& CALL_TYPE InitActor(const KC::IBundle& bundle) \
        { \
                if (nullptr == g_actor) { \
                    /*ExceptBacktrace::SetExceptFilter(bundle.getPath());*/ \
                    g_actor = new TActor(bundle); \
                } \
                return *g_actor; \
        } \
        void CALL_TYPE UninitActor(void) \
        { \
                delete g_actor; \
                g_actor = nullptr; \
        } \
    }
