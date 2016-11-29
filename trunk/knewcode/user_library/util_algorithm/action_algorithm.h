#pragma once

namespace KC
{
    class CKCAlgorithmAction : public IKCAlgorithmAction
    {
    public:
        CKCAlgorithmAction(IActionData&);

        // 获取当前时间字符串
		virtual const char* CALL_TYPE GetNowString(void);
        // 字符串代理
		virtual bool CALL_TYPE SetStringAgent(const char*, const char*);
		virtual const char* CALL_TYPE GetStringAgent(const char*);
		// 编码转换
		virtual const char* CALL_TYPE GBK2utf8(const char*);
		virtual const char* CALL_TYPE utf82GBK(const char*);

    private:
        IActionData& m_act;
        string m_str;
    };
}
