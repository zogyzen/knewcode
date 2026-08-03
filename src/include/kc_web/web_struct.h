#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <typeinfo>

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

#include "kc_web/kc_request_respond.h"
#include "framework/service_i.h"
#include "kc_web/web_exception.h"
#include "util/kc_tuple.h"

namespace KC
{
    ////////////////////////////////////通用类型///////////////////////////////////
    // 源请求和应答接口（来自于apache、nginx、其他程序调用，等）
    class ISrcRequestRespond : public IBaseRequestRespond
    {
    public:
        // 是否子调用
        virtual bool IsSubCall(void) = 0;

        ////////////////////////////请求部分///////////////////////////

        ////////////////////////////应答部分///////////////////////////

    protected:
        ~ISrcRequestRespond() override = default;
    };
	
    // 过程中的请求和应答处理数据
    class IKCRequestRespondData : public IKCRequestRespond
    {
    public:
        virtual ISrcRequestRespond& getRe(void) const = 0;

    protected:
        ~IKCRequestRespondData() override = default;
    };

    // 控制器请求的附加参数（for C++）
    struct IAttachParmForCpp : IKCController::IAttachParm
    {
        virtual const boost::property_tree::ptree& GetCtrlApiNode(void) const = 0;
        virtual const boost::property_tree::ptree& GetKCFilePt(void) const = 0;
    };
    template<typename TCtrl>
    struct TAttachParmForCpp : IAttachParmForCpp
    {
        TCtrl &m_ctrlD;
        TAttachParmForCpp(TCtrl &ctrlD) : m_ctrlD(ctrlD)
        {
        }

        // 传递数据
        bool GetBool(const char*) const override { return true; }
        int GetInt(const char*) const override { return 0; }
        double GetDouble(const char*) const override { return 0; }
        const char* GetStr(const char*) const override { return ""; }
        const char* GetBuf(const char*, unsigned& len) const override { len = 0; return ""; }
        void Set(const char*, bool) override {}
        void Set(const char*, int) override {}
        void Set(const char*, double) override {}
        void Set(const char*, const char*) override {}
        void Set(const char*, const char*, unsigned /*len*/) override {}

        // 返回控制器api节点xml
        const boost::property_tree::ptree& GetCtrlApiNode(void) const override
        {
            return m_ctrlD.PtCtrlApi();
        }
        // 返回控制器所在的kc文件xml
        const boost::property_tree::ptree& GetKCFilePt(void) const override
        {
            return m_ctrlD.Pt();
        }
    };
}
