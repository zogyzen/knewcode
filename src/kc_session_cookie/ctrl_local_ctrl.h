#pragma once

#include "std.h"
#include "ctrl_session.h"

// 转调本地控制器
class CCtrlLocalCtrl : public ICtrlSession
{
public:
    CCtrlLocalCtrl(IKCSessionCookie& own, string sName, property_tree::ptree& pt);

public:
    // 前缀名
    string PrefixName(string, string) const override;
    // 添加Session
    void SetSession(string sName, string sVal, IActionData* = nullptr) override;
    // 得到Session
    string GetSession(string, IActionData* = nullptr) override;
    // 得到下一个自增值
    string GetSessionNextVal(string, string, IActionData* = nullptr) override;
    // 删除Session
    void DelSession(string, IActionData* = nullptr) override;
    // 设置期限（d：天。h：小时。s：秒。）
    void SetExpire(string, string = "1d", IActionData* = nullptr) override;

protected:
    // 分解名称
    void SplitName(string, string&, string&, string&);
    // 获取cookie和参数
    pair<string, string> GetCookieParm(string, string = "");
    // 转换时间
    posix_time::ptime MakePTime(string = "1d");
    // 获取json值
    string GetJsonValue(json::value val);
    // 获取session值
    string GetSessionVal(string, string, IActionData*);

private:
    // 控制器名称
    string m_name;
    // 控制器
    string m_ctrl_get = "", m_ctrl_set = "", m_ctrl_del = "", m_ctrl_nextval = "";

protected:
    void GC(void) override;
};
