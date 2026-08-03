#pragma once

#include <string>
#include <iostream>

#include <boost/dll.hpp>

#include "util/load_library.h"
#include "kc_web/kc_websrv_proxy_i.h"

namespace KC
{
    class LoadWebSrvProxy
    {
    public:
        LoadWebSrvProxy(IWSProxyServerCB& srvCB)
        {
            std::string sDllPath = srvCB.fxPath() + std::string("/kc_websrv_proxy") + c_so_ext_name;
            std::cout << "*[knewcode] Load Webserve Proxy Begin - " << sDllPath << std::endl;
            boost::system::error_code ec;
            m_lib.load(sDllPath, ec, boost::dll::load_mode::rtld_now);
            std::string sErrMsg = KLoadInfo::_GetLoadDllError();
            if (ec)
                sErrMsg = (boost::format("%d-%s \t%s") % ec.value() % ec.message() % sErrMsg).str();
            if (!m_lib.is_loaded())
            {
                std::cout << "*[knewcode] Load Webserve Proxy Fail - " << sDllPath << " - " << sErrMsg << std::endl;
                throw std::runtime_error("Load Webserve Proxy Fail - " + sDllPath + " - " + sErrMsg);
            }
            if (!m_lib.has(c_proxyMakeFuncName))
            {
                std::cout << "Load Webserve Proxy Fail, not exists function " << c_proxyMakeFuncName << std::endl;
                throw std::runtime_error(std::string("Load Webserve Proxy Fail, not exists function ") + c_proxyMakeFuncName);
            }
            auto _getProxy = m_lib.get<IKCWebSrvProxy&(IWSProxyServerCB&)>(c_proxyMakeFuncName);
            m_proxy = &_getProxy(srvCB);
            std::cout << "*[knewcode] Load Webserve Proxy Success - " << sDllPath << std::endl;
        }
        ~LoadWebSrvProxy(void)
        {
            try
            {
                m_proxy = nullptr;
                if (m_lib.is_loaded())
                {
                    if (m_lib.has(c_proxyReleaseFuncName))
                        m_lib.get<void(void)>(c_proxyReleaseFuncName)();
                    m_lib.unload();
                }
            }
            catch (...) {}
        }

        bool isSuccess(void)
        {
            return m_lib.is_loaded() && nullptr != m_proxy;
        }

        IKCWebSrvProxy& Proxy(void)
        {
            if (nullptr == m_proxy)
            {
                std::cout << "*[knewcode] Get Webserve Proxy Fail - nullptr." << std::endl;
                throw std::runtime_error("Get Webserve Proxy Fail - nullptr");
            }
            else std::cout << "*[knewcode] Get Webserve Proxy." << std::endl;
            return *m_proxy;
        }

    private:
        boost::dll::shared_library m_lib;
        IKCWebSrvProxy* m_proxy = nullptr;
    };
}
