#pragma once

#include <string>
#include <memory>
#include <stdexcept>
#include <map>

#include <boost/dll.hpp>
#include <boost/format.hpp>

#include "common/base_type.h"
#include "util_funcs.h"

namespace KC
{
    // 加载动态库
    struct KLoadSubLib
    {
        ~KLoadSubLib(void)
        {
            UnloadLib();
        }

        // 加载动态库
        void LoadLib(std::string strLibFileName)
        {
            std::string dllName = strLibFileName + c_so_ext_name;
            if (!boost::filesystem::exists(dllName))
                throw std::runtime_error((boost::format("不存在动态库文件: %s") % dllName).str());
            std::string sError = "";
            boost::system::error_code ec;
            m_lib.load(dllName, ec, boost::dll::load_mode::rtld_now);
            if (ec)
                throw std::runtime_error((boost::format("不能加载动态库: %s\n%d - %s") % dllName % ec.value() % CUtilFunc::GbkToUtf8(ec.message())).str());
            else if (!m_lib.is_loaded())
                throw std::runtime_error((boost::format("不能加载动态库: %s") % dllName).str());
        }
        // 卸载动态库
        void UnloadLib(void)
        {
            if (IsLoaded()) m_lib.unload();
        }
        // 是否已加载
        bool IsLoaded(void)
        {
            return m_lib.is_loaded();
        }

        // 得到动态库函数
        template<typename TFunc>
        std::function<TFunc> GetFunc(std::string sName)
        {
            if (!IsLoaded())
                throw std::runtime_error("未加载动态库。不能调用：" + sName);
            if (!m_lib.has(sName))
                throw std::runtime_error("无引出函数: " + sName);
            auto Fn = m_lib.get<TFunc>(sName);
            if (nullptr == Fn)
                throw std::runtime_error("获取引出函数失败: " + sName);
            return Fn;
        }

        // 调用引出函数
        template<typename TResult, typename ...TArgs>
        TResult CallFunc(std::string sFuncName, TArgs... args)
        {
            return GetFunc<TResult(TArgs...)>(sFuncName)(args...);
        }
        template<typename ...TArgs>
        void CallProc(std::string sProcName, TArgs... args)
        {
            GetFunc<void(TArgs...)>(sProcName)(args...);
        }

    private:
        // 加载库
        boost::dll::shared_library m_lib;
    };
    typedef std::shared_ptr<KLoadSubLib> TKLoadSubLibPtr;

    // 子模块接口
    const char c_createFuncDef[] = "create", c_destroyFuncDef[] = "destroy";
    template<typename TInf>
    struct TInfInSubModule
    {
        TInf *m_inf = nullptr;
        KLoadSubLib &m_lib;
        std::string m_createFunc = c_createFuncDef;
        std::string m_destroyFunc = c_destroyFuncDef;
        unsigned m_revise = 0;      // 修订号。每次信息更新会加一。用于删除信息更新后，修订号未变的接口（也就是不用的接口）

        TInfInSubModule(KLoadSubLib &lib, std::string createFunc = c_createFuncDef, std::string destroyFunc = c_destroyFuncDef)
            : m_lib(lib), m_createFunc(createFunc), m_destroyFunc(destroyFunc)
        {
        }
        ~TInfInSubModule(void)
        {
            DestroyInf();
        }

        // 获取接口
        template<typename TOwn, typename ...TArgs>
        void CreateInf(TOwn& own, TArgs... args)
        {
            if (!m_lib.IsLoaded())
                throw std::runtime_error("未加载动态库。不能创建接口：" + m_createFunc);
            m_inf = &m_lib.GetFunc<TInf&(TOwn&, TArgs...)>(m_createFunc)(own, args...);
        }

        // 释放接口
        void DestroyInf(void)
        {
            if (m_lib.IsLoaded() && nullptr != m_inf)
                m_lib.GetFunc<void(TInf&)>(m_destroyFunc)(*m_inf);
            m_inf = nullptr;
        }

        // 是否有效
        bool IsValid(void) const
        {
            return m_lib.IsLoaded() && nullptr != m_inf;
        }
    };
    template<typename TInf>
    using TInfInSubModulePtr = std::shared_ptr<TInfInSubModule<TInf>>;

    // 子模块（单接口）
    template<typename TInf, const char* FuncCreate = c_createFuncDef, const char* FuncDestroy = c_destroyFuncDef>
    struct TSubModule
    {
        KLoadSubLib m_lib;
        TInfInSubModule<TInf> m_inf;

        // 构造
        TSubModule(void) : m_inf(m_lib, FuncCreate, FuncDestroy)
        {
        }

        // 接口
        TInf& Inf(void) const
        {
            if (nullptr == m_inf.m_inf) throw std::runtime_error(std::string("Nullptr - ") + typeid(TInf).name());
            return *m_inf.m_inf;
        }

