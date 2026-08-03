#include "std.h"
#include "json_yyjson.h"
#include "action_data.h"
#include "kc_webapi_work.h"

////////////////////////////////////////////////////////////////////////////////
// CKCyyJson类
CKCyyJson::CKCyyJson(CActionData& a, bool isVld) : m_act(a), m_IsSub(false)
  , m_json([&]() -> yyjson_val* {
        yyjson_val *pJson = nullptr;
        if (isVld)
        {
           m_sJson = "{}";
           unsigned iLen = 0;
           if (!m_act.re().IsMultipartFormData()) m_sJson = m_act.re().GetPostArgStr();
           else if (m_act.re().GetMultiFormDataCount() > 0)
           {
              m_act.re().SetCurrentMultiFormData(0);
              IKCRequestRespond::IMultiFormData& mfd = m_act.re().GetMultiFormData();
              if (!mfd.isFile()) m_sJson = mfd.GetBody(iLen);
           }
           trim(m_sJson);
           if (!m_sJson.empty())
           {
              m_sJson.append(4, 0);
              m_doc = yyjson_read(m_sJson.c_str(), m_sJson.size(), YYJSON_READ_INSITU | YYJSON_READ_ALLOW_TRAILING_COMMAS | YYJSON_READ_ALLOW_COMMENTS);
              if (nullptr != m_doc)
              {
                  pJson = yyjson_doc_get_root(m_doc);
                  if (nullptr == pJson)
                  {
                      string sMsg = string("[input] ") + m_act.webapi().getHint("json_format_error_") + "  " + cJSON_GetErrorPtr() + "  ";
                      m_act.Throw(sMsg.c_str(), __CURR_CODE_PLACE_C__);
                  }
              }
           }
           else pJson = cJSON_CreateObject();
        }
        return pJson;
    }())
{
    if (this->IsValid())
    {
        // 错误信息
        if (this->GetType() != IKCJson::edtArray)
        {
            this->SetVal(c_RESTful_errCode, 0);
            this->SetStr(c_RESTful_errMsg, "");
            //m_act.SetJsonRespond(GetItem(c_RESTful_errCode));
            //m_act.SetJsonRespond(GetItem(c_RESTful_errMsg));
        }
        // 检查client编号和session编号
        m_act.CheckKCSSID();
        m_act.CheckSessinID();
        // 请求编号（原封返回）
        IKCJson& jsonRID = this->GetItem(c_RESTful_RequestID);
        if (jsonRID.IsValid()) m_act.SetJsonRespond(this->GetItem(c_RESTful_RequestID));
        // 字符集
        string sCharset = algorithm::to_upper_copy(string(GetStr(c_RESTful_Charset, c_RESTful_UTF8)));
        m_act.re().SetCharset(sCharset.c_str());
        if (this->GetType() != IKCJson::edtArray)
        {
            this->SetStr(c_RESTful_Charset, sCharset.c_str());
            m_act.SetJsonRespond(this->GetItem(c_RESTful_Charset));
        }
    }
}
CKCyyJson::CKCyyJson(CActionData& a, cJSON& j, CKCyyJson *pOwn, bool isSub) : m_act(a), m_own(pOwn), m_IsSub(isSub), m_json(&j)
{
}
CKCyyJson::~CKCyyJson(void)
{
    this->GC();
    if (!m_IsSub) cJSON_Delete(m_json);
}

bool CKCyyJson::IsValid(void)
{
    return nullptr != m_json && !cJSON_IsInvalid(m_json);
}

const char* CKCyyJson::GetName(void)
{
    return this->IsValid() && nullptr != m_json->string ? m_json->string : "";
}

IKCJson::EDataType CKCyyJson::GetType(void)
{
    if (!this->IsValid() || cJSON_IsNull(m_json)) return IKCJson::edtNull;
    else if (cJSON_IsNumber(m_json)) return IKCJson::edtNumber;
    else if (cJSON_IsArray(m_json)) return IKCJson::edtArray;
    else if (cJSON_IsObject(m_json)) return IKCJson::edtObject;
    else return IKCJson::edtString;
}

// 父项
IKCJson* CKCyyJson::Own(void)
{
    return m_own;
}

