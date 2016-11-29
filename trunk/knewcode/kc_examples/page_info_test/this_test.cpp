#include <iostream>
#include <string>
using namespace std;

#include "for_user/page_interface.h"
using namespace KC;

extern "C"
{
	const char* CALL_TYPE ThisTest(IActionData& act)
	{
	    act.GetRequestRespond().OutputHTML("okokokok-");
	    return act.GetRequestRespond().GetLocalPagePath();
	}
}
