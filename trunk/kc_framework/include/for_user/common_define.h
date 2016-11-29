#pragma once

#ifdef WIN32    // Windows环境
    #define CALL_TYPE __cdecl
#else           // linux环境
    #define CALL_TYPE
#endif

namespace KC
{
    // 数据类型
    enum EKcDataType
    {
        EKcDtVoid,         // 无类线
        EKcDtBool,         // 布尔型
        EKcDtInt,          // 整型
        EKcDtDouble,       // 浮点型
        EKcDtStr,          // 字符串型
        EKcDtInterface     // 接口类型
    };

    // 简单数据类型
    struct IKcWebDataVal
    {
        // 数据类型
        virtual EKcDataType CALL_TYPE GetDataType(void) const = 0;
        virtual void CALL_TYPE SetDataType(EKcDataType) = 0;
        // 值
        virtual bool CALL_TYPE AsBool(void) const = 0;
        virtual int CALL_TYPE AsInt(void) const = 0;
        virtual double CALL_TYPE AsDouble(void) const = 0;
        virtual const char* CALL_TYPE AsString(void) const = 0;
        virtual int CALL_TYPE Size(void) const = 0;
        virtual bool CALL_TYPE SetVal(bool) = 0;
        virtual bool CALL_TYPE SetVal(int) = 0;
        virtual bool CALL_TYPE SetVal(double) = 0;
        virtual bool CALL_TYPE SetVal(const char*) = 0;
    };

    // 函数指针
    typedef void (*TFuncPointer)(void);
    // 对象指针
    namespace UnuseWithTheNamespace{ class ObjectPointer{ virtual ~ObjectPointer(void) = default; }; }
    typedef UnuseWithTheNamespace::ObjectPointer *TObjectPointer;

    // 接口的值
    struct IKcWebInfVal
    {
        // 接口名称
        virtual const char* CALL_TYPE GetName(void) const = 0;
        virtual bool CALL_TYPE SetName(const char*) = 0;
        // 接口指针
        virtual TObjectPointer CALL_TYPE GetPointer(void) const = 0;
        virtual bool CALL_TYPE SetPointer(TObjectPointer) = 0;
    };

    // 信号捕捉接口
    struct ISignoCatch
    {
        // 信号抛出接口
        struct ISignoTry
        {
            virtual const char* CALL_TYPE Name(void) = 0;
            virtual const char* CALL_TYPE Info(void) = 0;
            virtual const char* CALL_TYPE Detail(int&) = 0;
            virtual void CALL_TYPE Try(void) = 0;
        };
        // 信号捕捉函数
        virtual void CALL_TYPE SignoCatchPoint(ISignoTry&) = 0;
        virtual int CALL_TYPE SignoCatchPointNoThrow(ISignoTry&) = 0;
    };

    // 管理锁
    struct IKcLockWork
    {
        // 互斥量
        struct IMutex
        {
            virtual unsigned int CALL_TYPE MutexID(void) const = 0;
            virtual bool CALL_TYPE Lock(const char*, const char*) = 0;
            virtual void CALL_TYPE Unlock(const char*, const char*) = 0;
        };
        virtual IMutex* CALL_TYPE NewMutex(void) = 0;
        virtual void CALL_TYPE DelMutex(IMutex*&) = 0;
    };
}
