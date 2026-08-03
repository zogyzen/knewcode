#include "json_cjson.h"

////////////////////////////////////////////////////////////////////////////////
// CKCcJsonRoot 类
CKCcJsonRoot::CKCcJsonRoot(string json)
    : CKCcJson(*[&]() -> cJSON* {
        cJSON *pJson = boost::algorithm::trim_copy(json).empty() ? cJSON_CreateObject() : cJSON_Parse(json.c_str());
        if (nullptr == pJson)
            throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFmtErr, __CURR_CODE_PLACE_C__, string("[cJSON] ") + "json format error\n" + cJSON_GetErrorPtr() + "\n", "CKCcJsonRoot");
        return pJson;
    }())
{
}
CKCcJsonRoot::CKCcJsonRoot(cJSON& j) : CKCcJson(j)
{
}
CKCcJsonRoot::~CKCcJsonRoot(void)
{
    cJSON_Delete(&m_json);
}

// 是否子项
bool CKCcJsonRoot::IsSub(void) const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// CKCcJsonSub 类
CKCcJsonSub::CKCcJsonSub(cJSON& j) : CKCcJson(j)
{
}

// 是否子项
bool CKCcJsonSub::IsSub(void) const
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// CKCcJson 类
CKCcJson::CKCcJson(cJSON& j) : m_json(j)
{
}
CKCcJson::~CKCcJson(void)
{
    GCImpl();
}

// 是否有效
bool CKCcJson::IsValid(void) const
{
    return !cJSON_IsInvalid(&m_json);
}
// key名字
std::string CKCcJson::GetName(void) const
{
    return this->IsValid() ? CUtilFunc::PCharSafeToStr(m_json.string) : "";
}
// 数值类型
IKCJson::EDataType CKCcJson::GetType(void) const
{
    if (!this->IsValid() || cJSON_IsNull(&m_json)) return IKCJson::edtNull;
    else if (cJSON_IsNumber(&m_json)) return IKCJson::edtNumber;
    else if (cJSON_IsArray(&m_json)) return IKCJson::edtArray;
    else if (cJSON_IsObject(&m_json)) return IKCJson::edtObject;
    else return IKCJson::edtString;
}
// 是否子项
bool CKCcJson::IsSub(void) const
{
    return false;
}
// 读取子项
bool CKCcJson::HasItem(std::string name, bool caseSensitive) const
{
    return this->IsValid() && (caseSensitive ? (cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) != nullptr) : cJSON_HasObjectItem(&m_json, name.c_str()));
}
unsigned CKCcJson::GetItemCount(void) const
{
    return this->IsValid() ? cJSON_GetArraySize(&m_json) : 0;
}
IKCJsonCore::IJsonCorePtr CKCcJson::GetItem(unsigned id)
{
    IKCJsonCore::IJsonCorePtr ptr;
    if (this->IsValid() && id < this->GetItemCount())
    {
        cJSON *pJson = cJSON_GetArrayItem(&m_json, id);
        if (nullptr != pJson) ptr.reset(new CKCcJsonSub(*pJson));
    }
    return ptr;
}
IKCJsonCore::IJsonCorePtr CKCcJson::GetItem(std::string name, bool caseSensitive)
{
    IJsonCorePtr ptr;
    if (this->IsValid() && this->HasItem(name, caseSensitive))
    {
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJson) ptr.reset(new CKCcJsonSub(*pJson));
    }
    return ptr;
}
IKCJsonCore::IJsonCorePtr CKCcJson::GetItemNext(void)
{
    IJsonCorePtr ptr;
    if (this->IsValid() && nullptr != m_json.next)
        ptr.reset(new CKCcJsonSub(*m_json.next));
    return ptr;
}
double CKCcJson::GetVal(std::string name, double defVal, bool caseSensitive) const
{
    if (this->IsValid())
    {
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJson)
        {
            if (cJSON_IsNull(pJson)) return defVal;
            else if (cJSON_IsNumber(pJson)) return cJSON_GetNumberValue(pJson);
            else return atof(cJSON_GetStringValue(pJson));
        }
    }
    return defVal;
}
std::string CKCcJson::GetStr(std::string name, std::string defVal, bool caseSensitive) const
{
    if (this->IsValid())
    {
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJson)
        {
            if (cJSON_IsNull(pJson)) return defVal;
            else if (cJSON_IsNumber(pJson))
            {
                // return lexical_cast<string>(cJSON_GetNumberValue(pJson));      // 浮点数不精确
                return const_cast<CKCcJson&>(*this).GetItem(name, caseSensitive)->ToStr();
            }
            else if (cJSON_IsArray(pJson) || cJSON_IsObject(pJson))
                return const_cast<CKCcJson&>(*this).GetItem(name, caseSensitive)->ToStr();
            else
                return CUtilFunc::PCharSafeToStr(cJSON_GetStringValue(pJson));
        }
    }
    return defVal;
}
bool CKCcJson::IsNull(std::string name, bool caseSensitive) const
{
    if (IsValid())
    {
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJson)
        {
            if (cJSON_IsArray(pJson) || cJSON_IsObject(pJson))
                return cJSON_GetArraySize(pJson) == 0;
            return cJSON_IsNull(pJson);
        }
    }
    return true;
}

