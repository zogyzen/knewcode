#include "std.h"
#include "kc_asm.h"

// 汇编函数——动态调用函数
extern "C" void __DybCallFunc(KC::CKCFuncDyncall::TObjectPointer, KC::CKCFuncDyncall::TFuncPointer,
                                       bool, KC::CKCFuncDyncall::EDataType, size_t, int, int* const, void** const);
// 通过EAX寄存器返回
typedef void* (*FDybCallFuncResultEAX)(KC::CKCFuncDyncall::TObjectPointer, KC::CKCFuncDyncall::TFuncPointer,
                                       bool, KC::CKCFuncDyncall::EDataType, size_t, int, int* const, void** const);
// 通过浮点寄存器返回
typedef double (*FDybCallFuncResultST0)(KC::CKCFuncDyncall::TObjectPointer, KC::CKCFuncDyncall::TFuncPointer,
                                       bool, KC::CKCFuncDyncall::EDataType, size_t, int, int* const, void** const);

// 实参工作缓冲区（用于汇编）
struct TArgAsmWorkBuf : public KC_Asm::IArgAsmWorkBuf
{
    TArgAsmWorkBuf(KC::KC_Asm::TArgValueList& Args) : ArgList(Args) {}
    virtual ~TArgAsmWorkBuf(void)
    {
        this->Init();
    }
    void SetArgsInfo(void)
    {
        this->Init();
        // 设置参数数量和参数类型和值得缓冲区
        ArgCount = ArgList.size();
        pDtArr = ArgCount > 0 ? new int[ArgCount] : nullptr;
        pValArr = ArgCount > 0 ? new void*[ArgCount] : nullptr;
        // 设置参数信息
        for (size_t i = 0; i < ArgCount; ++i)
        {
            // 类型
            pDtArr[i] = (int)ArgList[i].m_Type;
            // 值和栈尺寸
            switch (ArgList[i].m_Type)
            {
            case KC::CKCFuncDyncall::DtBool:
                ArgStackSize += sizeof(int);        // 布尔型做为参数时，按整型字节数传递
                pValArr[i] = &ArgList[i].m_bVal;
                break;
            case KC::CKCFuncDyncall::DtInt:
                ArgStackSize += sizeof(int);
                pValArr[i] = &ArgList[i].m_iVal;
                break;
            case KC::CKCFuncDyncall::DtDouble:
                ArgStackSize += sizeof(double);
                pValArr[i] = &ArgList[i].m_dVal;
                break;
            default:
                ArgStackSize += sizeof(void*);
                pValArr[i] = &ArgList[i].m_pVal;
                break;
            }
        }
    }
    void Init(void)
    {
        delete[] pDtArr;
        delete[] pValArr;
        pDtArr = nullptr;
        pValArr = nullptr;
        ArgStackSize = ArgCount = 0;
    }

    // 实参列表
    KC::KC_Asm::TArgValueList& ArgList;
    // 参数数量
    size_t ArgCount = 0;
    // 参数占用的栈尺寸
    int ArgStackSize = 0;
    // 参数类型的数组
    int* pDtArr = nullptr;
    // 参数值指针数组
    void** pValArr = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// KC_Asm类
KC::KC_Asm::KC_Asm(void) : m_ArgAsmWorkBuf(new TArgAsmWorkBuf(m_ArgList))
{
}
KC::KC_Asm::~KC_Asm(void)
{
    delete m_ArgAsmWorkBuf;
}

// 添加实参
void KC::KC_Asm::AddArg(CKCFuncDyncall::EDataType dt, boost::any val)
{
    TArgValue arg;
    arg.m_Type = dt;
    switch (dt)
    {
    case KC::CKCFuncDyncall::DtBool:
        arg.m_bVal = boost::any_cast<bool>(val);
        break;
    case KC::CKCFuncDyncall::DtInt:
        arg.m_iVal = boost::any_cast<int>(val);
        break;
    case KC::CKCFuncDyncall::DtDouble:
        arg.m_dVal = boost::any_cast<double>(val);
        break;
    default:
        arg.m_pVal = boost::any_cast<void*>(val);
        break;
    }
    m_ArgList.push_back(arg);
}

// 设置返回值类型
void KC::KC_Asm::SetResult(CKCFuncDyncall::EDataType dt)
{
    m_haveResult = true;
    m_ResultType = dt;
}
void KC::KC_Asm::SetResult(void)
{
    m_haveResult = false;
    m_ResultType = CKCFuncDyncall::DtBool;
}

// 设置函数指针
void KC::KC_Asm::SetFunc(KC::CKCFuncDyncall::TFuncPointer pFunc)
{
    m_ThisPointer = nullptr;
    m_FuncPointer = pFunc;
}

// 设置成员函数指针
void KC::KC_Asm::SetMemberFunc(KC::CKCFuncDyncall::TObjectPointer pThis, KC::CKCFuncDyncall::TFuncPointer pMemFunc)
{
    m_ThisPointer = pThis;
    m_FuncPointer = pMemFunc;
}

// 设置虚函数指针
void KC::KC_Asm::SetVirtualFunc(KC::CKCFuncDyncall::TObjectPointer pThis, int FuncIndex)
{
    m_ThisPointer = pThis;
    m_FuncPointer = (*(KC::CKCFuncDyncall::TFuncPointer**)pThis)[FuncIndex];
}

// 清除设置
void KC::KC_Asm::Clear(void)
{
    m_ArgList.clear();
    m_FuncPointer = nullptr;
    m_ThisPointer = nullptr;
    m_haveResult = false;
    m_ResultType = CKCFuncDyncall::DtBool;
}

// 调用函数
void KC::KC_Asm::CallFunc(void*& pResult, double& dResult)
{
    if (nullptr == m_FuncPointer) throw "Function pointers cannot be empty";
    pResult = nullptr;
    dResult = 0;
    // 实参缓冲区
    TArgAsmWorkBuf* pArgBuf = dynamic_cast<TArgAsmWorkBuf*>(m_ArgAsmWorkBuf);
    if (nullptr == pArgBuf) throw __FUNCTION__;
    // 设置实参
    pArgBuf->SetArgsInfo();
    // 调用函数
    try
    {
        if (!m_haveResult)
            // 无返回值
            __DybCallFunc(m_ThisPointer, m_FuncPointer, m_haveResult, m_ResultType,
                                pArgBuf->ArgCount, pArgBuf->ArgStackSize, pArgBuf->pDtArr, pArgBuf->pValArr);
        else if (CKCFuncDyncall::DtDouble == m_ResultType)
            // 浮点数通过浮点寄存器返回
            dResult = (FDybCallFuncResultST0(&__DybCallFunc))(m_ThisPointer, m_FuncPointer, m_haveResult, m_ResultType,
                                pArgBuf->ArgCount, pArgBuf->ArgStackSize, pArgBuf->pDtArr, pArgBuf->pValArr);
        else
            // 其余类型通过EAX寄存器返回
            pResult = (FDybCallFuncResultEAX(&__DybCallFunc))(m_ThisPointer, m_FuncPointer, m_haveResult, m_ResultType,
                                pArgBuf->ArgCount, pArgBuf->ArgStackSize, pArgBuf->pDtArr, pArgBuf->pValArr);
    }
    catch (...)
    {
        throw;
    }
}
