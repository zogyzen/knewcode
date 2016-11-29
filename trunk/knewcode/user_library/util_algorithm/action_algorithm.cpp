#include "std.h"
#include "action_algorithm.h"

////////////////////////////////////////////////////////////////////////////////
// CKCAlgorithmAction��
CKCAlgorithmAction::CKCAlgorithmAction(IActionData& act) : m_act(act)
{
}

// ��ȡ��ǰʱ���ַ���
const char* CKCAlgorithmAction::GetNowString(void)
{
    m_str = posix_time::to_iso_string(posix_time::second_clock::local_time());
    return m_str.c_str();
}

// �ַ�������
bool CKCAlgorithmAction::SetStringAgent(const char*, const char*)
{
}

const char* CKCAlgorithmAction::GetStringAgent(const char*)
{
}

// ����ת��
const char* CKCAlgorithmAction::GBK2utf8(const char* src)
{
    m_str = boost::locale::conv::between(src, "UTF-8", "GBK");
    return m_str.c_str();
}

const char* CKCAlgorithmAction::utf82GBK(const char* src)
{
    m_str = boost::locale::conv::between(src, "GBK", "UTF-8");
    return m_str.c_str();
}
