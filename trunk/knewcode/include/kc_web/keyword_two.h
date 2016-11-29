#pragma once

namespace KC { namespace Keychar { namespace two_level
{
    /// 关键字
    // 接口
    constexpr char const g_KeywordInterface[] = "interface";
    // 包含其他文件
    constexpr char const g_KeywordInclude[] = "include";

    // 事件
    constexpr char const g_KeywordEvent[] = "event";
    constexpr char const g_KeywordBefore[] = "before";
    constexpr char const g_KeywordAfter[] = "after";

    // 延迟
    constexpr char const g_KeywordDelay[] = "delay";

    // 服务
    constexpr char const g_KeywordService[] = "service";

    /// 解析选项
    // 私有定义
    constexpr char const g_OptionPrivate[] = "--private--";
    // 变量引用
    constexpr char const g_OptionReference[] = "--ref--";
    // 静态变量
    constexpr char const g_OptionStatic[] = "--static--";

    /// 内部变量
    constexpr char const g_InnerCookie[] = "cookie";
    constexpr char const g_InnerSession[] = "session";
}}}
