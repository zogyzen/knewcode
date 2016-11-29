#pragma once

#include "kc_web/syntax_struct_one.h"
#include "util/load_library.h"

namespace KC
{
    ////////////////////////////////////定义语法///////////////////////////////////
    // 定义语法体系编号
    const int c_SynSysDefID = 40000;

    // 变量赋值
    const int c_SynSysDefID_VarAss = c_SynSysDefID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_VarAss, TKcVarSyn, TKcWebExpr> TKcVarAssDef;

    // 内部变量赋值
    const int c_SynSysDefID_InnerVarAss = c_SynSysDefID + 102;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_InnerVarAss, TKcInnerVarSyn, TKcWebExpr> TKcInnerVarAssDef;

    // 变量赋值列表
    const int c_SynSysDefID_VarAssList = c_SynSysDefID + 103;
    typedef TKcValListSynStructWithPos<TKcVarAssDef, c_SynSysDefID_VarAssList> TKcVarAssListDef;

    // 变量定义
    const int c_SynSysDefID_Var = c_SynSysDefID + 104;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_Var, TKcDataTypeSyn, TKcVarAssListDef, bool, bool, bool> TKcVarDef;

    // 表达式运算
    const int c_SynSysDefID_ExprWork = c_SynSysDefID + 105;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_ExprWork, TKcWebExpr> TKcExprWorkDef;

    // 自定义类型名
    const int c_SynSysDefID_TypeName = c_SynSysDefID + 106;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_TypeName, string> TKcTypeNameDef;

    // 事件位置
    const int c_SynSysDefID_EventOpPlace = c_SynSysDefID + 107;
    enum EEventOpPlace
    {
        OpPlaceBefore = 0,      // 在操作前
        OpPlaceAfter            // 在操作后
    };
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_EventOpPlace, EEventOpPlace> TKcEventOpPlaceDef;

    // 延迟定义
    const int c_SynSysDefID_Delay = c_SynSysDefID + 108;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_Delay, TKcTypeNameDef, int, TKcVarAssDef, TKcInnerVarAssDef, TKcExprWorkDef> TKcDelayDef;

    // 事件等级
    const int c_SynSysDefID_EventLevel = c_SynSysDefID + 109;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_EventLevel, int> TKcEventLevelDef;

    // 事件主体
    const int c_SynSysDefID_EventBody = c_SynSysDefID + 110;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_EventBody, TKcIDNameSyn, TKcMemberSyn, TKcVarAssDef, TKcInnerVarAssDef, TKcExprWorkDef> TKcEventBodyDef;

    // 事件定义
    const int c_SynSysDefID_Event = c_SynSysDefID + 111;
    typedef TKcTuple<TSynSectInBuf, c_SynSysDefID_Event, TKcEventOpPlaceDef, TKcTypeNameDef, TKcEventLevelDef, TKcEventBodyDef> TKcEventDef;

    ////////////////////////////////////函数库语法///////////////////////////////////
    // 定义语法体系编号
    const int c_SynSysLibID = 50000;

    // 形参
    const int c_SynSysLibID_Parm = c_SynSysLibID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_Parm, TKcDataTypeSyn, TKcTypeNameDef, bool> TKcParmFL;

    // 形参列表
    const int c_SynSysLibID_Parms = c_SynSysLibID + 102;
    typedef TKcValListSynStructWithPos<TKcParmFL, c_SynSysLibID_Parms> TKcParmsFL;

    // 函数声明
    const int c_SynSysLibID_FuncDef = c_SynSysLibID + 103;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_FuncDef, TKcParmFL, string, TKcParmsFL, int> TKcFuncDefFL;

    // 函数声明列表
    const int c_SynSysLibID_FuncDefs = c_SynSysLibID + 104;
    typedef TKcValListSynStruct<TKcFuncDefFL, c_SynSysLibID_FuncDefs> TKcFuncDefsFL;

    // 接口
    const int c_SynSysLibID_Inf = c_SynSysLibID + 105;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_Inf, TKcTypeNameDef, TKcTypeNameDef, bool> TKcInfFL;

    // 完整接口
    const int c_SynSysLibID_InfFull = c_SynSysLibID + 106;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_InfFull, TKcInfFL, TKcFuncDefsFL> TKcInfFullFL;

    // 加载
    const int c_SynSysLibID_Load = c_SynSysLibID + 107;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_Load, TKcVarAssDef, TKLoadLibraryPtr, bool> TKcLoadFL;

    // 完整加载
    const int c_SynSysLibID_LoadFull = c_SynSysLibID + 108;
    typedef TKcTuple<TSynSectInBuf, c_SynSysLibID_LoadFull, TKcLoadFL, TKcFuncDefsFL> TKcLoadFullFL;

    ////////////////////////////////////包含语法///////////////////////////////////
    // 定义语法体系编号
    const int c_SynIncludeID = 60000;

    // 单个包含
    const int c_SynIncludeID_Single = c_SynIncludeID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynIncludeID_Single, TKcWebExpr, bool> TKcIncludeCL;
}
