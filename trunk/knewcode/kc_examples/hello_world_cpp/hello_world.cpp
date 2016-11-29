#include <iostream>
#include <string>
#include <typeinfo>
using namespace std;

#include "for_user/common_define.h"

class IHelloWorld
{
public:
    virtual const char* CALL_TYPE HelloWorld(const char*) = 0;
};

class CHelloWorld : public IHelloWorld
{
public:
    virtual const char* CALL_TYPE HelloWorld(const char* n)
    {
        m_result = string() + "Hello " + n;
        //int* i = nullptr; *i = 12;
        //int i = 1 / 0; cout << i << endl;
        cout << m_result << "\t" << n << endl;
        //throw 12;
        return m_result.c_str();
    }

public:
    CHelloWorld() { cout << "CHelloWorld" << endl; }
    virtual ~CHelloWorld() { cout << "~CHelloWorld" << endl; }

private:
    string m_result;
};

extern "C"
{
	IHelloWorld& CALL_TYPE CreateInf(void)
	{
        cout << "CreateInf" << endl;
	    CHelloWorld* hw = new CHelloWorld;
	    return *hw;
	}
	void CALL_TYPE FreeInf(IHelloWorld*& inf)
	{
	    delete dynamic_cast<CHelloWorld*>(inf);
        cout << "FreeInf" << endl;
	    inf = nullptr;
	}
}
