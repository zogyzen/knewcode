#pragma once

#include "kc_func_dyncall.h"

namespace KC
{
    // ������Դ������
    class KC_Asm : public CKCFuncDyncall::IActionAsm
    {
    public:
        KC_Asm(void);
        virtual ~KC_Asm(void);

        // ���ʵ��
        void AddArg(CKCFuncDyncall::EDataType, boost::any);
        // ���÷���ֵ����
        void SetResult(CKCFuncDyncall::EDataType);
        void SetResult(void);
        // ���ú���ָ��
        void SetFunc(CKCFuncDyncall::TFuncPointer);
        // ���ó�Ա����ָ��
        void SetMemberFunc(CKCFuncDyncall::TObjectPointer, CKCFuncDyncall::TFuncPointer);
        // �����麯��ָ��
        void SetVirtualFunc(CKCFuncDyncall::TObjectPointer, int);
        // ���ú���
        void CallFunc(void*&, double&);
        // �������
        void Clear(void);

    public:
        // ʵ���б�
        struct TArgValue
        {
            CKCFuncDyncall::EDataType m_Type = CKCFuncDyncall::DtBool;
            union
            {
                bool m_bVal;
                int m_iVal;
                double m_dVal;
                void* m_pVal;
            };
        };
        typedef vector<TArgValue> TArgValueList;

    public:
        // ʵ�λ����������ڻ�ࣩ
        struct IArgAsmWorkBuf
        {
            virtual ~IArgAsmWorkBuf(void) = default;
        };

    private:
        // ����ָ��
        CKCFuncDyncall::TFuncPointer m_FuncPointer = nullptr;
        // ��ʵ��ָ��
        CKCFuncDyncall::TObjectPointer m_ThisPointer = nullptr;
        // ����ֵ����
        bool m_haveResult = false;
        CKCFuncDyncall::EDataType m_ResultType = CKCFuncDyncall::DtBool;
        // ʵ���б�
        TArgValueList m_ArgList;
        IArgAsmWorkBuf* m_ArgAsmWorkBuf = nullptr;
    };
}
