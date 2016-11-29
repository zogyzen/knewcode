#pragma once

namespace KC { namespace Keychar { namespace one_level
{
    /// ����
    // ���ͷ
    constexpr char const g_SymbolHeadUn[] = "<!--";
    constexpr char const g_SymbolHead[] = "<%";
    // ���β
    constexpr char const g_SymbolTailUn[] = "-->";
    constexpr char const g_SymbolTail[] = "%>";
    // �ؼ���ͷ
    constexpr char const g_SymbolKeywords = '#';
    // ��ʶ��ͷ
    constexpr char const g_SymbolIDName = '$';
    constexpr char const g_SymbolIDNameInner[] = "$$";
    // �Ǻ�
    constexpr char const g_SymbolStar = '*';
    // ���ã������ں����βΣ�
    constexpr char const g_SymbolRef = '&';
    // ˫����
    constexpr char const g_SymbolDbQuotes = '"';
    // ��ֵ
    constexpr char const g_SymbolEvaluate = '=';
    // Բ��
    constexpr char const g_SymbolDot = '.';
    // ����
    constexpr char const g_SymbolComma = ',';
    // �ֺ�
    constexpr char const g_SymbolSemic = ';';
    // ð��
    constexpr char const g_SymbolColon = ':';
    // ע��
    constexpr char const g_SymbolNoteLeft = '{';
    constexpr char const g_SymbolNoteRight = '}';
    // ������
    constexpr char const g_SymbolArrayLeft = '[';
    constexpr char const g_SymbolArrayRight = ']';
    // �»���
    constexpr char const g_SymbolUnderline = '_';
    // ׷��
    constexpr char const g_SymbolAppend[] = "+=";
    constexpr char const g_SymbolDetach[] = "-=";

    /// ������������ȼ���
    // ��
    constexpr char const g_SymbolAnd[] = "&&";
    // ��
    constexpr char const g_SymbolOr[] = "||";
    // ���
    constexpr char const g_SymbolXor = '^';
    // ����
    constexpr char const g_SymbolEqualTo[] = "==";
    // ������
    constexpr char const g_SymbolNotEqual[] = "!=";
    // ����
    constexpr char const g_SymbolMore = '>';
    // С��
    constexpr char const g_SymbolLess = '<';
    // ���ڵ���
    constexpr char const g_SymbolEqualMore[] = ">=";
    // С�ڵ���
    constexpr char const g_SymbolEqualLess[] = "<=";
    // ��
    constexpr char const g_SymbolPlus = '+';
    // ��
    constexpr char const g_SymbolSubt = '-';
    // ��
    constexpr char const g_SymbolDiv = '/';
    // ȡ��
    constexpr char const g_SymbolMod = '\\';
    // ��
    constexpr char const g_SymbolNot = '!';
    // Բ����
    constexpr char const g_SymbolYKHLeft = '(';
    constexpr char const g_SymbolYKHRight = ')';

    /// �ؼ���
    // ���������ļ�
    constexpr char const g_KeywordImport[] = "import";
    constexpr char const g_KeywordExport[] = "export";
    // ģ�����
    constexpr char const g_KeywordLoadMod[] = "load";
    // ��������
    constexpr char const g_KeywordVoid[] = "void";
    constexpr char const g_KeywordInt[] = "int";
    constexpr char const g_KeywordDouble[] = "double";
    constexpr char const g_KeywordStr[] = "string";
    constexpr char const g_KeywordBool[] = "bool";
    constexpr char const g_KeywordTrue[] = "true";
    constexpr char const g_KeywordFalse[] = "false";

    // �������ҳ
    constexpr char const g_KeywordPrint[] = "print";
    constexpr char const g_KeywordPrintSE = 'p';
    // �˳����
    constexpr char const g_KeywordExit[] = "exit";
    // ִ�����
    constexpr char const g_KeywordExecute[] = "exec";
    // ��֧���
    constexpr char const g_KeywordIf[] = "if";
    constexpr char const g_KeywordElseIf[] = "else-if";
    constexpr char const g_KeywordElse[] = "else";
    constexpr char const g_KeywordEndIf[] = "end-if";
    // ѭ�����
    constexpr char const g_KeywordWhile[] = "while";
    constexpr char const g_KeywordBreak[] = "break";
    constexpr char const g_KeywordContinue[] = "continue";
    constexpr char const g_KeywordEndWhile[] = "end-while";

    // ע��
    constexpr char const g_KeywordNoteLeft[] = "note";
    constexpr char const g_KeywordNoteRight[] = "end-note";

    /// �ڲ�����
    constexpr char const g_InnerVarGet[] = "get";
    constexpr char const g_InnerVarPage[] = "page";
    constexpr char const g_InnerVarPath[] = "path";
    constexpr char const g_InnerVarPost[] = "post";
    constexpr char const g_InnerVarUser[] = "user";
}}}
