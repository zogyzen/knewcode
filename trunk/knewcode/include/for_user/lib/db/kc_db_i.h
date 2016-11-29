#pragma once

#include "for_user/page_interface.h"

namespace KC
{
    class IDBTypeList;
    // ��������
    class IDBType
    {
    public:
        // ��������
        enum EDBType {dvtInt = 0, dvtBigInt, dvtDouble, dvtString, dvtDatetime};
		// ����
		virtual EDBType CALL_TYPE DBType(void) const = 0;
        // ����
		virtual const char* CALL_TYPE Name(void) const = 0;
        // ���������б�
		virtual IDBTypeList& CALL_TYPE DBTypeList(void) = 0;

    public:
        virtual CALL_TYPE ~IDBType(void) = default;
    };

    // ���������б�
    class IDBTypeList
    {
    public:
		// ����
		virtual int CALL_TYPE Count(void) const = 0;
        // ����
		virtual bool CALL_TYPE Exists(const char*) const = 0;
        // �õ���������
		virtual IDBType& CALL_TYPE DBType(int) = 0;
		virtual IDBType& CALL_TYPE DBType(const char*) = 0;
		// ͨ�����Ƶõ���ţ�0��׼��
		virtual int CALL_TYPE Index(const char*) const = 0;

    public:
        virtual CALL_TYPE ~IDBTypeList(void) = default;
    };
    typedef IDBTypeList IDBFieldList;

    class IDBValueList;
    // ����ֵ
    class IDBValue
    {
    public:
        // ��������
		virtual IDBType& CALL_TYPE DBType(void) = 0;
		// ֵ
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
        // ����ֵ�б�
		virtual IDBValueList& CALL_TYPE DBValueList(void) = 0;

    public:
        virtual CALL_TYPE ~IDBValue(void) = default;
    };

    // ����ֵ�б�
    class IDBValueList
    {
    public:
        // ���������б�
		virtual IDBTypeList& CALL_TYPE DBTypeList(void) = 0;
		// �õ�ֵ
		virtual IDBValue& CALL_TYPE Value(int) = 0;
		virtual IDBValue& CALL_TYPE Value(const char*) = 0;
    };

    // ���ݿ����
    class IDBParms : public IDBValueList
    {
    public:
        // ��Ӳ���
		virtual IDBValue& CALL_TYPE Add(const char*, int) = 0;
		virtual IDBValue& CALL_TYPE Add(const char*, double) = 0;
		virtual IDBValue& CALL_TYPE Add(const char*, const char*) = 0;
		virtual IDBValue& CALL_TYPE Add(const char*, const char*, const char*, const char*) = 0;
		// virtual IDBValue& CALL_TYPE Add(const char*, const char*, int) = 0;
		// �Ƴ�����
		virtual bool CALL_TYPE Remove(int) = 0;
		virtual bool CALL_TYPE Remove(const char*) = 0;
		virtual bool CALL_TYPE Clear(void) = 0;

    public:
        virtual CALL_TYPE ~IDBParms(void) = default;
    };

    class IDBRowsBase;
    // ������
    class IDBRow : public IDBValueList
    {
    public:
        // ָ�ɵ�����
		virtual bool CALL_TYPE AssignTo(IDBParms&) = 0;
        // ���ݱ�
		virtual IDBRowsBase& CALL_TYPE Table(void) = 0;

    public:
        virtual CALL_TYPE ~IDBRow(void) = default;

    protected:
        // ���ͷ���Ȩ��
		using IDBValueList::DBTypeList;
    };

    // ���ݼ������ӿ�(���򲽽�)
    class IDBRowsBase
    {
    public:
        // ����
		virtual const char* CALL_TYPE Name(void) = 0;
		// �ֶ��б�
		virtual IDBFieldList& CALL_TYPE Fields(void) = 0;
		// ��ȡ��ǰ��¼��
		virtual IDBRow& CALL_TYPE Row(void) = 0;
		// ��ǰ��¼��
		virtual int CALL_TYPE RowIndex(void) const = 0;
		// ������¼
		virtual bool CALL_TYPE Next(void) = 0;
		// �ж��Ƿ����
		virtual bool CALL_TYPE Eof(void) = 0;
    };

    // ���ݱ�(���򲽽�)
    class IDBTable : public IDBRowsBase
    {
    public:
        virtual CALL_TYPE ~IDBTable(void) = default;
    };

    // ���ݼ���˫��������ת��
    class IDBSet : public IDBRowsBase
    {
    public:
        // ��¼��
		virtual int CALL_TYPE RowCount(void) = 0;
		virtual bool CALL_TYPE Empty(void) = 0;
		// ��ȡ��¼��
		virtual IDBRow& CALL_TYPE Row(void) = 0;
		virtual IDBRow& CALL_TYPE Row(int) = 0;
		// ��ǰ��¼��λ��
		virtual bool CALL_TYPE Prev(void) = 0;
		virtual bool CALL_TYPE Move(int) = 0;
		virtual IDBRow& CALL_TYPE First(void) = 0;
		virtual IDBRow& CALL_TYPE Last(void) = 0;

    public:
        virtual CALL_TYPE ~IDBSet(void) = default;
    };

    // ���ݿ��ӿ�
    class IDBAction
    {
    public:
        // ��ѯ���ݣ�ռ�����ӣ�
		virtual IDBTable& CALL_TYPE Select(const char*, const char* = "TableName") = 0;
		virtual IDBTable& CALL_TYPE Select(const char*, IDBParms&, const char* = "TableName") = 0;
        // ��ȡֵ����ռ�����ӣ�
		virtual const char* CALL_TYPE GetValue(const char*) = 0;
		virtual const char* CALL_TYPE GetValue(const char*, IDBParms&) = 0;
		// �������ݼ�����ռ�����ӣ����ڿ⼶����SQLite��
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, const char* = "TableName") = 0;
		virtual IDBSet& CALL_TYPE SelectToSet(const char*, IDBParms&, const char* = "TableName") = 0;
		virtual IDBSet& CALL_TYPE CreateDBSet(IDBTable&) = 0;
		virtual void CALL_TYPE ReleaseDBSet(IDBSet*&) = 0;
		// ִ����䣨���롢���¡�ɾ����
		virtual int CALL_TYPE ExecSQL(const char*) = 0;
		virtual int CALL_TYPE ExecSQL(const char*, IDBParms&) = 0;
		// ��ռ���������������Ϊ�����������ڿ⼶����SQLite��
        virtual int CALL_TYPE ExecSQLLck(const char*) = 0;
		virtual int CALL_TYPE ExecSQLLck(const char*, IDBParms&) = 0;

    public:
        virtual CALL_TYPE ~IDBAction(void) = default;
    };

    // ���ݿ⹤���ӿ�
    class IKCDBWork
    {
    public:
		// �������ݿ�����������Ϊ�������ơ������������Ӵ���
		virtual IDBAction& CALL_TYPE CreateDBAction(IActionData&, const char*, const char*, const char*) = 0;
		// �������ݿ����
		virtual IDBParms& CALL_TYPE NewParms(IActionData&) = 0;
    };
}