// 写入子项
IKCJsonCore::IJsonCorePtr CKCcJson::AddItem(std::string name, bool repeat, bool caseSensitive)
{
    IJsonCorePtr ptr;
    if (this->IsValid())
    {
        if (!repeat)
        {
            IJsonCorePtr jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.get() != nullptr && jsonRes->IsValid()) return jsonRes;
        }
        ptr.reset(new CKCcJsonSub(*cJSON_AddObjectToObject(&m_json, CUtilFunc::PCharSafeToPChar(name.c_str()))));
    }
    return ptr;
}
IKCJsonCore::IJsonCorePtr CKCcJson::AddItem(unsigned id, bool bReplace)
{
    IJsonCorePtr ptr = GetItem(id);
    CKCcJson* pKcJson = dynamic_cast<CKCcJson*>(ptr.get());
    if (this->IsValid() && (nullptr == pKcJson || (!cJSON_IsObject(&pKcJson->m_json) && !cJSON_IsArray(&pKcJson->m_json))) && cJSON_IsArray(&m_json))
    {
        cJSON *pJson = cJSON_CreateObject();
        unsigned szArr = GetItemCount();
        if (id < szArr)
        {
            if (bReplace)
                cJSON_ReplaceItemInArray(&m_json, id, pJson);
            else
                cJSON_InsertItemInArray(&m_json, id, pJson);
            GCImpl();
        }
        else
        {
            for (unsigned i = szArr; i < id; ++i) cJSON_AddItemToArray(&m_json, cJSON_CreateNull());
            cJSON_AddItemToArray(&m_json, pJson);
        }
        ptr.reset(new CKCcJsonSub(*pJson));
    }
    return ptr;
}
IKCJsonCore::IJsonCorePtr CKCcJson::AddArray(std::string name, bool repeat, bool caseSensitive)
{
    IJsonCorePtr ptr;
    if (this->IsValid())
    {
        if (!repeat)
        {
            IJsonCorePtr jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.get() != nullptr && jsonRes->IsValid()) return jsonRes;
        }
        ptr.reset(new CKCcJsonSub(*cJSON_AddArrayToObject(&m_json, CUtilFunc::PCharSafeToPChar(name.c_str()))));
    }
    return ptr;
}
IKCJsonCore::IJsonCorePtr CKCcJson::AddArray(unsigned id, bool bReplace)
{
    IJsonCorePtr ptr = GetItem(id);
    CKCcJson* pKcJson = dynamic_cast<CKCcJson*>(ptr.get());
    if (this->IsValid() && (nullptr == pKcJson || !cJSON_IsArray(&pKcJson->m_json)) && cJSON_IsArray(&m_json))
    {
        cJSON *pJson = cJSON_CreateArray();
        unsigned szArr = GetItemCount();
        if (id < szArr)
        {
            if (bReplace)
                cJSON_ReplaceItemInArray(&m_json, id, pJson);
            else
                cJSON_InsertItemInArray(&m_json, id, pJson);
            GCImpl();
        }
        else
        {
            for (unsigned i = szArr; i < id; ++i) cJSON_AddItemToArray(&m_json, cJSON_CreateNull());
            cJSON_AddItemToArray(&m_json, pJson);
        }
        ptr.reset(new CKCcJsonSub(*pJson));
    }
    return ptr;
}
IKCJsonCore::IJsonCorePtr CKCcJson::AddJson(std::string name, std::string json, bool repeat, bool caseSensitive)
{
    IJsonCorePtr ptr;
    if (this->IsValid() && !json.empty())
    {
        if (!repeat)
        {
            IJsonCorePtr jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.get() != nullptr && jsonRes->IsValid()) this->DelItem(name, caseSensitive);
        }
        cJSON *pJson = cJSON_Parse(json.c_str());
        if (nullptr == pJson)
            throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFmtErr, __CURR_CODE_PLACE_C__, string("[cJSON] ") + "json format error\n" + cJSON_GetErrorPtr() + "\n", typeid(*this).name());
        if (cJSON_AddItemToObject(&m_json, name.c_str(), pJson))
            ptr.reset(new CKCcJsonSub(*pJson));
    }
    return ptr;
}
IKCJsonCore::IJsonCorePtr CKCcJson::AddJson(unsigned id, std::string json, bool bReplace)
{
    IJsonCorePtr ptr;
    if (this->IsValid() && cJSON_IsArray(&m_json) && !json.empty())
    {
        cJSON *pJson = cJSON_Parse(json.c_str());
        unsigned szArr = GetItemCount();
        if (id < szArr)
        {
            if (bReplace)
                cJSON_ReplaceItemInArray(&m_json, id, pJson);
            else
                cJSON_InsertItemInArray(&m_json, id, pJson);
            GCImpl();
        }
        else
        {
            for (unsigned i = szArr; i < id; ++i) cJSON_AddItemToArray(&m_json, cJSON_CreateNull());
            cJSON_AddItemToArray(&m_json, pJson);
        }
        ptr.reset(new CKCcJsonSub(*pJson));
    }
    return ptr;
}
bool CKCcJson::AddJson(std::string name, IKCJsonCore& sub, bool repeat, bool caseSensitive)
{
    bool bResult = false;
    if (this->IsValid() && sub.IsValid())
    {
        if (!repeat)
        {
            IJsonCorePtr jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.get() != nullptr && jsonRes->IsValid()) this->DelItem(name, caseSensitive);
        }
        CKCcJson *pKCcJson = dynamic_cast<CKCcJson*>(&sub);
        cJSON *pJson = nullptr != pKCcJson ? &pKCcJson->m_json : cJSON_Parse(sub.GetStr("{}").c_str());
        if (nullptr == pJson)
            throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFmtErr, __CURR_CODE_PLACE_C__, string("[cJSON] ") + "json format error\n" + cJSON_GetErrorPtr() + "\n", typeid(*this).name());
        bResult = cJSON_AddItemToObject(&m_json, name.c_str(), pJson);
    }
    return bResult;
}
bool CKCcJson::AddNull(std::string name)
{
    if (this->IsValid()) return cJSON_AddNullToObject(&m_json, name.c_str()) != nullptr;
    return false;
}

