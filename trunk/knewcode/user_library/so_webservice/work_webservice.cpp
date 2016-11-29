#include "std.h"
#include "work_webservice.h"

constexpr const char* c_ResponseContentType = "text/xml";

////////////////////////////////////////////////////////////////////////////////
extern "C"
{
	IKCWebService& CALL_TYPE GetWorkInf(void)
	{
	    static CKCWebService s_Inf;
	    return s_Inf;
	}
}


////////////////////////////////////////////////////////////////////////////////
class CKCWSMethodRespond : public SOAPMethod
{
public:
    CKCWSMethodRespond(string n, string v, string a) : m_FuncName(n), m_ResValue(v), m_FuncAttr(a) {}
    virtual string methodName()
    {
        return m_FuncName;
    }
    virtual bool encode(SOAPEncoder& soapEncoder)
    {
        soapEncoder.encodeArgument(m_FuncName + "Result", m_ResValue);
        return true;
    }
    virtual string methodNameAttributes()
    {
        return m_FuncAttr;
    }

private:
    string m_FuncName = "", m_ResValue = "", m_FuncAttr = "";

};

////////////////////////////////////////////////////////////////////////////////
// CKCWebService��
CKCWebService::CKCWebService(void)
{
}

// ִ��soap����
bool CKCWebService::RunSoap(IActionData& act)
{
    bool bResult = false;
    act.GetRequestRespond().SetResponseContentType(c_ResponseContentType);
    string sOutXML = "";
    string sFuncName = "";
    string SoapTagName = "";
    string sFuncAttr = "";
    string sScript = this->getMethodDetails(act.GetInnerVarVal(Keychar::one_level::g_InnerVarPost, c_ResponseContentType).AsString(), sFuncName, SoapTagName, sFuncAttr);
    if (sScript.size() == 0)
        // ���ش�����Ϣ
        sOutXML = this->SoapFault("Could not parse the Soap request");
    else
        try
        {
            // �Զ����ڲ�����
            string sUserInner = "__KCWS___";
            do
            {
                sUserInner = "__KCWS_" + posix_time::to_iso_string(posix_time::second_clock::local_time()) + "R" + lexical_cast<string>(rand()) + "___";
            } while (act.ExistsInnerVar(Keychar::one_level::g_InnerVarUser, sUserInner.c_str()));
            // ���ú���
            sScript = string("<% $$") + Keychar::one_level::g_InnerVarUser + "[\"" + sUserInner + "\"]=" +  sScript + " %>";
            bResult = act.ParseActionScript("so_webservice##CKCWebService::RunSoap@@script", sScript.c_str());
            // �õ�����ֵ
            IKcWebDataVal& RetVal = act.GetInnerVarVal(Keychar::one_level::g_InnerVarUser, sUserInner.c_str());
            string sRetVal = RetVal.AsString();
            act.ReleaseDataVal(RetVal);
            // ���ɷ�����Ϣ
            sOutXML = this->encodeMethodResponse(sFuncName, sRetVal, SoapTagName, sFuncAttr);
        }
        catch (TException& e)
        {
            sOutXML = this->SoapFault(e.error_info());
        }
        catch (boost::bad_lexical_cast& e)
        {
            sOutXML = this->SoapFault(e.what());
        }
        catch (std::exception& e)
        {
            sOutXML = this->SoapFault(e.what());
        }
        catch (...)
        {
            sOutXML = this->SoapFault("Unknown Soap Error");
        }
    act.GetRequestRespond().OutputData(c_ResponseContentType, sOutXML.c_str());
    return bResult;
}

// ����Soap
string CKCWebService::getMethodDetails(string sSoap, string& sFuncName, string& SoapTagName, string& sFuncAttr)
{
    string sResult = "";
    // ����soap��
    SOAPElement theCall;
    SOAPParser soapParser;
    if (soapParser.parseMessage(sSoap, theCall))
    {
        SOAPElement* pEnvelope = nullptr;
        theCall.getElement("Envelope", &pEnvelope);
        if (nullptr != pEnvelope)
        {
            SoapTagName = pEnvelope->namespaceName();
            SOAPElement* pBody = nullptr;
            pEnvelope->getElement("Body", &pBody);
            if (nullptr != pBody)
            {
                // ֻ�����1������
                SOAPElement& aFunc = pBody->elementAt(0);
                sFuncAttr = aFunc.namespaceName();
                string sObjName = replace_last_copy(replace_first_copy(sFuncAttr, "xmlns=\"", ""), "\"", "");
                sFuncName = aFunc.accessorName();
                sResult += "$" + sObjName + "." + sFuncName + "(";
                // ѭ��ÿ������
                for (long i = 0, j = aFunc.numElements(); i < j; ++i)
                {
                    SOAPElement& aParm = aFunc.elementAt(i);
                    string sValue = aParm.value();
                    sResult += (0 == i ? "\"" : ",\"") + sValue + "\"";
                }
                sResult += ");";
            }
        }
    }
    return sResult;
}

string CKCWebService::encodeMethodResponse(string sFName, string sVal, string SoapTagName, string sFuncAttr)
{
    CKCWSMethodRespond wsres(sFName, sVal, sFuncAttr);
    SOAPEncoder theEncoder(SoapTagName);
    return theEncoder.encodeMethodResponse(wsres);
}

// ���ɴ�����Ϣ
string CKCWebService::SoapFault(string sErr)
{
    SOAPEncoder theEncoder;
    SOAPFault soapFault;
    soapFault.setSpecificFault("Call_KC_Webservice_Failure", SOAPFault::Client);
    soapFault.faultString() = sErr;
    return theEncoder.encodeFault(soapFault);
}
