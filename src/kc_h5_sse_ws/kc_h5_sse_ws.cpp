#include "kc_h5_sse_ws.h"
#include "ctrl_sse.h"
#include "ctrl_ws.h"

////////////////////////////////////////////////////////////////////////////////
// CKCSSEWS类

// 初始化控制器
void CKCSSEWS::initAllCtrl(void)
{
    CCtrlCommon::GetAllCtrl(*this, m_pt, m_context.GetCfgFile(), m_ctrls, [&](string sName, property_tree::ptree& pt) {
        string sType = "sse";
        if (pt.get_child_optional("<xmlattr>.type")) sType = pt.get<string>("<xmlattr>.type");
        return "sse" == sType ? new CCtrlSSE(*this, sName, pt) : new CCtrlWS(*this, sName, pt);
    });
}

// 发送消息
bool CKCSSEWS::Send(const char* /*wsid*/, const char* /*type*/, const char* /*msg*/, const char* /*ctrl*/) const
{
    return false;
}