bool CKCcJson::AddVal(std::string name, double val)
{
    if (this->IsValid()) return cJSON_AddNumberToObject(&m_json, name.c_str(), val) != nullptr;
    return false;
}
bool CKCcJson::AddStr(std::string name, std::string val)
{
    if (this->IsValid()) return cJSON_AddStringToObject(&m_json, name.c_str(), val.c_str()) != nullptr;
    return false;
}
bool CKCcJson::SetNull(std::string name, bool caseSensitive)
{
    if (this->IsValid())
        return this->DelItem(name, caseSensitive) && this->AddNull(name);
    return false;
}
bool CKCcJson::SetNull(unsigned id)
{
    IJsonCorePtr ptr = GetItem(id);
    CKCcJson* pKcJson = dynamic_cast<CKCcJson*>(ptr.get());
    if (this->IsValid() && (nullptr == pKcJson || !cJSON_IsNull(&pKcJson->m_json)) && cJSON_IsArray(&m_json))
    {
        cJSON *pJson = cJSON_CreateNull();
        unsigned szArr = GetItemCount();
        if (id < szArr)
            cJSON_ReplaceItemInArray(&m_json, id, pJson);
        else
        {
            for (unsigned i = szArr; i < id; ++i) cJSON_AddItemToArray(&m_json, cJSON_CreateNull());
            cJSON_AddItemToArray(&m_json, pJson);
        }
        return true;
    }
    return false;
}
bool CKCcJson::SetVal(std::string name, double val, bool caseSensitive)
{
    bool bResult = false;
    if (this->IsValid())
    {
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJson)
        {
            if (cJSON_IsNumber(pJson)) cJSON_SetNumberValue(pJson, val), bResult = true;
            // else if (cJSON_IsString(pJson)) bResult = cJSON_SetValuestring(pJson, lexical_cast<string>(val).c_str()) != nullptr;
            else
                bResult = this->DelItem(name, caseSensitive) && this->AddVal(name, val);
        }
        else bResult = this->AddVal(name, val);
    }
    return bResult;
}
bool CKCcJson::SetVal(unsigned id, double val)
{
    IJsonCorePtr ptr = GetItem(id);
    CKCcJson* pKcJson = dynamic_cast<CKCcJson*>(ptr.get());
    if (this->IsValid() && (nullptr == pKcJson || !cJSON_IsNumber(&pKcJson->m_json)) && cJSON_IsArray(&m_json))
    {
        cJSON *pJson = cJSON_CreateNumber(val);
        unsigned szArr = GetItemCount();
        if (id < szArr)
            cJSON_ReplaceItemInArray(&m_json, id, pJson);
        else
        {
            for (unsigned i = szArr; i < id; ++i) cJSON_AddItemToArray(&m_json, cJSON_CreateNull());
            cJSON_AddItemToArray(&m_json, pJson);
        }
        return true;
    }
    return false;
}
bool CKCcJson::SetStr(std::string name, std::string val, bool caseSensitive)
{
    bool bResult = false;
    if (this->IsValid())
    {
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJson)
        {
            if (cJSON_IsString(pJson)) bResult = cJSON_SetValuestring(pJson, val.c_str()) != nullptr;
            // else if (cJSON_IsNumber(pJson)) cJSON_SetNumberValue(pJson, atof(val)), bResult = true;
            else
                bResult = this->DelItem(name, caseSensitive) && this->AddStr(name, val);
        }
        else bResult = this->AddStr(name, val);
    }
    return bResult;
}
bool CKCcJson::SetStr(unsigned id, std::string str)
{
    IJsonCorePtr ptr = GetItem(id);
    CKCcJson* pKcJson = dynamic_cast<CKCcJson*>(ptr.get());
    if (this->IsValid() && (nullptr == pKcJson || !cJSON_IsString(&pKcJson->m_json)) && cJSON_IsArray(&m_json))
    {
        cJSON *pJson = cJSON_CreateString(str.c_str());
        unsigned szArr = GetItemCount();
        if (id < szArr)
            cJSON_ReplaceItemInArray(&m_json, id, pJson);
        else
        {
            for (unsigned i = szArr; i < id; ++i) cJSON_AddItemToArray(&m_json, cJSON_CreateNull());
            cJSON_AddItemToArray(&m_json, pJson);
        }
        return true;
    }
    return false;
}

