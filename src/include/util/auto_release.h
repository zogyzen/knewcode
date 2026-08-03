#pragma once

namespace KC
{
	// 自动释放资源
	class CAutoRelease
	{
	public:
		template<typename FEnd>
        CAutoRelease(const FEnd& f2, bool noExpt = false) : m_pHelder(new Helder<FEnd>(f2, noExpt)) {}
        template<typename FBegin, typename FEnd>
        CAutoRelease(const FBegin& f1, const FEnd& f2, bool noExpt = false) : m_pHelder(new Helder2<FBegin, FEnd>(f1, f2, noExpt)) {}
		~CAutoRelease() { delete m_pHelder; }

	private:
		struct IHelder
		{
			virtual ~IHelder() = default;
		};
		IHelder* m_pHelder = nullptr;

		template<typename FEnd>
		struct Helder : public IHelder
		{
			FEnd m_fEnd;
            bool m_noExpt = false;
            Helder(const FEnd& f2, bool noExpt) : m_fEnd(f2), m_noExpt(noExpt) {}
            virtual ~Helder()
            {
                if (m_noExpt)
                try
                {
                    m_fEnd();
                }
                catch (...) {}
                else m_fEnd();
            }
		};

		template<typename FBegin, typename FEnd>
		struct Helder2 : public IHelder
		{
			FEnd m_fEnd;
            bool m_noExpt = false;
            Helder2(const FBegin& f1, const FEnd& f2, bool noExpt) : m_fEnd(f2), m_noExpt(noExpt)
            {
                if (m_noExpt)
                try
                {
                    f1();
                }
                catch (...) {}
                else f1();
            }
            virtual ~Helder2()
            {
                if (m_noExpt)
                try
                {
                    m_fEnd();
                }
                catch (...) {}
                else m_fEnd();
            }
		};
	};
}
