#include "std.h"
#include "kc_func_dyncall.h"
#include "kc_asm.h"

////////////////////////////////////////////////////////////////////////////////
// CKCFuncDyncall类
KC::CKCFuncDyncall::CKCFuncDyncall(void) : m_ActionAsm(new KC_Asm)
{
}
KC::CKCFuncDyncall::~CKCFuncDyncall(void)
{
    delete m_ActionAsm;
}

// 添加实参
void KC::CKCFuncDyncall::AddArg(EDataType dt, boost::any val)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->AddArg(dt, val);
}

// 设置返回值类型
void KC::CKCFuncDyncall::SetResult(EDataType dt)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->SetResult(dt);
}
void KC::CKCFuncDyncall::SetResult(void)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->SetResult();
}

// 设置函数指针
void KC::CKCFuncDyncall::SetFunc(KC::CKCFuncDyncall::TFuncPointer func)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->SetFunc(func);
}

// 设置成员函数指针
void KC::CKCFuncDyncall::SetMemberFunc(KC::CKCFuncDyncall::TObjectPointer thisPtr, KC::CKCFuncDyncall::TFuncPointer memFunc)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->SetMemberFunc(thisPtr, memFunc);
}

// 设置虚函数指针
void KC::CKCFuncDyncall::SetVirtualFunc(KC::CKCFuncDyncall::TObjectPointer thisPtr, int index)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->SetVirtualFunc(thisPtr, index);
}

// 调用函数
void KC::CKCFuncDyncall::CallFunc(void*& pResult, double& dResult)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->CallFunc(pResult, dResult);
}

// 清除设置
void KC::CKCFuncDyncall::Clear(void)
{
    KC_Asm* ksm = dynamic_cast<KC_Asm*>(m_ActionAsm);
    if (nullptr == ksm) throw __FUNCTION__;
    ksm->Clear();
}