bool CKCcJson::SetJson(std::string name, std::string sub, bool caseSensitive)
{
    bool bResult = false;
    if (this->IsValid())
    {
        cJSON *pJsonOld = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJsonOld)
        {
            cJSON *pJson = cJSON_Parse(sub.c_str());
            if (nullptr == pJson)
                throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFmtErr, __CURR_CODE_PLACE_C__, string("[cJSON] ") + "json format error\n" + cJSON_GetErrorPtr() + "\n", typeid(*this).name());
            bResult = caseSensitive ? cJSON_ReplaceItemInObjectCaseSensitive(&m_json, name.c_str(), pJson) : cJSON_ReplaceItemInObject(&m_json, name.c_str(), pJson);
        }
        else bResult = AddJson(name, sub, false, caseSensitive).get() != nullptr;
    }
    return bResult;
}
bool CKCcJson::SetJson(std::string name, IKCJsonCore& sub, bool caseSensitive)
{
    bool bResult = false;
    if (this->IsValid() && sub.IsValid())
    {
        cJSON *pJsonOld = caseSensitive ? cJSON_GetObjectItemCaseSensitive(&m_json, name.c_str()) : cJSON_GetObjectItem(&m_json, name.c_str());
        if (nullptr != pJsonOld)
        {
            CKCcJson *pKCcJson = dynamic_cast<CKCcJson*>(&sub);
            cJSON *pJson = nullptr != pKCcJson ? &pKCcJson->m_json : cJSON_Parse(sub.GetStr("{}").c_str());
            if (nullptr == pJson)
                throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonFmtErr, __CURR_CODE_PLACE_C__, string("[cJSON] ") + "json format error\n" + cJSON_GetErrorPtr() + "\n", typeid(*this).name());
            bResult = caseSensitive ? cJSON_ReplaceItemInObjectCaseSensitive(&m_json, name.c_str(), pJson) : cJSON_ReplaceItemInObject(&m_json, name.c_str(), pJson);
        }
        else bResult = AddJson(name, sub, false, caseSensitive);
    }
    return bResult;
}
bool CKCcJson::SetJson(unsigned id, IKCJsonCore& sub)
{
    IJsonCorePtr ptr = GetItem(id);
    if (this->IsValid() && sub.IsValid() && cJSON_IsArray(&m_json))
    {
        CKCcJson *pKCcJson = dynamic_cast<CKCcJson*>(&sub);
        cJSON *pJson = nullptr != pKCcJson ? &pKCcJson->m_json : cJSON_Parse(sub.GetStr("{}").c_str());
        unsigned szArr = GetItemCount();
        if (id < szArr)
            cJSON_ReplaceItemInArray(&m_json, id, &pKCcJson->m_json);
        else
        {
            for (unsigned i = szArr; i < id; ++i) cJSON_AddItemToArray(&m_json, cJSON_CreateNull());
            cJSON_AddItemToArray(&m_json, pJson);
        }
        return true;
    }
    return false;
}
bool CKCcJson::DelItem(std::string name, bool caseSensitive)
{
    bool bResult = this->IsValid();
    if (bResult && this->HasItem(name, caseSensitive))
    {
        if (caseSensitive) cJSON_DeleteItemFromObjectCaseSensitive(&m_json, name.c_str());
        else cJSON_DeleteItemFromObject(&m_json, name.c_str());
        // 修复cJson里的bug（删除最后一个节点时，“child->prev”的值变成野指针）
        cJSON* pPrev = nullptr;
        for (pPrev = m_json.child; nullptr != pPrev && nullptr != pPrev->next; pPrev = pPrev->next);
        if (nullptr != pPrev) m_json.child->prev = pPrev;
    }
    return bResult;
}
bool CKCcJson::DelItem(unsigned id)
{
    bool bResult = this->IsValid();
    if (bResult && id < this->GetItemCount())
        cJSON_DeleteItemFromArray(&m_json, id);
    return bResult;
}

