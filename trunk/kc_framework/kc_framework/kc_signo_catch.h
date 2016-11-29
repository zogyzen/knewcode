#pragma once

namespace KC
{
    // 信号捕捉
    class CSignoCatch : public ISignoCatch
    {
    public:
        // 信号捕捉函数
        void CALL_TYPE SignoCatchPoint(ISignoTry&) override;
        int CALL_TYPE SignoCatchPointNoThrow(ISignoTry&) override;

    public:
        CSignoCatch(void);
        virtual ~CSignoCatch(void);

        // 注册信号处理函数
        static void Signal(void);

    private:
        // 信号处理函数
        static void SignalFunc(int);
        void SignalLongjmp(int);

    private:
        int m_threadID = 0;
        // 引发信号时的位置信息
        int m_SignoCurrLine = 0;
        string m_JmpPointName = "NoJmpPoint";
        string m_SignoCurrPosInfo = "@";
        string m_BackTrace = "";
        // 跳转点栈
        struct TJmpBuf
        {
            string name;
            jmp_buf& env;
            ISignoTry& it;
            TJmpBuf(string n, jmp_buf& e, ISignoTry& i) : name(n), env(e), it(i) {}
            TJmpBuf(const TJmpBuf& c) : name(c.name), env(c.env), it(c.it) {}
        };
        vector<TJmpBuf> m_JmpVec;

    private:
        // 信号捕捉
        int SignoTryPoint(ISignoTry&, jmp_buf&);
        // 压入和弹出栈跳转点
        void PushJmpPoint(string, jmp_buf&, ISignoTry&);
        void PopJmpPoint(void);
        bool ExistsJmpPoint(void);
        // 获取栈顶跳转点
        TJmpBuf& TopJmpPoint(void);
        // 信号结果处理
        void SignoResult(int, string);
    };
}

extern CSignoCatch& GetSignoCatch(void);