// 子项
bool CKCyyJson::HasItem(const char* name)
{
    return this->HasItem(name, m_act.m_jsonCaseSensitive);
}
bool CKCyyJson::HasItem(const char* name, bool caseSensitive)
{
    return this->IsValid() && (caseSensitive ? (cJSON_GetObjectItemCaseSensitive(m_json, name) != nullptr) : cJSON_HasObjectItem(m_json, name));
}
unsigned CKCyyJson::GetItemCount(void)
{
    return this->IsValid() ? cJSON_GetArraySize(m_json) : 0;
}
IKCJson& CKCyyJson::GetItem(unsigned id)
{
    if (this->IsValid() && id < this->GetItemCount())
    {
        auto it = m_arrItem.find(id);
        if (m_arrItem.end() != it) return *it->second;
        cJSON *pJson = cJSON_GetArrayItem(m_json, id);
        if (nullptr != pJson)
        {
            CKCyyJson *pKCJson = new CKCyyJson(m_act, *pJson, this);
            m_arrItem.insert(make_pair(id, std::shared_ptr<CKCyyJson>(pKCJson)));
            return *pKCJson;
        }
    }
    return m_act.invalidJson();
}
IKCJson& CKCyyJson::GetItem(const char* name)
{
    return GetItem(name, m_act.m_jsonCaseSensitive);
}
IKCJson& CKCyyJson::GetItem(const char* name, bool caseSensitive)
{
    if (this->IsValid() && this->HasItem(name, caseSensitive))
    {
        auto it = m_subItem.find(name);
        if (m_subItem.end() != it) return *it->second;
        //cJSON *pJson = cJSON_GetObjectItem(m_json, name);
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(m_json, name) : cJSON_GetObjectItem(m_json, name);
        if (nullptr != pJson)
        {
            CKCyyJson *pKCJson = new CKCyyJson(m_act, *pJson, this);
            m_subItem.insert(make_pair(string(name), std::shared_ptr<CKCyyJson>(pKCJson)));
            return *pKCJson;
        }
    }
    return m_act.invalidJson();
}
IKCJson& CKCyyJson::AddItem(const char* name, bool repeat)
{
    return this->AddItem(name, repeat, m_act.m_jsonCaseSensitive);
}
IKCJson& CKCyyJson::AddItem(const char* name, bool repeat, bool caseSensitive)
{
    if (this->IsValid())
    {
        if (!repeat)
        {
            IKCJson& jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.IsValid()) return jsonRes;
        }
        CKCyyJson *pKCJson = new CKCyyJson(m_act, *cJSON_AddObjectToObject(m_json, name), this);
        m_subItem.insert(make_pair(string(name), std::shared_ptr<CKCyyJson>(pKCJson)));
        return *pKCJson;
    }
    return m_act.invalidJson();
}
bool CKCyyJson::AddJson(const char* name,const char* sub, bool repeat)
{
    return this->AddJson(name, sub, repeat, m_act.m_jsonCaseSensitive);
}
bool CKCyyJson::AddJson(const char* name, const char* sub, bool repeat, bool caseSensitive)
{
    bool bResult = false;
    if (this->IsValid() && nullptr != sub)
    {
        if (!repeat)
        {
            IKCJson& jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.IsValid()) this->DelItem(name);
        }
        cJSON *pJson = cJSON_Parse(sub);
        if (nullptr == pJson) throw std::runtime_error(cJSON_GetErrorPtr());
        bResult = cJSON_AddItemToObject(m_json, name, pJson);
    }
    return bResult;
}
IKCJson& CKCyyJson::AddArray(const char* name, bool repeat)
{
    return this->AddArray(name, repeat, m_act.m_jsonCaseSensitive);
}
IKCJson& CKCyyJson::AddArray(const char* name, bool repeat, bool caseSensitive)
{
    if (this->IsValid())
    {
        if (!repeat)
        {
            IKCJson& jsonRes = this->GetItem(name, caseSensitive);
            if (jsonRes.IsValid()) return jsonRes;
        }
        CKCyyJson *pKCJson = new CKCyyJson(m_act, *cJSON_AddArrayToObject(m_json, name), this);
        m_subItem.insert(make_pair(string(name), std::shared_ptr<CKCyyJson>(pKCJson)));
        return *pKCJson;
    }
    return m_act.invalidJson();
}
void CKCyyJson::AddNull(const char* name)
{
    if (this->IsValid()) cJSON_AddNullToObject(m_json, name);
}
void CKCyyJson::AddVal(const char* name, double val)
{
    if (this->IsValid()) cJSON_AddNumberToObject(m_json, name, val);
}
void CKCyyJson::AddStr(const char* name, const char* val)
{
    if (this->IsValid())
    {
        string sVal = val;
        string sCharset = m_act.re().GetCharset();
        if (c_RESTful_GBK == sCharset) sVal = CUtilFunc::Utf8ToGbk(sVal);
        cJSON_AddStringToObject(m_json, name, sVal.c_str());
    }
}
void CKCyyJson::SetNull(const char* name)
{
    if (this->IsValid())
    {
        this->DelItem(name);
        this->AddNull(name);
    }
}
void CKCyyJson::SetNull(const char* name, bool caseSensitive)
{
    if (this->IsValid())
    {
        this->DelItem(name, caseSensitive);
        this->AddNull(name);
    }
}
void CKCyyJson::SetVal(const char* name, double val)
{
    SetVal(name, val, m_act.m_jsonCaseSensitive);
}
void CKCyyJson::SetVal(const char* name, double val, bool caseSensitive)
{
    if (this->IsValid())
    {
        //cJSON *pJson = cJSON_GetObjectItem(m_json, name);
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(m_json, name) : cJSON_GetObjectItem(m_json, name);
        if (nullptr != pJson)
        {
            if (cJSON_IsNumber(pJson)) cJSON_SetNumberValue(pJson, val);
            else if (cJSON_IsString(pJson)) cJSON_SetValuestring(pJson, std::to_string(val).c_str());
            else
            {
                this->DelItem(name);
                this->AddVal(name, val);
            }
        }
        else this->AddVal(name, val);
    }
}
void CKCyyJson::SetStr(const char* name, const char* val)
{
    SetStr(name, val, m_act.m_jsonCaseSensitive);
}
void CKCyyJson::SetStr(const char* name, const char* val, bool caseSensitive)
{
    if (this->IsValid())
    {
        //cJSON *pJson = cJSON_GetObjectItem(m_json, name);
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(m_json, name) : cJSON_GetObjectItem(m_json, name);
        if (nullptr != pJson)
        {
            if (cJSON_IsNumber(pJson)) cJSON_SetNumberValue(pJson, atof(val));
            else if (cJSON_IsString(pJson))
            {
                string sVal = val;
                string sCharset = m_act.re().GetCharset();
                if (c_RESTful_GBK == sCharset) sVal = CUtilFunc::Utf8ToGbk(sVal);
                cJSON_SetValuestring(pJson, sVal.c_str());
            }
            else
            {
                this->DelItem(name);
                this->AddStr(name, val);
            }
        }
        else this->AddStr(name, val);
    }
}
void CKCyyJson::SetJson(const char* name, const char* sub)
{
    this->SetJson(name, sub, m_act.m_jsonCaseSensitive);
}
void CKCyyJson::SetJson(const char* name, const char* sub, bool caseSensitive)
{
    if (this->IsValid())
    {
        cJSON *pJsonOld = caseSensitive ? cJSON_GetObjectItemCaseSensitive(m_json, name) : cJSON_GetObjectItem(m_json, name);
        if (nullptr != pJsonOld)
        {
            cJSON *pJsonNew = cJSON_Parse(sub);
            caseSensitive ? cJSON_ReplaceItemInObjectCaseSensitive(m_json, name, pJsonNew) : cJSON_ReplaceItemInObject(m_json, name, pJsonNew);
        }
        else AddJson(name, sub, caseSensitive);
    }
}
double CKCyyJson::GetVal(const char* name, double defVal)
{
    return GetVal(name, defVal, m_act.m_jsonCaseSensitive);
}
double CKCyyJson::GetVal(const char* name, double defVal, bool caseSensitive)
{
    if (this->IsValid())
    {
        //cJSON *pJson = cJSON_GetObjectItem(m_json, name);
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(m_json, name) : cJSON_GetObjectItem(m_json, name);
        if (nullptr != pJson)
        {
            if (cJSON_IsNull(pJson)) return defVal;
            else if (cJSON_IsNumber(pJson)) return cJSON_GetNumberValue(pJson);
            else return atof(cJSON_GetStringValue(pJson));
        }
    }
    return defVal;
}
const char* CKCyyJson::GetStr(const char* name, const char* defVal)
{
    return GetStr(name, defVal, m_act.m_jsonCaseSensitive);
}
const char* CKCyyJson::GetStr(const char* name, const char* defVal, bool caseSensitive)
{
    if (this->IsValid())
    {
        static thread_local string sResult;
        sResult.clear();
        //cJSON *pJson = cJSON_GetObjectItem(m_json, name);
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(m_json, name) : cJSON_GetObjectItem(m_json, name);
        if (nullptr != pJson)
        {
            if (cJSON_IsNull(pJson)) return defVal;
            else if (cJSON_IsNumber(pJson))
            {
                sResult = std::to_string(cJSON_GetNumberValue(pJson));
                return sResult.c_str();
            }
            else if (cJSON_IsArray(pJson) || cJSON_IsObject(pJson))
                return this->GetItem(name).ToStr(1);
            else
            {
                sResult = CUtilFunc::PCharSafeToStr(cJSON_GetStringValue(pJson));
                string sCharset = m_act.re().GetCharset();
                if (c_RESTful_GBK == sCharset) sResult = CUtilFunc::GbkToUtf8(sResult);
                return sResult.c_str();
            }
        }
    }
    return defVal;
}
bool CKCyyJson::IsNull(const char* name)
{
    return IsNull(name, m_act.m_jsonCaseSensitive);
}
bool CKCyyJson::IsNull(const char* name, bool caseSensitive)
{
    if (IsValid())
    {
        //cJSON *pJson = cJSON_GetObjectItem(m_json, name);
        cJSON *pJson = caseSensitive ? cJSON_GetObjectItemCaseSensitive(m_json, name) : cJSON_GetObjectItem(m_json, name);
        if (nullptr != pJson) return cJSON_IsNull(pJson);
    }
    return true;
}
void CKCyyJson::DelItem(unsigned id)
{
    if (IsValid() && id < GetItemCount())
    {
        auto it = m_arrItem.find(id);
        if (m_arrItem.end() != it) m_arrItem.erase(it);
        cJSON_DeleteItemFromArray(m_json, id);
    }
}
void CKCyyJson::DelItem(const char* name)
{
    this->DelItem(name, m_act.m_jsonCaseSensitive);
}
void CKCyyJson::DelItem(const char* name, bool caseSensitive)
{
    if (IsValid() && HasItem(name, caseSensitive))
    {
        auto it = m_subItem.find(name);
        if (m_subItem.end() != it) m_subItem.erase(it);
        //cJSON_DeleteItemFromObject(m_json, name);
        if (caseSensitive) cJSON_DeleteItemFromObjectCaseSensitive(m_json, name);
        else cJSON_DeleteItemFromObject(m_json, name);
        // 修复cJson里的bug（删除最后一个节点时，“child->prev”的值变成野指针）
        cJSON* pPrev = nullptr;
        for (pPrev = m_json->child; nullptr != pPrev && nullptr != pPrev->next; pPrev = pPrev->next);
        if (nullptr != pPrev) m_json->child->prev = pPrev;
    }
}

