#pragma once

#include "std.h"

class CCtrlXML : public IKCController
{
public:
    CCtrlXML(IKOSFile& own, string sName, property_tree::ptree&);
    ~CCtrlXML() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

private:
    // 读取子节点列表
    string ListSub(ICtrlApiData&, map<string, string>&);
    // 读取节点信息
    string GetNode(ICtrlApiData&, map<string, string>&);
    // 删除节点
    string DelNode(ICtrlApiData&, map<string, string>&);
    // 清除节点
    string ClearNode(ICtrlApiData&, map<string, string>&);
    // 写入节点信息
    string SetNode(ICtrlApiData&, map<string, string>&);
    void SetNode(boost::property_tree::ptree &pt, const IKCJson& json);

private:
    // 宿主
    IKOSFile& m_own;
    string m_name;
    string m_type;
};
