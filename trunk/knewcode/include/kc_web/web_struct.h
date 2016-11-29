#pragma once

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <typeinfo>

#include <boost/any.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr.hpp>

#include "for_user/page_interface.h"
#include "framework/service_i.h"
#include "kc_web/keyword_one.h"
#include "kc_web/keyword_two.h"
#include "kc_web/web_exception.h"
#include "util/kc_tuple.h"

namespace KC
{
    ////////////////////////////////////通用类型///////////////////////////////////
    // apache或isapi请求和应答接口
    class IWebRequestRespond : public IBaseRequestRespond
    {
    public:
        ////////////////////////////请求部分///////////////////////////

        ////////////////////////////应答部分///////////////////////////

    protected:
        virtual CALL_TYPE ~IWebRequestRespond() = default;
    };

    // 解析用的文本缓冲区
    struct TKcWebParseTextBuffer
    {
        // 缓冲区指针
        char *first = nullptr, *last = nullptr;
        mutable char *step = nullptr;
        // 文件名
        string fileName = "";
        // 文件缓冲区大小
        int bufSize = 0;
        // 文件修改时间
        time_t fileMtime = time_t();
        // 缓冲区
        typedef boost::shared_array<char> CContentBuffer;
        CContentBuffer buffer;
        // 构造
        TKcWebParseTextBuffer(string f) : fileName(f) {}
        // 禁止拷贝
        TKcWebParseTextBuffer(const TKcWebParseTextBuffer&) = delete;
        void operator=(const TKcWebParseTextBuffer&) = delete;
    };

    // 简单数据类型
    struct TKcWebDataVal : public IKcWebDataVal
    {
        // 数据类型
        EKcDataType dataType = EKcDtVoid;
        // 简单类型
        union SimpleDBType
        {
            int i = 0;
            bool b;
            double d;
        } simData;
        // 字符串
        string strData = "";
        // 构造、拷贝
        TKcWebDataVal(EKcDataType dt = EKcDtVoid) : dataType(dt) {}
        TKcWebDataVal(const TKcWebDataVal& clone) : dataType(clone.dataType), simData(clone.simData), strData(clone.strData) {}
        TKcWebDataVal& operator=(const TKcWebDataVal& clone)
        { this->dataType = clone.dataType; this->simData = clone.simData; this->strData = clone.strData; return *this; }
        // 数据类型
        virtual EKcDataType CALL_TYPE GetDataType(void) const { return dataType; }
        virtual void CALL_TYPE SetDataType(EKcDataType dt) { dataType = dt; }
        // 值
        virtual bool CALL_TYPE AsBool(void) const
        {
            if (EKcDtBool == dataType) return simData.b;
            else if (EKcDtInt == dataType) return simData.i != 0;
            else if (EKcDtDouble == dataType) return simData.d != 0;
            else if (EKcDtStr == dataType) return strData == "true";
            else return false;
        }
        virtual int CALL_TYPE AsInt(void) const
        {
            if (EKcDtBool == dataType) return simData.b ? 1 : 0;
            else if (EKcDtInt == dataType) return simData.i;
            else if (EKcDtDouble == dataType) return (int)simData.d;
            else if (EKcDtStr == dataType) return lexical_cast<int>(strData);
            else return 0;
        }
        virtual double CALL_TYPE AsDouble(void) const
        {
            if (EKcDtBool == dataType) return simData.b ? 1 : 0;
            else if (EKcDtInt == dataType) return simData.i;
            else if (EKcDtDouble == dataType) return simData.d;
            else if (EKcDtStr == dataType) return lexical_cast<double>(strData);
            else return 0;
        }
        virtual const char* CALL_TYPE AsString(void) const
        {
            if (EKcDtBool == dataType) const_cast<TKcWebDataVal*>(this)->strData = simData.b ? "true" : "false";
            else if (EKcDtInt == dataType) const_cast<TKcWebDataVal*>(this)->strData = lexical_cast<string>(simData.i);
            else if (EKcDtDouble == dataType) const_cast<TKcWebDataVal*>(this)->strData = lexical_cast<string>(simData.d);
            return strData.c_str();
        }
        virtual int CALL_TYPE Size(void) const
        {
            if (EKcDtBool == dataType) return sizeof(bool);
            else if (EKcDtInt == dataType) return sizeof(int);
            else if (EKcDtDouble == dataType) return sizeof(double);
            else if (EKcDtStr == dataType) return (int)strData.size();
            else return 0;
        }
        virtual bool CALL_TYPE SetVal(bool b)
        {
            if (EKcDtBool == dataType) simData.b = b;
            else if (EKcDtInt == dataType) simData.i = b ? 1 : 0;
            else if (EKcDtDouble == dataType) simData.d = b ? 1 : 0;
            else if (EKcDtStr == dataType) strData = b ? "true" : "false";
            return true;
        }
        virtual bool CALL_TYPE SetVal(int i)
        {
            if (EKcDtBool == dataType) simData.b = i != 0;
            else if (EKcDtInt == dataType) simData.i = i;
            else if (EKcDtDouble == dataType) simData.d = i;
            else if (EKcDtStr == dataType) strData = lexical_cast<string>(i);
            return true;
        }
        virtual bool CALL_TYPE SetVal(double d)
        {
            if (EKcDtBool == dataType) simData.b = d != 0;
            else if (EKcDtInt == dataType) simData.i = (int)d;
            else if (EKcDtDouble == dataType) simData.d = d;
            else if (EKcDtStr == dataType) strData = lexical_cast<string>(d);
            return true;
        }
        virtual bool CALL_TYPE SetVal(const char* s)
        {
            strData = s;
            if (EKcDtBool == dataType) simData.b = strData == "true";
            else if (EKcDtInt == dataType) simData.i = lexical_cast<int>(strData);
            else if (EKcDtDouble == dataType) simData.d = lexical_cast<double>(strData);
            return true;
        }
    };