// 读取
double CKCcJson::GetVal(double defVal) const
{
    if (this->IsValid())
    {
        if (cJSON_IsNull(&m_json)) return defVal;
        else if (cJSON_IsNumber(&m_json)) return cJSON_GetNumberValue(&m_json);
        else return atof(cJSON_GetStringValue(&m_json));
    }
    return defVal;
}
std::string CKCcJson::GetStr(std::string defVal) const
{
    if (this->IsValid())
    {
        if (cJSON_IsNull(&m_json)) return defVal;
        else if (cJSON_IsArray(&m_json) || cJSON_IsObject(&m_json))
            return this->ToStr();
        else if (cJSON_IsNumber(&m_json))
        {
            // return lexical_cast<string>(cJSON_GetNumberValue(&m_json));     // 浮点数不精确
            return this->ToStr();
        }
        else
            return CUtilFunc::PCharSafeToStr(cJSON_GetStringValue(&m_json));
    }
    return defVal;
}
bool CKCcJson::IsNull(void) const
{
    if (IsValid())
    {
        if (cJSON_IsArray(&m_json) || cJSON_IsObject(&m_json))
            return cJSON_GetArraySize(&m_json) == 0;
        return cJSON_IsNull(&m_json);
    }
    return true;
}

// 写入
bool CKCcJson::SetVal(double val)
{
    bool bResult = true;
    if (IsValid())
    {
        if (cJSON_IsNumber(&m_json)) cJSON_SetNumberValue(&m_json, val);
        else if (cJSON_IsString(&m_json)) cJSON_SetValuestring(&m_json, std::to_string(val).c_str());
        else
            throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonItemTypeErr, __CURR_CODE_PLACE_C__, string("[cJSON] ") + "The field type don't match - " + m_json.string + "\n", typeid(*this).name());
    }
    else bResult = false;
    return bResult;
}
bool CKCcJson::SetStr(std::string val)
{
    bool bResult = true;
    if (IsValid())
    {
        if (cJSON_IsNumber(&m_json)) cJSON_SetNumberValue(&m_json, atof(val.c_str()));
        else if (cJSON_IsString(&m_json)) cJSON_SetValuestring(&m_json, val.c_str());
        else
            throw TCtrlJsonException(ecd_ErrCode_KCWebApiWork_JsonItemTypeErr, __CURR_CODE_PLACE_C__, string("[cJSON] ") + "The field type don't match - " + m_json.string + "\n", typeid(*this).name());
    }
    else bResult = false;
    return bResult;
}

