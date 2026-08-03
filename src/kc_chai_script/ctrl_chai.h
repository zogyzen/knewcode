#pragma once

#include "std.h"
#include "chai_module.h"

class CCtrlChai : public ICtrlChai
{
public:
    CCtrlChai(IKCChaiScript&, string, property_tree::ptree&);
    ~CCtrlChai() override = default;

    // 开始
    bool start(void) override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

    // 宿主
    IKCChaiScript& Own(void) override { return m_own; }

protected:
    // 页面活动对象（用于保存脚本执行器，在整个页面执行期间有效）
    struct TChaiConnObj final : public IActionData::TActObj
    {
        chaiscript::ChaiScript m_chai;

        TChaiConnObj(CCtrlChai& /*own*/) {}
        void Release(void) override
        {
            delete this;
        }
    };

    // 执行脚本
    void ExecScript(CSqlFunc::TConnObjWork<TChaiConnObj> &chaiObj, ICtrlApiData& objCtrlD, IAttachParmForCpp&, string sScript, string sJsonParm);

private:
    // 宿主
    IKCChaiScript& m_own;
    string m_name = "default";
    // 基础模块
    ChaiBaseModule m_baseMod;
};
