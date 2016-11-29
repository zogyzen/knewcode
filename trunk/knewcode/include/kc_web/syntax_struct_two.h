#pragma once

#include "kc_web/syntax_struct_one.h"
#include "util/load_library.h"

namespace KC
{
    ////////////////////////////////////�����﷨///////////////////////////////////
    // �����﷨��ϵ���
    const int c_SynSysDefID = 40000;

    // ������ֵ
    const int c_SynSysDefID_VarAss = c_SynSysDefID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_VarAss, TKcVarSyn, TKcWebExpr> TKcVarAssDef;

    // �ڲ�������ֵ
    const int c_SynSysDefID_InnerVarAss = c_SynSysDefID + 102;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_InnerVarAss, TKcInnerVarSyn, TKcWebExpr> TKcInnerVarAssDef;

    // ������ֵ�б�
    const int c_SynSysDefID_VarAssList = c_SynSysDefID + 103;
    typedef TKcValListSynStructWithPos<TKcVarAssDef, c_SynSysDefID_VarAssList> TKcVarAssListDef;

    // ��������
    const int c_SynSysDefID_Var = c_SynSysDefID + 104;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_Var, TKcDataTypeSyn, TKcVarAssListDef, bool, bool, bool> TKcVarDef;

    // ���ʽ����
    const int c_SynSysDefID_ExprWork = c_SynSysDefID + 105;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_ExprWork, TKcWebExpr> TKcExprWorkDef;

    // �Զ���������
    const int c_SynSysDefID_TypeName = c_SynSysDefID + 106;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_TypeName, string> TKcTypeNameDef;

    // �¼�λ��
    const int c_SynSysDefID_EventOpPlace = c_SynSysDefID + 107;
    enum EEventOpPlace
    {
        OpPlaceBefore = 0,      // �ڲ���ǰ
        OpPlaceAfter            // �ڲ�����
    };
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_EventOpPlace, EEventOpPlace> TKcEventOpPlaceDef;

    // �ӳٶ���
    const int c_SynSysDefID_Delay = c_SynSysDefID + 108;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_Delay, TKcTypeNameDef, int, TKcVarAssDef, TKcInnerVarAssDef, TKcExprWorkDef> TKcDelayDef;

    // �¼��ȼ�
    const int c_SynSysDefID_EventLevel = c_SynSysDefID + 109;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_EventLevel, int> TKcEventLevelDef;

    // �¼�����
    const int c_SynSysDefID_EventBody = c_SynSysDefID + 110;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_EventBody, TKcIDNameSyn, TKcMemberSyn, TKcVarAssDef, TKcInnerVarAssDef, TKcExprWorkDef> TKcEventBodyDef;

    // �¼�����
    const int c_SynSysDefID_Event = c_SynSysDefID + 111;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_Event, TKcEventOpPlaceDef, TKcTypeNameDef, TKcEventLevelDef, TKcEventBodyDef> TKcEventDef;

    ////////////////////////////////////�������﷨///////////////////////////////////
    // �����﷨��ϵ���
    const int c_SynSysLibID = 50000;

    // �β�
    const int c_SynSysLibID_Parm = c_SynSysLibID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_Parm, TKcDataTypeSyn, TKcTypeNameDef, bool> TKcParmFL;

    // �β��б�
    const int c_SynSysLibID_Parms = c_SynSysLibID + 102;
    typedef TKcValListSynStructWithPos<TKcParmFL, c_SynSysLibID_Parms> TKcParmsFL;

    // ��������
    const int c_SynSysLibID_FuncDef = c_SynSysLibID + 103;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_FuncDef, TKcParmFL, string, TKcParmsFL, int> TKcFuncDefFL;

    // ���������б�
    const int c_SynSysLibID_FuncDefs = c_SynSysLibID + 104;
    typedef TKcValListSynStruct<TKcFuncDefFL, c_SynSysLibID_FuncDefs> TKcFuncDefsFL;

    // �ӿ�
    const int c_SynSysLibID_Inf = c_SynSysLibID + 105;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_Inf, TKcTypeNameDef, TKcTypeNameDef, bool> TKcInfFL;

    // �����ӿ�
    const int c_SynSysLibID_InfFull = c_SynSysLibID + 106;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_InfFull, TKcInfFL, TKcFuncDefsFL> TKcInfFullFL;

    // ����
    const int c_SynSysLibID_Load = c_SynSysLibID + 107;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_Load, TKcVarAssDef, TKLoadLibraryPtr, bool> TKcLoadFL;

    // ��������
    const int c_SynSysLibID_LoadFull = c_SynSysLibID + 108;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_LoadFull, TKcLoadFL, TKcFuncDefsFL> TKcLoadFullFL;

    ////////////////////////////////////�����﷨///////////////////////////////////
    // �����﷨��ϵ���
    const int c_SynIncludeID = 60000;

    // ��������
    const int c_SynIncludeID_Single = c_SynIncludeID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynIncludeID_Single, TKcWebExpr, bool> TKcIncludeCL;
}
