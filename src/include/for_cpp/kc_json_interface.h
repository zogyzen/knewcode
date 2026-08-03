#pragma once

#include <string>
#include <memory>

#include "common/base_type.h"
#include "util/util_funcs.h"
#include "for_user/page_interface.h"

namespace KC
{
    // json功能核心
    class IKCJsonCore : public IKCObject
    {
    public:
        typedef std::shared_ptr<IKCJsonCore> IJsonCorePtr;
        // 是否有效
        virtual bool IsValid(void) const = 0;
        // key名字
        virtual std::string GetName(void) const = 0;
        // 数值类型
        virtual IKCJson::EDataType GetType(void) const = 0;
        // 是否子项
        virtual bool IsSub(void) const = 0;
        // 读取子项
        virtual bool HasItem(std::string name, bool caseSensitive) const = 0;
        virtual unsigned GetItemCount(void) const = 0;
        virtual IJsonCorePtr GetItem(unsigned) = 0;
        virtual IJsonCorePtr GetItem(std::string name, bool caseSensitive) = 0;
        virtual IJsonCorePtr GetItemNext(void) = 0;
        virtual double GetVal(std::string name, double, bool caseSensitive) const = 0;
        virtual std::string GetStr(std::string name, std::string def, bool caseSensitive) const = 0;
        virtual bool IsNull(std::string name, bool caseSensitive) const = 0;
        // 写入子项
        virtual IJsonCorePtr AddItem(std::string name, bool repeat, bool caseSensitive) = 0;
        virtual IJsonCorePtr AddItem(unsigned id, bool bReplace = false) = 0;
        virtual IJsonCorePtr AddArray(std::string name, bool repeat, bool caseSensitive) = 0;
        virtual IJsonCorePtr AddArray(unsigned id, bool bReplace = false) = 0;
        virtual IJsonCorePtr AddJson(std::string name, std::string json, bool repeat, bool caseSensitive) = 0;
        virtual IJsonCorePtr AddJson(unsigned id, std::string json, bool bReplace = false) = 0;
        virtual bool AddJson(std::string name, IKCJsonCore&, bool repeat, bool caseSensitive) = 0;
        virtual bool AddNull(std::string name) = 0;
        virtual bool AddVal(std::string name, double) = 0;
        virtual bool AddStr(std::string name, std::string str) = 0;
        virtual bool SetNull(std::string name, bool caseSensitive) = 0;
        virtual bool SetNull(unsigned id) = 0;
        virtual bool SetVal(std::string name, double val, bool caseSensitive) = 0;
        virtual bool SetVal(unsigned id, double val) = 0;
        virtual bool SetStr(std::string name, std::string str, bool caseSensitive) = 0;
        virtual bool SetStr(unsigned id, std::string str) = 0;
        virtual bool SetJson(std::string name, std::string json, bool caseSensitive) = 0;
        virtual bool SetJson(std::string name, IKCJsonCore&, bool caseSensitive) = 0;
        virtual bool SetJson(unsigned id, IKCJsonCore&) = 0;
        virtual bool DelItem(std::string name, bool caseSensitive) = 0;
        virtual bool DelItem(unsigned) = 0;
        // 读取
        virtual double GetVal(double = 0) const = 0;
        virtual std::string GetStr(std::string def) const = 0;
        virtual bool IsNull(void) const = 0;
        // 写入
        virtual bool SetVal(double) = 0;
        virtual bool SetStr(std::string str) = 0;
        // 判断
        virtual bool EqualTo(const IKCJsonCore&, bool caseSensitive = true) const = 0;
        // 克隆成根json
        virtual IJsonCorePtr Clone2Root(void) const = 0;
        // 分离子项成根json
        virtual IJsonCorePtr Detach2Root(std::string name, bool caseSensitive = true) = 0;
        virtual IJsonCorePtr Detach2Root(const unsigned) = 0;
        virtual IJsonCorePtr Detach2Root(const IKCJsonCore&) = 0;
        // 生成字符串
        virtual const char* ToStr(bool fmt = false) const = 0;
        // 内存回收
        virtual bool GC(void) const = 0;
    };
    typedef std::weak_ptr<IKCJsonCore> IJsonCoreWeakPtr;

    // 核心的参数和json
    class ICoreParmJson : public IKCObject
    {
    public:
        // 输入输出参数
        virtual IKCCtrlParmInOut& Parm(void) = 0;
        virtual const IKCCtrlParmInOut& Parm(void) const = 0;

        // Json
        virtual IKCJsonCore& Json(void) = 0;
        virtual const IKCJsonCore& Json(void) const = 0;
    };
    typedef std::shared_ptr<ICoreParmJson> ICoreParmJsonPtr;
    typedef std::weak_ptr<ICoreParmJson> ICoreParmJsonWeakPtr;

    // 回调接口
    class IJsonCallBack : public IKCObject
    {
    public:
        // 抛一个c++异常
        virtual void CALL_TYPE Throw(const char* ex, const char* place) const = 0;
        // 获取本地化提示信息
        virtual const char* CALL_TYPE GetHint(const char* key) const = 0;
        // 各单个请求信息
        virtual const char* CALL_TYPE GetSingleInfo(const char*, const char* = "") const = 0;
        // Json库
        virtual std::string CALL_TYPE JsonLibrary(void) const = 0;
        // 是否区分大小写
        virtual bool CALL_TYPE JsonCaseSensitive(void) const = 0;
        // 字符集
        virtual const char* CALL_TYPE GetCharset(void) const = 0;
        // 无效的json
        virtual IKCJson& CALL_TYPE InvalidJson(void) = 0;
        // 得到配置的日志等级
        virtual int CALL_TYPE GetCfgLogLevel(void) const = 0;
        // 获取固定字段的名称
        virtual std::string CALL_TYPE GetFixParmName(std::string) const = 0;
    };
}
