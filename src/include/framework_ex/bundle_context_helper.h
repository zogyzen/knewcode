#pragma once

#include <string>
#include <iostream>

#include <boost/dll.hpp>

#include "util/load_library.h"
#include "framework/framework_i.h"
#include "framework_ex/bundle_context_ex_i.h"
#include "framework_ex/service_ex_i.h"

namespace KC
{
    class BundleContextHelper
    {
    public:
        BundleContextHelper(IKCStartWork& own, std::string sDir, std::string sCfg = "")
        {
            std::string sDllPath = sDir + "/kc_framework" + c_so_ext_name;
            std::cout << "*[knewcode] Load DLL kc_framework Begin: " << sDllPath << std::endl << "\t" << sCfg << std::endl;
            boost::system::error_code ec;
            m_lib.load(sDllPath, ec, boost::dll::load_mode::rtld_now);
            std::string sErrMsg = KLoadInfo::_GetLoadDllError();
            if (ec)
                sErrMsg = (boost::format("%d-%s \t%s") % ec.value() % ec.message() % sErrMsg).str();
            if (!m_lib.is_loaded())
            {
                std::cout << "*[knewcode] Load framework fail - " << sErrMsg << " - " << sDllPath << std::endl;
                throw std::runtime_error("Load framework fail - " + sDllPath + " - " + sErrMsg);
            }
            if (!m_lib.has(c_frameworFuncName))
            {
                std::cout << "*[knewcode] Load framework fail, not exists function " << c_frameworFuncName << std::endl;
                throw std::runtime_error(std::string("Load framework fail, not exists function ") + c_frameworFuncName);
            }
            auto _getFramework = m_lib.get<IFramework&(IKCStartWork&, const char*, const char*)>(c_frameworFuncName);
            IFramework& fx = _getFramework(own, sDir.c_str(), sCfg.c_str());
            m_fx = &fx;
            m_context = dynamic_cast<IBundleContextEx*>(fx.NewContext());
            std::cout << std::endl << "*[knewcode] Load DLL kc_framework Success: " << sDllPath << std::endl << "\t" << sCfg << std::endl;
        }
        ~BundleContextHelper(void)
        {
            if (m_lib.is_loaded() && nullptr != m_fx && nullptr != m_context)
                m_fx->FreeContext(m_context);
            if (m_lib.is_loaded())
                m_lib.unload();
        }

        bool isSuccess(void)
        {
            return m_lib.is_loaded() && nullptr != m_context;
        }

        IBundleContextEx& getContext(void)
        {
            if (!this->isSuccess())
            {
                cout << "*[knewcode] getContext: The framework not loaded." << endl;
                throw std::runtime_error("The framework not loaded.");
            }
            return dynamic_cast<IBundleContextEx&>(*m_context);
        }

    private:
        boost::dll::shared_library m_lib;
        IFramework* m_fx = nullptr;
        IBundleContext* m_context = nullptr;
    };
}
