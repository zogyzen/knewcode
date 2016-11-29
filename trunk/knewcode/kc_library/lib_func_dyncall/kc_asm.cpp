#include "std.h"
#include "kc_asm.h"

// ��ຯ��������̬���ú���
extern "C" void __DybCallFunc(KC::CKCFuncDyncall::TObjectPointer, KC::CKCFuncDyncall::TFuncPointer,
                                       bool, KC::CKCFuncDyncall::EDataType, size_t, int, int* const, void** const);
// ͨ��EAX�Ĵ�������
typedef void* (*FDybCallFuncResultEAX)(KC::CKCFuncDyncall::TObjectPointer, KC::CKCFuncDyncall::TFuncPointer,
                                       bool, KC::CKCFuncDyncall::EDataType, size_t, int, int* const, void** const);
// ͨ������Ĵ�������
typedef double (*FDybCallFuncResultST0)(KC::CKCFuncDyncall::TObjectPointer, KC::CKCFuncDyncall::TFuncPointer,
                                       bool, KC::CKCFuncDyncall::EDataType, size_t, int, int* const, void** const);

// ʵ�ι��������������ڻ�ࣩ
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
        // ���ò��������Ͳ������ͺ�ֵ�û�����
        ArgCount = ArgList.size();
        pDtArr = ArgCount > 0 ? new int[ArgCount] : nullptr;
        pValArr = ArgCount > 0 ? new void*[ArgCount] : nullptr;
        // ���ò�����Ϣ
        for (size_t i = 0; i < ArgCount; ++i)
        {
            // ����
            pDtArr[i] = (int)ArgList[i].m_Type;
            // ֵ��ջ�ߴ�
            switch (ArgList[i].m_Type)
            {
            case KC::CKCFuncDyncall::DtBool:
                ArgStackSize += sizeof(int);        // ��������Ϊ����ʱ���������ֽ�������
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

    // ʵ���б�
    KC::KC_Asm::TArgValueList& ArgList;
    // ��������
    size_t ArgCount = 0;
    // ����ռ�õ�ջ�ߴ�
    int ArgStackSize = 0;
    // �������͵�����
    int* pDtArr = nullptr;
    // ����ֵָ������
    void** pValArr = nullptr;
};

////////////////////////////////////////////////////////////////////////////////
// KC_Asm��
KC::KC_Asm::KC_Asm(void) : m_ArgAsmWorkBuf(new TArgAsmWorkBuf(m_ArgList))
{
}
KC::KC_Asm::~KC_Asm(void)
{
    delete m_ArgAsmWorkBuf;
}

// ���ʵ��
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

// ���÷���ֵ����
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

// ���ú���ָ��
void KC::KC_Asm::SetFunc(KC::CKCFuncDyncall::TFuncPointer pFunc)
{
    m_ThisPointer = nullptr;
    m_FuncPointer = pFunc;
}

// ���ó�Ա����ָ��
void KC::KC_Asm::SetMemberFunc(KC::CKCFuncDyncall::TObjectPointer pThis, KC::CKCFuncDyncall::TFuncPointer pMemFunc)
{
    m_ThisPointer = pThis;
    m_FuncPointer = pMemFunc;
}

// �����麯��ָ��
void KC::KC_Asm::SetVirtualFunc(KC::CKCFuncDyncall::TObjectPointer pThis, int FuncIndex)
{
    m_ThisPointer = pThis;
    m_FuncPointer = (*(KC::CKCFuncDyncall::TFuncPointer**)pThis)[FuncIndex];
}

// �������
void KC::KC_Asm::Clear(void)
{
    m_ArgList.clear();
    m_FuncPointer = nullptr;
    m_ThisPointer = nullptr;
    m_haveResult = false;
    m_ResultType = CKCFuncDyncall::DtBool;
}

// ���ú���
void KC::KC_Asm::CallFunc(void*& pResult, double& dResult)
{
    if (nullptr == m_FuncPointer) throw "Function pointers cannot be empty";
    pResult = nullptr;
    dResult = 0;
    // ʵ�λ�����
    TArgAsmWorkBuf* pArgBuf = dynamic_cast<TArgAsmWorkBuf*>(m_ArgAsmWorkBuf);
    if (nullptr == pArgBuf) throw __FUNCTION__;
    // ����ʵ��
    pArgBuf->SetArgsInfo();
    // ���ú���
    try
    {
        if (!m_haveResult)
            // �޷���ֵ
            __DybCallFunc(m_ThisPointer, m_FuncPointer, m_haveResult, m_ResultType,
                                pArgBuf->ArgCount, pArgBuf->ArgStackSize, pArgBuf->pDtArr, pArgBuf->pValArr);
        else if (CKCFuncDyncall::DtDouble == m_ResultType)
            // ������ͨ������Ĵ�������
            dResult = (FDybCallFuncResultST0(&__DybCallFunc))(m_ThisPointer, m_FuncPointer, m_haveResult, m_ResultType,
                                pArgBuf->ArgCount, pArgBuf->ArgStackSize, pArgBuf->pDtArr, pArgBuf->pValArr);
        else
            // ��������ͨ��EAX�Ĵ�������
            pResult = (FDybCallFuncResultEAX(&__DybCallFunc))(m_ThisPointer, m_FuncPointer, m_haveResult, m_ResultType,
                                pArgBuf->ArgCount, pArgBuf->ArgStackSize, pArgBuf->pDtArr, pArgBuf->pValArr);
    }
    catch (...)
    {
        throw;
    }
}
