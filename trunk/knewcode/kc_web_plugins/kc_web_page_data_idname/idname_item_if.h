#pragma once

namespace KC
{
    // ������
    class TVarItemBase : public IVarItemBase
    {
    public:
        // ��ȡҳ������
        virtual IKCActionData& GetAction(void);

    public:
        TVarItemBase(IIDNameItemWork&, IKCActionData&, EKcDataType, string);

        // ��������
        static TIDNameItermPtr CreateVarItem(IIDNameItemWork&, IKCActionData&, EKcDataType, string, const TKcWebExpr&);

    protected:
        IIDNameItemWork& m_work;
        IKCActionData& m_act;
        // ����
        EKcDataType m_dt;
        // ������
        string m_varName;
    };

    // ��������
    class TFuncLibItemBase : public IFuncLibItemBase
    {
    public:
        // ��ȡҳ������
        virtual IKCActionData& GetAction(void);

    public:
        TFuncLibItemBase(IIDNameItemWork&, IKCActionData&, string);

        // ����������
        static TIDNameItermPtr CreateLibItem(IIDNameItemWork&, IKCActionData&, string, const TKcLoadFullFL&);

    protected:
        IIDNameItemWork& m_work;
        IKCActionData& m_act;
        // ����
        string m_libName;
    };
}
