#pragma once

namespace KC
{
    class CActionData;
    class CKCyyJson : public IKCJson
    {
    public:
        CKCyyJson(CActionData&, bool = true);
        CKCyyJson(CActionData&, cJSON&, CKCyyJson* = nullptr, bool = true);
        ~CKCyyJson(void) override;

        bool CALL_TYPE IsValid(void) override;
        const char* CALL_TYPE GetName(void) override;
        IKCJson::EDataType CALL_TYPE GetType(void) override;
        // 父项
        IKCJson* Own(void) override;
        // 子项
        bool CALL_TYPE HasItem(const char*) override;
        bool CALL_TYPE HasItem(const char*, bool caseSensitive) override;
        unsigned CALL_TYPE GetItemCount(void) override;
        IKCJson& CALL_TYPE GetItem(unsigned) override;
        IKCJson& CALL_TYPE GetItem(const char*) override;
        IKCJson& CALL_TYPE GetItem(const char*, bool caseSensitive) override;
        IKCJson& CALL_TYPE AddItem(const char*, bool = true) override;
        IKCJson& CALL_TYPE AddItem(const char*, bool repea, bool caseSensitive) override;
        bool CALL_TYPE AddJson(const char*, const char*, bool repeat = true) override;
        bool CALL_TYPE AddJson(const char*, const char*, bool repea, bool caseSensitive) override;
        IKCJson& CALL_TYPE AddArray(const char*, bool = true) override;
        IKCJson& CALL_TYPE AddArray(const char*, bool repeat, bool caseSensitive) override;
        void CALL_TYPE AddNull(const char*) override;
        void CALL_TYPE AddVal(const char*, double) override;
        void CALL_TYPE AddStr(const char*, const char*) override;
        void CALL_TYPE SetNull(const char*) override;
        void CALL_TYPE SetNull(const char*, bool caseSensitive) override;
        void CALL_TYPE SetVal(const char*, double) override;
        void CALL_TYPE SetVal(const char*, double, bool caseSensitive) override;
        void CALL_TYPE SetStr(const char*, const char*) override;
        void CALL_TYPE SetStr(const char*, const char*, bool caseSensitive) override;
        void CALL_TYPE SetJson(const char*, const char*) override;
        void CALL_TYPE SetJson(const char*, const char*, bool caseSensitive) override;
        double CALL_TYPE GetVal(const char*, double) override;
        double CALL_TYPE GetVal(const char*, double, bool caseSensitive) override;
        const char* CALL_TYPE GetStr(const char*, const char*) override;
        const char* CALL_TYPE GetStr(const char*, const char*, bool caseSensitive) override;
        bool CALL_TYPE IsNull(const char*) override;
        bool CALL_TYPE IsNull(const char*, bool caseSensitive) override;
        void CALL_TYPE DelItem(unsigned) override;
        void CALL_TYPE DelItem(const char*) override;
        void CALL_TYPE DelItem(const char*, bool caseSensitive) override;
        // 读取
        double CALL_TYPE GetVal(double = 0) override;
        const char* CALL_TYPE GetStr(const char* = "") override;
        bool CALL_TYPE IsNull(void) override;
        // 写入
        void CALL_TYPE SetVal(double) override;
        void CALL_TYPE SetStr(const char*) override;
        // 判断
        bool CALL_TYPE EqualTo(IKCJson&) const override;
        // 转换
        void CALL_TYPE AssignTo(IKCJson&) override;
        // 内存回收
        void CALL_TYPE GC(void) override;
        // 生成字符串（fmt：0有无格式，由日志等级决定；1强制无格式；2强制有格式）
        const char* ToStr(int fmt = 0) override;

    public:
        // 用字符串重置json
        bool ResetByStr(const char*);
        // 用新字符集重置json
        void ResetByCharSet(void);
        // 垃圾回收（按子项名）
        void GC(string);

    private:
        CActionData &m_act;
        CKCyyJson *m_own = nullptr;
        bool m_IsSub = true;
        string m_sJson;
        yyjson_doc *m_doc = nullptr;
        yyjson_val *m_json = nullptr;
        multimap<string, std::shared_ptr<CKCyyJson>> m_subItem;
        map<int, std::shared_ptr<CKCyyJson>> m_arrItem;
        char *m_print = nullptr;
    };
}