// 读取
double CKCyyJson::GetVal(double defVal)
{
    if (IsValid())
    {
        if (cJSON_IsNull(m_json)) return defVal;
        else if (cJSON_IsNumber(m_json)) return cJSON_GetNumberValue(m_json);
        else return atof(cJSON_GetStringValue(m_json));
    }
    return defVal;
}
const char* CKCyyJson::GetStr(const char* defVal)
{
    if (IsValid())
    {
        static thread_local string sResult;
        sResult.clear();
        if (cJSON_IsNull(m_json)) return defVal;
        else if (cJSON_IsArray(m_json) || cJSON_IsObject(m_json))
            return ToStr(1);
        else if (cJSON_IsNumber(m_json))
        {
            sResult = std::to_string(cJSON_GetNumberValue(m_json));
            return sResult.c_str();
        }
        else
        {
            sResult = CUtilFunc::PCharSafeToStr(cJSON_GetStringValue(m_json));
            string sCharset = m_act.re().GetCharset();
            if (c_RESTful_GBK == sCharset) sResult = CUtilFunc::GbkToUtf8(sResult);
            return sResult.c_str();
        }
    }
    return defVal;
}
bool CKCyyJson::IsNull(void)
{
    if (IsValid())
        return cJSON_IsNull(m_json);
    return true;
}

