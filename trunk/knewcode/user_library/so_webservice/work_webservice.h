#pragma once

namespace KC
{
    class CKCWebService : public IKCWebService
    {
    public:
        CKCWebService(void);

        // Ö´ÐÐsoap·½·¨
		virtual bool CALL_TYPE RunSoap(IActionData&);

    private:
        string getMethodDetails(string sSoap, string& sFuncName, string& SoapTagName, string& sFuncAttr);
        string encodeMethodResponse(string sFName, string sVal, string SoapTagName, string sFuncAttr);
        string SoapFault(string);
    };
}
