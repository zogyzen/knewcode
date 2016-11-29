#include <string.h>
#include <iostream>
#include <string>
#include <typeinfo>
#include <iostream>
#include <fstream>
using namespace std;

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost;

#include "for_user/page_interface.h"
using namespace KC;

class ILoadTest
{
public:
    virtual const char* CALL_TYPE Test(const char*) = 0;
};

// 数据缓冲
class CBufStepData : public IKCRequestRespond::IReStepData
{
public:
    CBufStepData(IActionData& act)
    {
        // int i = 1024 * 1024 * 20;
        int i = 10;
        m_p = new char[i];
        memset(m_p, 0, i);
    }
    virtual CALL_TYPE ~CBufStepData()
    {
        delete[] m_p;
    }

    // 得到名称
    virtual const char* CALL_TYPE GetName(void)
    {
        return CBufStepData::GetNameS();
    }
    static const char* GetNameS(void)
    {
        return "kc_load_test@buf#1945/09/03";
    }

public:
    char* m_p = nullptr;
};

class CLoadTest : public ILoadTest
{
public:
    virtual const char* CALL_TYPE Test(const char* n)
    {
        /*
        string sLogDir = "/tmp/knewcode";
        if (!filesystem::exists(sLogDir)) filesystem::create_directories(sLogDir);
        string sLogFile = sLogDir + "/" + lexical_cast<string>(getpid()) + "-" + lexical_cast<string>(pthread_self()) + ".log";
        ofstream flog(sLogFile, ios::out | ios::app);
        flog << "test" << endl;
        flog.close();
        */

        m_result = string() + "Hello " + n;
        int* i = nullptr; //*i = 12;
        //int i = 1 / 0; cout << i << endl;
        cout << m_result << "\t" << n << endl;
        //throw 12;
        return m_result.c_str();
    }

public:
    CLoadTest(IActionData& act)
    {
        cout << "CLoadTest" << endl;
        // int i = 1024 * 1024 * 30;
        int i = 10;
        m_p = new char[i];
        memset(m_p, 0, i);
    }
    virtual ~CLoadTest()
    {
        delete[] m_p;
        cout << "~CLoadTest" << endl;
    }

private:
    string m_result;
    char* m_p = nullptr;
};

extern "C"
{
	ILoadTest& CALL_TYPE CreateInf(IActionData& act)
	{
        cout << "CreateInf" << endl;
	    CLoadTest* hw = new CLoadTest(act);
        if (!act.GetRequestRespond().ExistsReStepData(CBufStepData::GetNameS()))
        {
            IKCRequestRespond::IReStepData* data = new CBufStepData(act.GetActRoot());
            act.GetRequestRespond().AddReStepData(CBufStepData::GetNameS(), data);
        }
	    return *hw;
	}
	void CALL_TYPE FreeInf(ILoadTest*& inf)
	{
	    delete dynamic_cast<CLoadTest*>(inf);
        cout << "FreeInf" << endl;
	    inf = nullptr;
	}
}