// 写入
void CKCyyJson::SetVal(double val)
{
    if (IsValid())
    {
        if (cJSON_IsNumber(m_json)) cJSON_SetNumberValue(m_json, val);
        else if (cJSON_IsString(m_json)) cJSON_SetValuestring(m_json, std::to_string(val).c_str());
        else
            throw TKCWebApiWorkException(c_ErrCode_KCWebApiWork + 4, __CURR_CODE_PLACE_C__, string(m_act.webapi().getHint("The_field_type_don_t_match_")) + m_json->string + "\n" + m_act.re().GetSingleInfo("the_request"), m_act.webapi());
    }
}
void CKCyyJson::SetStr(const char* val)
{
    if (IsValid())
    {
        if (cJSON_IsNumber(m_json)) cJSON_SetNumberValue(m_json, atof(val));
        else if (cJSON_IsString(m_json))
        {
            string sVal = val;
            string sCharset = m_act.re().GetCharset();
            if (c_RESTful_GBK == sCharset) sVal = CUtilFunc::Utf8ToGbk(sVal);
            cJSON_SetValuestring(m_json, sVal.c_str());
        }
        else
            throw TKCWebApiWorkException(c_ErrCode_KCWebApiWork + 5, __CURR_CODE_PLACE_C__, string(m_act.webapi().getHint("The_field_type_don_t_match_")) + m_json->string + "\n" + m_act.re().GetSingleInfo("the_request"), m_act.webapi());
    }
}\