    // 接口数据类型
    struct TKcWebInfVal : public IKcWebInfVal
    {
        // 接口名称
        string name = "";
        // 接口指针
        TObjectPointer pointer = nullptr;
        // 转换为字符串
        string str(void) { return name + "[&" + lexical_cast<string>(pointer) + "]"; }
        // 构造、拷贝
        TKcWebInfVal(string n = "", TObjectPointer p = nullptr) : name(n), pointer(p) {}
        TKcWebInfVal(const TKcWebInfVal& clone) : name(clone.name), pointer(clone.pointer) {}
        TKcWebInfVal& operator=(const TKcWebInfVal& clone)
        { this->name = clone.name; this->pointer = clone.pointer; return *this; }
        // 接口名称
        virtual const char* CALL_TYPE GetName(void) const { return name.c_str(); }
        virtual bool CALL_TYPE SetName(const char* n) { name = n; return true; }
        // 接口指针
        virtual TObjectPointer CALL_TYPE GetPointer(void) const { return pointer; }
        virtual bool CALL_TYPE SetPointer(TObjectPointer ptr) { pointer = ptr; return true; }
    };

    // 表达式结构
    struct TKcWebExpr
    {
        // 表达式节点基类
        struct IExprTreeNodeBase
        {
            virtual ~IExprTreeNodeBase() = default;

        protected:
            IExprTreeNodeBase() = default;
        };
        // 表达式树节点指针
        typedef boost::shared_ptr<IExprTreeNodeBase> TExprTreeNodePtr;
        TExprTreeNodePtr exprTreeNodePtr;
        bool IsNullptr(void) const { return nullptr == exprTreeNodePtr.get(); }
        // 构造
        TKcWebExpr(void) = default;
        TKcWebExpr(IExprTreeNodeBase* tn) : exprTreeNodePtr(tn) {}
        // 拷贝
        TKcWebExpr(const TKcWebExpr& clone) : exprTreeNodePtr(clone.exprTreeNodePtr) {}
        TKcWebExpr& operator=(const TKcWebExpr& clone) { this->exprTreeNodePtr = clone.exprTreeNodePtr; return *this; }
    };

    // 任何类型的值列表
    typedef std::vector<boost::any> TAnyTypeValList;

    //////////////////////////////////语法通用类型/////////////////////////////////
    // 确定语法位置的语法属性
    struct TSynPosAttr
    {
        const char *pBegin = nullptr, *pEnd = nullptr;
        // 构造、拷贝
        TSynPosAttr(const char* f = nullptr, const char* l = nullptr) : pBegin(f), pEnd(l) {}
        TSynPosAttr(const TSynPosAttr& clone) : pBegin(clone.pBegin), pEnd(clone.pEnd) {}
        TSynPosAttr& operator=(const TSynPosAttr& clone)
        { this->pBegin = clone.pBegin; this->pEnd = clone.pEnd; return *this; }
        // 成员函数
        bool empty() const { return nullptr == pBegin; }
        bool insert(const char* p, char c) const { throw __FUNCTION__; }
        const char* end() const { throw __FUNCTION__; }
        typedef TSynPosAttr value_type;
    };
    typedef std::vector<TSynPosAttr> TSynPosAttrList;
    typedef boost::shared_ptr<TSynPosAttrList> TSynPosAttrListPtr;