        // 加载动态库
        template<typename TOwn, typename ...TArgs>
        std::string Load(TOwn& own, std::string sDll, TArgs... args)
        {
            std::string sResult = (boost::format("不能加载动态库 - %s\n") % sDll).str();
            try
            {
                if (m_lib.IsLoaded() && nullptr != m_inf.m_inf)
                    throw std::runtime_error("重复加载 - " + sDll);
                m_lib.LoadLib(sDll);
                m_inf.CreateInf(own, args...);
                sResult.clear();
            }
            catch (std::exception &ex)
            {
                sResult += std::string(" <") + typeid(ex).name() + "> " + ex.what() + " (" + __CURR_CODE_PLACE__ + ")";
                UnLoad();
            }
            catch (...)
            {
                UnLoad();
            }
            return sResult;
        }
        typedef std::shared_ptr<TSubModule<TInf, FuncCreate, FuncDestroy>> TSubModPtr;
        template<typename TOwn, typename ...TArgs>
        static TSubModPtr SLoadD(TOwn& own, std::string sDll, TArgs... args)
        {
            TSubModPtr plugin(new TSubModule<TInf, FuncCreate, FuncDestroy>);
            std::string sErr = plugin->Load(own, sDll, args...);
            if (!sErr.empty()) throw std::runtime_error(sErr);
            if (plugin.get() == nullptr || nullptr == plugin->m_inf.m_inf) throw std::runtime_error("nullptr");
            return plugin;
        }

        // 卸载动态库
        void UnLoad(void)
        {
            try
            {
                if (m_lib.IsLoaded() && nullptr != m_inf.m_inf)
                    m_inf.DestroyInf();
                m_lib.UnloadLib();
            }
            catch (...) {}
        }

        // 是否有效
        bool IsValid(void) const
        {
            return m_inf.IsValid();
        }
    };

    // 子模块（多接口）
    template<typename TInf, const char* FuncCreate = c_createFuncDef, const char* FuncDestroy = c_destroyFuncDef>
    struct TSubModuleMultiInf
    {
        KLoadSubLib m_lib;
        std::map<int, TInfInSubModulePtr<TInf>> m_infs;
        boost::shared_mutex m_mtx;

        ~TSubModuleMultiInf(void)
        {
            m_infs.clear();
        }

        // 加载动态库
        std::string Load(std::string sDll)
        {
            std::string sResult;
            sResult.clear();
            try
            {
                if (m_lib.IsLoaded())
                    throw std::runtime_error("重复加载 - " + sDll);
                m_lib.LoadLib(sDll);
            }
            catch (std::exception &ex)
            {
                sResult = (boost::format("不能加载动态库 - %s\n<%s> %s") % sDll % typeid(ex).name() % ex.what()).str();
                UnLoad();
            }
            catch (...)
            {
                sResult = (boost::format("不能加载动态库 - %s\n") % sDll).str();
                UnLoad();
            }
            return sResult;
        }
        typedef std::shared_ptr<TSubModuleMultiInf<TInf, FuncCreate, FuncDestroy>> TSubModPtr;
        static TSubModPtr SLoadD(std::string sDll)
        {
            TSubModPtr plugin(new TSubModuleMultiInf<TInf, FuncCreate, FuncDestroy>);
            std::string sErr = plugin->Load(sDll);
            if (!sErr.empty()) throw std::runtime_error(sErr);
            if (plugin.get() == nullptr) throw std::runtime_error("nullptr");
            return plugin;
        }

        // 卸载动态库
        void UnLoad(void)
        {
            try
            {
                m_lib.UnloadLib();
            }
            catch (...) {}
        }

        // 创建接口
        template<typename TOwn, typename ...TArgs>
        void MakeInf(std::function<void(TInf&)> fCreated, std::function<void(TInf&)> fModified, TOwn& own, int id, unsigned rev, TArgs... args)
        {
            try
            {
                if (!m_lib.IsLoaded()) throw std::runtime_error("动态库未加载");
                boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtx);
                auto it = m_infs.find(id);
                // 更新已存在的接口
                if (m_infs.end() != it)
                {
                    if (it->second.get() != nullptr)
                    {
                        it->second->m_revise = rev;
                        fModified(*it->second->m_inf);
                    }
                }
                // 创建新接口
                else
                {
                    TInfInSubModulePtr<TInf> ptr(new TInfInSubModule<TInf>(m_lib, FuncCreate, FuncDestroy));
                    ptr->m_revise = rev;
                    ptr->CreateInf(own, args...);
                    {
                        boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                        m_infs.insert(std::make_pair(id, ptr));
                    }
                    fCreated(*ptr->m_inf);
                }
            }
            catch (...) {}
        }

        // 删除修订号未更新的接口
        void RemoveByRevise(unsigned rev, std::function<void(TInf&)> fRmBefore)
        {
            try
            {
                // 收集修订号小于指定值的接口
                std::vector<int> vctDel;
                boost::upgrade_lock<boost::shared_mutex> lckShare(m_mtx);
                for (auto inf : m_infs)
                    if (inf.second.get() != nullptr && inf.second->m_revise < rev)
                        vctDel.push_back(inf.first);
                // 删除接口
                if (!vctDel.empty())
                {
                    boost::upgrade_to_unique_lock<boost::shared_mutex> lckUnique(lckShare);
                    for (auto id : vctDel)
                    {
                        auto it = m_infs.find(id);
                        if (m_infs.end() != it)
                        {
                            fRmBefore(*it->second->m_inf);
                            m_infs.erase(it);
                        }
                    }
                }
            }
            catch (...) {}
        }

        // 获取接口
        TInfInSubModulePtr<TInf> GetInf(int id)
        {
            TInfInSubModulePtr<TInf> ptr;
            {
                boost::shared_lock<boost::shared_mutex> lck(m_mtx);
                auto it = m_infs.find(id);
                if (m_infs.end() != it) ptr = it->second;
            }
            return ptr;
        }
    };
}
