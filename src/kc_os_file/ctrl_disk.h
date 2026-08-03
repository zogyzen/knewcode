#pragma once

#include "std.h"

class CCtrlDisk : public IKCController
{
public:
    CCtrlDisk(IKOSFile& own, string sName, property_tree::ptree&);
    ~CCtrlDisk() override;

    // 执行控制器
    void CALL_TYPE Perform(ICtrlApiData&, IKCController::IAttachParm&) override;

private:
    // 处理
    void Deal(ICtrlApiData&, map<string, string>&, std::function<string()>);
    void DealPath(ICtrlApiData&, map<string, string>&, string, std::function<bool(string, system::error_code&)>);
    // 创建目录
    void CreateDir(ICtrlApiData&, map<string, string>&);
    // 删除目录（文件）
    void Remove(ICtrlApiData&, map<string, string>&);
    // 写入文本文件
    void WriteTxt(ICtrlApiData&, map<string, string>&);
    // 读出文本文件
    void ReadTxt(ICtrlApiData&, map<string, string>&);
    // 拷贝文件（目录）
    void Copy(ICtrlApiData&, map<string, string>&);
    // 遍历目录
    void TraversalDir(ICtrlApiData&, map<string, string>&);
    void TraversalDir(ICtrlApiData&, IKCJson&, boost::filesystem::path, string extName = "*", int layer = 0, string layerDir = "");

private:
    // 宿主
    IKOSFile& m_own;
    string m_name;
    string m_type;
};