    // 语法实例基类
    struct TKcSynBaseClass
    {
        // 获取语法编号
        virtual int GetID() const = 0;
        // 获取语法起始位置指针
        virtual const char* GetBeginPtr(void) const { return nullptr; }
        virtual const char* GetEndPtr(void) const { return nullptr; }
        // 构造、拷贝
        TKcSynBaseClass(void) = default;
        TKcSynBaseClass(const TKcSynBaseClass&) = default;
        virtual ~TKcSynBaseClass(void) = default;
        TKcSynBaseClass& operator=(const TKcSynBaseClass& clone) = default;
    };

    // 语法在缓冲区的位置区间
    struct TSynSectInBuf : public TKcSynBaseClass
    {
        // 语法位置属性列表
        TSynPosAttrListPtr SynPosAttrList;
        // 获取语法起始位置指针
        virtual const char* GetBeginPtr(void) const
        {
            return (SynPosAttrList->size() == 0) ? TKcSynBaseClass::GetBeginPtr() : SynPosAttrList->begin()->pBegin;
        }
        virtual const char* GetEndPtr(void) const
        {
            return (SynPosAttrList->size() == 0) ? TKcSynBaseClass::GetEndPtr() : SynPosAttrList->rbegin()->pEnd;
        }
        // 构造
        TSynSectInBuf(void) : SynPosAttrList(new TSynPosAttrList) {}
        virtual ~TSynSectInBuf(void) = default;
        // 拷贝
        TSynSectInBuf(const TSynSectInBuf& clone) : SynPosAttrList(clone.SynPosAttrList) {}
        TSynSectInBuf& operator=(const TSynSectInBuf& clone) { this->SynPosAttrList = clone.SynPosAttrList; return *this; }
    };

    ////////////////////////////////////通用语法实例类///////////////////////////////////
    // 列表语法实例
    template<typename TVal, int TypeID = 0, typename TBaseClass = TKcSynBaseClass>
    struct TKcValListSynStructModel : public TBaseClass
    {
        // 获取编号
        virtual int GetID() const { return TypeID; }
        // 值列表
        typedef std::vector<TVal> TValList;
        typedef boost::shared_ptr<TValList> TValListPtr;
        TValListPtr ValList;
        void PushVal(const TVal& v) { ValList->push_back(v); }
        int Count() { return (int)ValList->size(); }
        // 构造
        TKcValListSynStructModel() : TBaseClass(), ValList(new TValList) {}
        virtual ~TKcValListSynStructModel(void) = default;
        // 拷贝
        TKcValListSynStructModel(const TKcValListSynStructModel& clone) : TBaseClass(clone), ValList(clone.ValList) {}
        TKcValListSynStructModel& operator=(const TKcValListSynStructModel& clone)
        { TBaseClass::operator=(clone); this->ValList = clone.ValList; return *this; }
    };
    // 带位置
    template<typename TVal, int TypeID = 0>
    using TKcValListSynStructWithPos = TKcValListSynStructModel<TVal, TypeID, TSynSectInBuf>;
    // 带位置
    template<typename TVal, int TypeID = 0>
    using TKcValListSynStruct = TKcValListSynStructModel<TVal, TypeID, TKcSynBaseClass>;

    // 运算符语法实例
    template<int SynID = 0>
    struct TKcSynExprOpr : public TKcSynBaseClass, public TSynPosAttr
    {
        // 构造、拷贝
        TKcSynExprOpr(const char* f = nullptr, const char* l = nullptr) : TKcSynBaseClass(), TSynPosAttr(f, l) {}
        TKcSynExprOpr(const TKcSynExprOpr& clone) : TKcSynBaseClass(clone), TSynPosAttr(clone) {}
        TKcSynExprOpr& operator=(const TKcSynExprOpr& clone)
        { TKcSynBaseClass::operator=(clone); TSynPosAttr::operator=(clone); return *this; }
        TKcSynExprOpr& operator=(const TSynPosAttr& clone)
        { TSynPosAttr::operator=(clone); return *this; }
        // 获取语法编号
        virtual int GetID() const { return SynID; }
        // 获取语法起始位置指针
        virtual const char* GetBeginPtr(void) const { return pBegin; }
        virtual const char* GetEndPtr(void) const { return pEnd; }
    };

    // 非顺序语句的条件栈
    struct TConditionItem
    {
        string keychar;
        bool cond = false;

        // 构造、拷贝
        TConditionItem(string k = "", bool c = false) : keychar(k), cond(c) {}
        TConditionItem(const TConditionItem& c) : keychar(c.keychar), cond(c.cond) {}
        TConditionItem& operator=(const TConditionItem& c)
        { this->keychar = c.keychar; this->cond = c.cond; return *this; }
    };
    typedef vector<TConditionItem> TConditionStatck;
}
