#pragma once

#include "std.h"

class CCtrlUpDown : public IKCController
{
public:
    CCtrlUpDown(IKOSFile& own, string sName, property_tree::ptree&);
    ~CCtrlUpDown() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

private:
    // 上传文件
    void upfile(ICtrlApiData&, string, map<string, string>&);
    // 下载文件
    void downfile(ICtrlApiData&, string, map<string, string>&);

private:
    // 宿主
    IKOSFile& m_own;
    string m_name;
    string m_type;
};