// 判断
bool CKCcJson::EqualTo(const IKCJsonCore& j, bool caseSensitive) const
{
    const CKCcJson* rv = dynamic_cast<const CKCcJson*>(&j);
    return nullptr != rv ? cJSON_Compare(&m_json, &rv->m_json, caseSensitive) : false;
}

// 克隆成根
IKCJsonCore::IJsonCorePtr CKCcJson::Clone2Root(void) const
{
    return IJsonCorePtr(new CKCcJsonRoot(*cJSON_Duplicate(&m_json, true)));
}
// 分离子项成根json
IKCJsonCore::IJsonCorePtr CKCcJson::Detach2Root(std::string name, bool caseSensitive)
{
    return IJsonCorePtr(new CKCcJsonRoot(caseSensitive ? *cJSON_DetachItemFromObjectCaseSensitive(&m_json, name.c_str()) :  *cJSON_DetachItemFromObject(&m_json, name.c_str())));
}
IKCJsonCore::IJsonCorePtr CKCcJson::Detach2Root(const unsigned id)
{
    return IJsonCorePtr(new CKCcJsonRoot(*cJSON_DetachItemFromArray(&m_json, id)));
}
IKCJsonCore::IJsonCorePtr CKCcJson::Detach2Root(const IKCJsonCore& sub)
{
    IKCJsonCore::IJsonCorePtr resPtr;
    const CKCcJson* pKCcJson = dynamic_cast<const CKCcJson*>(&sub);
    if (nullptr != pKCcJson)
    {
        cJSON *pJson = cJSON_DetachItemViaPointer(&m_json, &pKCcJson->m_json);
        if (nullptr != pJson) resPtr.reset(new CKCcJsonRoot(*pJson));
    }
    return resPtr;
}

// 生成字符串
const char* CKCcJson::ToStr(bool fmt) const
{
    if (IsValid())
    {
        if (nullptr != m_print) cJSON_free(m_print);
        m_print = fmt ? cJSON_Print(&m_json) : cJSON_PrintUnformatted(&m_json);
        return m_print;
    }
    else return "{}";
}

// 内存回收
bool CKCcJson::GC(void) const
{
    return GCImpl();
}
bool CKCcJson::GCImpl(void) const
{
    if (nullptr != m_print) cJSON_free(m_print);
    m_print = nullptr;
    return true;
}
