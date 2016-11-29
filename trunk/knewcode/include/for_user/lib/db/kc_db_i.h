#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    class IDBTypeList;
    // 数据类型
    class IDBType
    {
    public:
        // 数据类型
        enum EDBType {dvtInt = 0, dvtBigInt, dvtDouble, dvtString, dvtDatetime};
		// 类型
		virtual EDBType CALL_TYPE DBType(void) const = 0;
        // 名称
		virtual const char* CALL_TYPE Name(void) const = 0;
        // 数据类型列表
		virtual IDBTypeList& CALL_TYPE DBTypeList(void) = 0;

    public:
        virtual CALL_TYPE ~IDBType(void) = default;
    };

    // 数据类型列表
    class IDBTypeList
    {
    public:
		// 数量
		virtual int CALL_TYPE Count(void) const = 0;
        // 存在
		virtual bool CALL_TYPE Exists(const char*) const = 0;
        // 得到数据类型
		virtual IDBType& CALL_TYPE DBType(int) = 0;
		virtual IDBType& CALL_TYPE DBType(const char*) = 0;
		// 通过名称得到序号（0基准）
		virtual int CALL_TYPE Index(const char*) const = 0;

    public:
        virtual CALL_TYPE ~IDBTypeList(void) = default;
    };
    typedef IDBTypeList IDBFieldList;

    class IDBValueList;
    // 数据值
    class IDBValue
    {
    public:
        // 数据类型
		virtual IDBType& CALL_TYPE DBType(void) = 0;
		// 值
		virtual bool CALL_TYPE IsNULL(void) const = 0;
		virtual void CALL_TYPE SetNULL(void) = 0;
		virtual int CALL_TYPE AsInt(void) const = 0;
		virtual void CALL_TYPE SetInt(int) = 0;
		virtual double CALL_TYPE AsDouble(void) const = 0;
		virtual void CALL_TYPE SetDouble(double) = 0;
		virtual const char* CALL_TYPE AsString(void) const = 0;
		virtual void CALL_TYPE SetString(const char*) = 0;
		virtual const char* CALL_TYPE AsString(const char*, const char*) const = 0;
		virtual void CALL_TYPE SetString(const char*, const char*, const char*) = 0;
		// virtual const char* CALL_TYPE AsBytes(int&) const = 0;
		// virtual void CALL_TYPE SetBytes(const char*, int) = 0;
        // 数据值列表
		virtual IDBValueList& CALL_TYPE DBValueList(void) = 0;

    public:
        virtual CALL_TYPE ~IDBValue(void) = default;
    };

    // 数据值列表
    class IDBValueList
    {
    public:
        // 数据类型列表
		virtual IDBTypeList& CALL_TYPE DBTypeList(void) = 0;
		// 得到值
		virtual IDBValue& CALL_TYPE Value(int) = 0;
		virtual IDBValue& CALL_TYPE Value(const char*) = 0;
    };

    // 数据库参数
    class IDBParms : public IDBValueList
    {
    public:
        // 添加参数
		virtual IDBValue& CALL_TYPE Add(const char*, int) = 0;
		virtual IDBValue& CALL_TYPE Add(const char*, double) = 0;
		virtual IDBValue& CALL_TYPE Add(const char*, const char*) = 0;
		virtual IDBValue& CALL_TYPE Add(const char*, const char*, const char*, const char*) = 0;
		// virtual IDBValue& CALL_TYPE Add(const char*, const char*, int) = 0;
		// 移除参数
		virtual bool CALL_TYPE Remove(int) = 0;
		virtual bool CALL_TYPE Remove(const char*) = 0;
		virtual bool CALL_TYPE Clear(void) = 0;

    public:
        virtual CALL_TYPE ~IDBParms(void) = default;
    };

    class IDBRowsBase;
    // 数据行
    class IDBRow : public IDBValueList
    {
    public:
        // 指派到参数
		virtual bool CALL_TYPE AssignTo(IDBParms&) = 0;
        // 数据表
		virtual IDBRowsBase& CALL_TYPE Table(void) = 0;

    public:
        virtual CALL_TYPE ~IDBRow(void) = default;

    protected:
        // 降低访问权限
		using IDBValueList::DBTypeList;
    };

    // 数据集基础接口(单向步进)
    class IDBRowsBase
    {
    public:
        // 名称
		virtual const char* CALL_TYPE Name(void) = 0;
		// 字段列表
		virtual IDBFieldList& CALL_TYPE Fields(void) = 0;
		// 获取当前记录行
		virtual IDBRow& CALL_TYPE Row(void) = 0;
		// 当前记录行
		virtual int CALL_TYPE RowIndex(void) const = 0;
		// 下条记录
		virtual bool CALL_TYPE Next(void) = 0;
		// 判断是否结束
		virtual bool CALL_TYPE Eof(void) = 0;
    };

    // 数据表(单向步进)
    class IDBTable : public IDBRowsBase
    {
    public:
        virtual CALL_TYPE ~IDBTable(void) = default;
    };

    // 数据集（双向随意跳转）
    class IDBSet : public IDBRowsBase
    {
    public:
        // 记录数
		virtual int CALL_TYPE RowCount(void) = 0;
		virtual bool CALL_TYPE Empty(void) = 0;
		// 获取记录行
		virtual IDBRow& CALL_TYPE Row(void) = 0;
		virtual IDBRow& CALL_TYPE Row(int) = 0;
		// 当前记录行位置
		virtual bool CALL_TYPE Prev(void) = 0;
		virtual bool CALL_TYPE Move(int) = 0;
		virtual IDBRow& CALL_TYPE First(void) = 0;
		virtual IDBRow& CALL_TYPE Last(void) = 0;

    public:
        virtual CALL_TYPE ~IDBSet(void) = default;
    };

    // 数据库活动接口
    class IDBAction
    {
    public:
        // 查询数据（占用链接）
		virtual IDBTable& CALL_TYPE Select(const char*, const char* = "TableName") = 0;
		virtual IDBTable& CALL_TYPE Select(const char*, IDBParms&, const char* = "TableName") = 0;
        // 获取值（不占用链接）
		virtual const char* CALL_TYPE GetValue(const char*) = 0;
		virtual const char* CALL_TYPE GetValue(const char*, IDBParms&) = 0;
		// 独立数据集（不占用链接，用于库级锁的SQLite）
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, const char* = "TableName") = 0;
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, IDBParms&, const char* = "TableName") = 0;
		virtual IDBSet& CALL_TYPE CreateDBSet(IDBTable&) = 0;
		virtual void CALL_TYPE ReleaseDBSet(IDBSet*&) = 0;
		// 执行语句（插入、更新、删除）
		virtual int CALL_TYPE ExecSQL(const char*) = 0;
		virtual int CALL_TYPE ExecSQL(const char*, IDBParms&) = 0;
		// 独占进程锁（其余操作为共享锁，用于库级锁的SQLite）
        virtual int CALL_TYPE ExecSQLLck(const char*) = 0;
		virtual int CALL_TYPE ExecSQLLck(const char*, IDBParms&) = 0;

    public:
        virtual CALL_TYPE ~IDBAction(void) = default;
    };

    // 数据库工作接口
    class IKCDBWork
    {
    public:
		// 创建数据库活动（参数依次为链接名称、驱动名、连接串）
		virtual IDBAction& CALL_TYPE CreateDBAction(IActionData&, const char*, const char*, const char*) = 0;
		// 创建数据库参数
		virtual IDBParms& CALL_TYPE NewParms(IActionData&) = 0;
    };
}