// 判断
bool CKCyyJson::EqualTo(IKCJson& j) const
{
    return cJSON_Compare(this->m_json, dynamic_cast<CKCyyJson&>(j).m_json, m_act.m_jsonCaseSensitive);
}

// 转换
void CKCyyJson::AssignTo(IKCJson& j)
{
    if (nullptr != m_print) cJSON_free(m_print);
    m_print = nullptr;
    CKCyyJson &src = dynamic_cast<CKCyyJson&>(j);
    this->m_json = src.m_json;
    this->m_own = src.m_own;
    this->m_IsSub = src.m_IsSub;
    m_subItem.clear();
    m_arrItem.clear();
}

// 内存回收
void CKCyyJson::GC(void)
{
    m_subItem.clear();
    m_arrItem.clear();
    if (nullptr != m_print) cJSON_free(m_print);
    m_print = nullptr;
}
void CKCyyJson::GC(string name)
{
    auto it = m_subItem.find(name);
    if (m_subItem.end() != it) m_subItem.erase(it);
}

// 生成字符串
const char* CKCyyJson::ToStr(int fmt)
{
    if (IsValid())
    {
        if (nullptr != m_print) cJSON_free(m_print);
        m_print = 2 == fmt || (m_act.webapi().getContext().GetCfgLogLevel() <= 1 && 1 != fmt) ? cJSON_Print(m_json) : cJSON_PrintUnformatted(m_json);
        return m_print;
    }
    else return "{}";
}

// 用字符串重置json
bool CKCyyJson::ResetByStr(const char* sNewJsonStr)
{
    GC();
    if (IsValid()) m_json = cJSON_Parse(sNewJsonStr);
    return IsValid();
}

// 用新字符集重置json
void CKCyyJson::ResetByCharSet(void)
{
    if (IsValid())
    {
        if (cJSON_IsString(m_json))
        {
            string sVal = cJSON_GetStringValue(m_json);
            string sCharset = m_act.re().GetCharset();
            if (c_RESTful_GBK == sCharset) sVal = CUtilFunc::Utf8ToGbk(sVal);
            else if (c_RESTful_UTF8 == sCharset) sVal = CUtilFunc::GbkToUtf8(sVal);
            cJSON_SetValuestring(m_json, sVal.c_str());
        }
        else for (int i = 0, c = this->GetItemCount(); i < c; ++i)
            dynamic_cast<CKCyyJson&>(this->GetItem(i)).ResetByCharSet();
    }
    this->GC();
}
