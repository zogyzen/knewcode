#pragma once

namespace KC
{
	// 自动释放资源
	class CAutoRelease
	{
	public:
		template<typename FEnd>
		CAutoRelease(const FEnd& f2) : m_pHelder(new Helder<FEnd>(f2)) {}
		template<typename FBegin, typename FEnd>
		CAutoRelease(const FBegin& f1, const FEnd& f2) : m_pHelder(new Helder2<FBegin, FEnd>(f1, f2)) {}
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
			Helder(const FEnd& f2) : m_fEnd(f2) {}
			virtual ~Helder() { m_fEnd(); }
		};

		template<typename FBegin, typename FEnd>
		struct Helder2 : public IHelder
		{
			FEnd m_fEnd;
			Helder2(const FBegin& f1, const FEnd& f2) : m_fEnd(f2) { f1(); }
			virtual ~Helder2() { m_fEnd(); }
		};
	};
}
