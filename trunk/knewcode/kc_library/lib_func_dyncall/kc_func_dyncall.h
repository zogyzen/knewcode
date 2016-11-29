#pragma once

#include <boost/any.hpp>

namespace KC
{
    // ��̬���ú�������
    class CKCFuncDyncall
    {
    private:
        class ObjectPointer{ virtual ~ObjectPointer(void) = default; };

    public:
        // ��������
        enum EDataType
        {
            DtPointer   = 0,    // ָ������
            DtBool      = 1,    // ������
            DtInt       = 2,    // ����
            DtDouble    = 3,    // ������
            DtInterface = 4     // �ӿ���
        };
        // ����ָ��
        typedef void (*TFuncPointer)(void);
        // ����ָ��
        typedef ObjectPointer *TObjectPointer;

    public:
        CKCFuncDyncall(void);
        ~CKCFuncDyncall(void);

        // ���ʵ��
        void AddArg(EDataType, boost::any);
        // ���÷���ֵ����
        void SetResult(EDataType);
        void SetResult(void);
        // ���ú���ָ��
        void SetFunc(TFuncPointer);
        // ���ó�Ա����ָ��
        void SetMemberFunc(TObjectPointer, TFuncPointer);
        // �����麯��ָ��
        void SetVirtualFunc(TObjectPointer, int);
        // ���ú���
        void CallFunc(void*&, double&);
        // �������
        void Clear(void);

    public:
        // ʵ�ʴ���ӿ�
        class IActionAsm
        {
        public:
            virtual ~IActionAsm(void) = default;
        };

    private:
        // ʵ�ʴ���ӿ�
        IActionAsm* m_ActionAsm = nullptr;
    };
}
