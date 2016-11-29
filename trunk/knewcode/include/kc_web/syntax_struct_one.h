#pragma once

#include "kc_web/web_struct.h"

namespace KC
{
    // 注解语法体系编号
    const int c_SynSysNoteID = 5000;

    ////////////////////////////////////结构语句///////////////////////////////////
    // 结构语句语法体系编号
    const int c_SynSysSentID = 10000;

    // 输出语句
    const int c_SynSysSentID_Print = c_SynSysSentID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_Print, TKcWebExpr, TKcWebExpr> TKcPrintSent;

    // 分支语句
    const int c_SynSysSentID_If = c_SynSysSentID + 201;
    const int c_SynSysSentID_ElseIf = c_SynSysSentID + 202;
    const int c_SynSysSentID_Else = c_SynSysSentID + 203;
    const int c_SynSysSentID_EndIf = c_SynSysSentID + 204;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_If, TKcWebExpr> TKcIfSent;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_ElseIf, TKcWebExpr> TKcElseIfSent;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_Else> TKcElseSent;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_EndIf> TKcEndIfSent;

    // 循环语句
    const int c_SynSysSentID_While = c_SynSysSentID + 301;
    const int c_SynSysSentID_Break = c_SynSysSentID + 302;
    const int c_SynSysSentID_Continue = c_SynSysSentID + 303;
    const int c_SynSysSentID_EndWhile = c_SynSysSentID + 304;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_While, TKcWebExpr> TKcWhileSent;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_Break, TKcWebExpr> TKcBreakSent;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_Continue, TKcWebExpr> TKcContinueSent;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_EndWhile, TKcWebExpr> TKcEndWhileSent;

    // exit语句
    const int c_SynSysSentID_Exit = c_SynSysSentID + 401;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_Exit, TKcWebExpr, TKcWebExpr> TKcExitSent;

    // 执行语句
    const int c_SynSysSentID_Exec = c_SynSysSentID + 501;
    typedef TKcTuple<TSynSectInBuf, c_SynSysSentID_Exec, TKcWebExpr, TKcWebExpr> TKcExecSent;

    ////////////////////////////////////基本语法///////////////////////////////////
    // 基本语法体系编号
    const int c_SynSysBaseID = 20000;

    // Html串
    const int c_SynSysBaseID_Html = c_SynSysBaseID + 101;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_Html, string> TKcHtmlSyn;

    // 字符串
    const int c_SynSysBaseID_String = c_SynSysBaseID + 201;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_String, string> TKcStringSyn;

    // 标识符
    const int c_SynSysBaseID_IDName = c_SynSysBaseID + 202;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_IDName, string> TKcIDNameSyn;

    // 成员引用
    const int c_SynSysBaseID_Member = c_SynSysBaseID + 203;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_Member, string> TKcMemberSyn;

    // 数据类型
    const int c_SynSysBaseID_DataType = c_SynSysBaseID + 205;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_DataType, EKcDataType> TKcDataTypeSyn;

    // 布尔型常量
    const int c_SynSysBaseID_ConstBool = c_SynSysBaseID + 206;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_ConstBool, bool> TKcConstBoolSyn;

    // 常量
    const int c_SynSysBaseID_Const = c_SynSysBaseID + 207;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_Const, EKcDataType, double, int, TKcConstBoolSyn, TKcStringSyn> TKcConstSyn;

    // 变量
    typedef TKcIDNameSyn TKcVarSyn;

    // 表达式列表
    const int c_SynSysBaseID_ExprList = c_SynSysBaseID + 208;
    typedef TKcValListSynStructWithPos<TKcWebExpr, c_SynSysBaseID_ExprList> TKcExprListSyn;

    // 内部变量
    const int c_SynSysBaseID_InnerVar = c_SynSysBaseID + 209;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_InnerVar, string, TKcExprListSyn> TKcInnerVarSyn;

    // 函数调用体
    const int c_SynSysBaseID_FuncCallBody = c_SynSysBaseID + 210;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_FuncCallBody, TKcMemberSyn, TKcExprListSyn> TKcFuncCallBodySyn;

    // 函数调用体列表
    const int c_SynSysBaseID_FuncCallBodyList = c_SynSysBaseID + 211;
    typedef TKcValListSynStruct<TKcFuncCallBodySyn, c_SynSysBaseID_FuncCallBodyList> TKcFuncCallBodyListSyn;

    // 函数调用
    const int c_SynSysBaseID_FuncCall = c_SynSysBaseID + 212;
    typedef TKcTuple<TSynSectInBuf, c_SynSysBaseID_FuncCall, TKcIDNameSyn, TKcWebExpr, TKcFuncCallBodyListSyn> TKcFuncCallSyn;

    /////////////////////////////////////表达式////////////////////////////////////
    // 基本语法体系编号
    const int c_SynSysExprID = 30000;

    // 括号
    const int c_SynSysExprID_YKHLeft = c_SynSysExprID + 101;
    const int c_SynSysExprID_YKHRight = c_SynSysExprID + 102;
    typedef TKcSynExprOpr<c_SynSysExprID_YKHLeft> TKcYKHLeftExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_YKHRight> TKcYKHRightExpr;

    // 负号
    const int c_SynSysExprID_Subt = c_SynSysExprID + 103;
    typedef TKcSynExprOpr<c_SynSysExprID_Subt> TKcSubtExpr;

    // 非
    const int c_SynSysExprID_Not = c_SynSysExprID + 104;
    typedef TKcSynExprOpr<c_SynSysExprID_Not> TKcNotExpr;

    // 乘、除、取余
    const int c_SynSysExprID_Star = c_SynSysExprID + 105;
    const int c_SynSysExprID_Div = c_SynSysExprID + 106;
    const int c_SynSysExprID_Mod = c_SynSysExprID + 107;
    typedef TKcSynExprOpr<c_SynSysExprID_Star> TKcStarExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_Div> TKcDivExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_Mod> TKcModExpr;

    // 加、减
    const int c_SynSysExprID_Plus = c_SynSysExprID + 108;
    const int c_SynSysExprID_Minus = c_SynSysExprID + 109;
    typedef TKcSynExprOpr<c_SynSysExprID_Plus> TKcPlusExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_Minus> TKcMinusExpr;


    // 等于、不等于、大于等于、小于等于、大于、小于
    const int c_SynSysExprID_EqualTo = c_SynSysExprID + 110;
    const int c_SynSysExprID_NotEqual = c_SynSysExprID + 111;
    const int c_SynSysExprID_EqualMore = c_SynSysExprID + 112;
    const int c_SynSysExprID_EqualLess = c_SynSysExprID + 113;
    const int c_SynSysExprID_More = c_SynSysExprID + 114;
    const int c_SynSysExprID_Less = c_SynSysExprID + 115;
    typedef TKcSynExprOpr<c_SynSysExprID_EqualTo> TKcEqualToExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_NotEqual> TKcNotEqualExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_EqualMore> TKcEqualMoreExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_EqualLess> TKcEqualLessExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_More> TKcMoreExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_Less> TKcLessExpr;

    // 与、或、异或
    const int c_SynSysExprID_And = c_SynSysExprID + 116;
    const int c_SynSysExprID_Or = c_SynSysExprID + 117;
    const int c_SynSysExprID_Xor = c_SynSysExprID + 118;
    typedef TKcSynExprOpr<c_SynSysExprID_And> TKcAndExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_Or> TKcOrExpr;
    typedef TKcSynExprOpr<c_SynSysExprID_Xor> TKcXorExpr;
}
