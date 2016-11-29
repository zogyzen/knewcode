#pragma once

namespace KC { namespace Keychar { namespace one_level
{
    /// 符号
    // 标记头
    constexpr char const g_SymbolHeadUn[] = "<!--";
    constexpr char const g_SymbolHead[] = "<%";
    // 标记尾
    constexpr char const g_SymbolTailUn[] = "-->";
    constexpr char const g_SymbolTail[] = "%>";
    // 关键字头
    constexpr char const g_SymbolKeywords = '#';
    // 标识符头
    constexpr char const g_SymbolIDName = '$';
    constexpr char const g_SymbolIDNameInner[] = "$$";
    // 星号
    constexpr char const g_SymbolStar = '*';
    // 引用（仅用于函数形参）
    constexpr char const g_SymbolRef = '&';
    // 双引号
    constexpr char const g_SymbolDbQuotes = '"';
    // 赋值
    constexpr char const g_SymbolEvaluate = '=';
    // 圆点
    constexpr char const g_SymbolDot = '.';
    // 逗号
    constexpr char const g_SymbolComma = ',';
    // 分号
    constexpr char const g_SymbolSemic = ';';
    // 冒号
    constexpr char const g_SymbolColon = ':';
    // 注释
    constexpr char const g_SymbolNoteLeft = '{';
    constexpr char const g_SymbolNoteRight = '}';
    // 方括号
    constexpr char const g_SymbolArrayLeft = '[';
    constexpr char const g_SymbolArrayRight = ']';
    // 下划线
    constexpr char const g_SymbolUnderline = '_';
    // 追加
    constexpr char const g_SymbolAppend[] = "+=";
    constexpr char const g_SymbolDetach[] = "-=";

    /// 运算符（按优先级）
    // 与
    constexpr char const g_SymbolAnd[] = "&&";
    // 或
    constexpr char const g_SymbolOr[] = "||";
    // 异或
    constexpr char const g_SymbolXor = '^';
    // 等于
    constexpr char const g_SymbolEqualTo[] = "==";
    // 不等于
    constexpr char const g_SymbolNotEqual[] = "!=";
    // 大于
    constexpr char const g_SymbolMore = '>';
    // 小于
    constexpr char const g_SymbolLess = '<';
    // 大于等于
    constexpr char const g_SymbolEqualMore[] = ">=";
    // 小于等于
    constexpr char const g_SymbolEqualLess[] = "<=";
    // 加
    constexpr char const g_SymbolPlus = '+';
    // 减
    constexpr char const g_SymbolSubt = '-';
    // 除
    constexpr char const g_SymbolDiv = '/';
    // 取余
    constexpr char const g_SymbolMod = '\\';
    // 非
    constexpr char const g_SymbolNot = '!';
    // 圆括号
    constexpr char const g_SymbolYKHLeft = '(';
    constexpr char const g_SymbolYKHRight = ')';

    /// 关键字
    // 包含其他文件
    constexpr char const g_KeywordImport[] = "import";
    constexpr char const g_KeywordExport[] = "export";
    // 模块加载
    constexpr char const g_KeywordLoadMod[] = "load";
    // 数据类型
    constexpr char const g_KeywordVoid[] = "void";
    constexpr char const g_KeywordInt[] = "int";
    constexpr char const g_KeywordDouble[] = "double";
    constexpr char const g_KeywordStr[] = "string";
    constexpr char const g_KeywordBool[] = "bool";
    constexpr char const g_KeywordTrue[] = "true";
    constexpr char const g_KeywordFalse[] = "false";

    // 输出到网页
    constexpr char const g_KeywordPrint[] = "print";
    constexpr char const g_KeywordPrintSE = 'p';
    // 退出语句
    constexpr char const g_KeywordExit[] = "exit";
    // 执行语句
    constexpr char const g_KeywordExecute[] = "exec";
    // 分支语句
    constexpr char const g_KeywordIf[] = "if";
    constexpr char const g_KeywordElseIf[] = "else-if";
    constexpr char const g_KeywordElse[] = "else";
    constexpr char const g_KeywordEndIf[] = "end-if";
    // 循环语句
    constexpr char const g_KeywordWhile[] = "while";
    constexpr char const g_KeywordBreak[] = "break";
    constexpr char const g_KeywordContinue[] = "continue";
    constexpr char const g_KeywordEndWhile[] = "end-while";

    // 注解
    constexpr char const g_KeywordNoteLeft[] = "note";
    constexpr char const g_KeywordNoteRight[] = "end-note";

    /// 内部变量
    constexpr char const g_InnerVarGet[] = "get";
    constexpr char const g_InnerVarPage[] = "page";
    constexpr char const g_InnerVarPath[] = "path";
    constexpr char const g_InnerVarPost[] = "post";
    constexpr char const g_InnerVarUser[] = "user";
}}}
