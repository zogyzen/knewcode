#include "kc_os_file.h"
#include "ctrl_updown.h"
#include "ctrl_disk.h"
#include "ctrl_xml.h"

////////////////////////////////////////////////////////////////////////////////
// CKCOSFile类

// 初始化控制器
void CKCOSFile::initAllCtrl(void)
{
    CCtrlCommon::GetAllCtrl(*this, m_pt, m_context.GetCfgFile(), m_ctrls, [&](string sName, property_tree::ptree& pt) -> IKCController*
    {
        string sType = "disk";
        if (pt.get_child_optional("<xmlattr>.type")) sType = pt.get<string>("<xmlattr>.type");
        // 上传下载文件
        if ("updown" == sType) return new CCtrlUpDown(*this, sName, pt);
        // xml配置文件
        else if ("xml" == sType) return new CCtrlXML(*this, sName, pt);
        // 磁盘文件管理
        else return new CCtrlDisk(*this, sName, pt);
    });
}
