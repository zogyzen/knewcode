#pragma once

#include "std.h"

class ICtrlSession;
class CKCSessionCookie : public TKCCtrlWork<IKCSessionCookie>
{
public:
    CKCSessionCookie(const IBundle& bundle);

    // 初始化控制器
    void initAllCtrl(void) override;

    // session值
    bool CALL_TYPE IsSession(const char*) const override;
    const char* CALL_TYPE GetSessionVal(const char*, IActionData&) override;
    const char* CALL_TYPE GetSessionVal(const char*, unsigned&, IActionData&) override;
    const char* CALL_TYPE GetSessionVal(const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") override;
    const char* CALL_TYPE GetSessionVal(const char*, unsigned&, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") override;
    void CALL_TYPE SetSessionVal(const char*, const char*, IActionData&) override;
    void CALL_TYPE SetSessionVal(const char*, const char*, unsigned, IActionData&) override;
    void CALL_TYPE SetSessionVal(const char*, const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") override;
    void CALL_TYPE SetSessionVal(const char*, const char*, unsigned, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") override;
    void CALL_TYPE SetSessionExpire(const char*, const char*, IActionData&) override;
    void CALL_TYPE SetSessionExpire(const char*, const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") override;
    const char* CALL_TYPE GetSessionNextVal(const char*, const char*, const char* = c_RESTful_KCGlobalKCSSID, const char* = "") override;
    const char* CALL_TYPE GetSessionNextVal(const char*, const char*, IActionData&) override;

protected:
    // 执行session操作
    void TrySession(std::function<void(string&, string&, string&)> func);
    // 得到session控制器
    ICtrlSession* GetSSCtrl(string sName, string sKCSSID, string sKCCLNID, string &sSession);

protected:
    // 基准名
    const string m_shareMemNameBase;
    // 插件配置
    boost::property_tree::ptree m_pt;
    // 锁
    // boost::interprocess::named_mutex m_mtxShareMem;
    // 共享内存
    typedef std::shared_ptr<interprocess::managed_shared_memory> managed_shared_memory_ptr;
    managed_shared_memory_ptr m_segment;
    managed_shared_memory_ptr CreateOrOpenShareMem(void);
    // 映射文件
    typedef std::shared_ptr<interprocess::managed_mapped_file> managed_mapped_file_ptr;
    managed_mapped_file_ptr m_mapfile;
    managed_mapped_file_ptr CreateOrOpenMapFile(void);
};
