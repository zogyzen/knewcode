#pragma once

#include "std.h"

// 封装cJSON功能
class CKCcJson : public IKCJsonCore
{
public:
    CKCcJson(const CKCcJson&) = delete;     // 删除默认的拷贝构造
    // CKCcJson(CKCcJson&&) = default;         // 默认的移动构造
    CKCcJson& operator=(const CKCcJson&) = delete;
    ~CKCcJson(void) override;

    // 是否有效
    bool IsValid(void) const override;
    // key名字
    std::string GetName(void) const override;
    // 数值类型
    IKCJson::EDataType GetType(void) const override;
    // 是否子项
    bool IsSub(void) const override;
    // 读取子项
    bool HasItem(std::string, bool caseSensitive) const override;
    unsigned GetItemCount(void) const override;
    IJsonCorePtr GetItem(unsigned) override;
    IJsonCorePtr GetItem(std::string, bool caseSensitive) override;
    IJsonCorePtr GetItemNext(void) override;
    double GetVal(std::string, double, bool caseSensitive) const override;
    std::string GetStr(std::string, std::string, bool caseSensitive) const override;
    bool IsNull(std::string, bool caseSensitive) const override;
    // 写入子项
    IJsonCorePtr AddItem(std::string, bool repeat, bool caseSensitive) override;
    IJsonCorePtr AddItem(unsigned id, bool bReplace = false) override;
    IJsonCorePtr AddArray(std::string, bool repeat, bool caseSensitive) override;
    IJsonCorePtr AddArray(unsigned id, bool bReplace = false) override;
    IJsonCorePtr AddJson(std::string, std::string, bool repeat, bool caseSensitive) override;
    IJsonCorePtr AddJson(unsigned id, std::string json, bool bReplace = false) override;
    bool AddJson(std::string, IKCJsonCore&, bool repeat, bool caseSensitive) override;
    bool AddNull(std::string) override;
    bool AddVal(std::string, double) override;
    bool AddStr(std::string, std::string) override;
    bool SetNull(std::string, bool caseSensitive) override;
    bool SetNull(unsigned id) override;
    bool SetVal(std::string, double, bool caseSensitive) override;
    bool SetVal(unsigned id, double val) override;
    bool SetStr(std::string, std::string, bool caseSensitive) override;
    bool SetStr(unsigned id, std::string str) override;
    bool SetJson(std::string, std::string, bool caseSensitive) override;
    bool SetJson(std::string, IKCJsonCore&, bool caseSensitive) override;
    bool SetJson(unsigned id, IKCJsonCore&) override;
    bool DelItem(std::string, bool caseSensitive) override;
    bool DelItem(unsigned) override;
    // 读取
    double GetVal(double override) const override;
    std::string GetStr(std::string) const override;
    bool IsNull(void) const override;
    // 写入
    bool SetVal(double) override;
    bool SetStr(std::string) override;
    // 判断
    bool EqualTo(const IKCJsonCore&, bool caseSensitive) const override;
    // 克隆成根
    IJsonCorePtr Clone2Root(void) const override;
    // 分离子项成根json
    IJsonCorePtr Detach2Root(std::string name, bool caseSensitive) override;
    IJsonCorePtr Detach2Root(const unsigned) override;
    IJsonCorePtr Detach2Root(const IKCJsonCore&) override;
    // 生成字符串
    const char* ToStr(bool fmt = false) const override;
    // 内存回收
    bool GC(void) const override;

protected:
    // 构造
    CKCcJson(cJSON&);
    // 内存回收
    bool GCImpl(void) const;

protected:
    cJSON &m_json;
    mutable char *m_print = nullptr;

    friend class CKCcJsonRoot;
    friend class CKCcJsonSub;
};
// 根项
class CKCcJsonRoot final : public CKCcJson
{
    friend class CKCcJson;

public:
    // 构造
    CKCcJsonRoot(string json);
    ~CKCcJsonRoot(void) override;

    // 是否子项
    bool IsSub(void) const override;

private:
    CKCcJsonRoot(cJSON&);
};
// 子项
class CKCcJsonSub final : public CKCcJson
{
    friend class CKCcJson;

public:
    // 是否子项
    bool IsSub(void) const override;

private:
    // 构造
    CKCcJsonSub(cJSON&);
};
