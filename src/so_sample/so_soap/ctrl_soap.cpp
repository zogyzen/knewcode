#include "std.h"
#include "ctrl_soap.h"

CCtrlSoap *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* dir, const char* name)
    {
        if (nullptr == g_ctrl) g_ctrl = new CCtrlSoap(name);
        return *g_ctrl;
    }
    void CALL_TYPE UninitActor(IKCController&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////
// CCtrlToken类
CCtrlSoap::CCtrlSoap(string sName) : m_name(sName)
{
}

CCtrlSoap::~CCtrlSoap()
{
}

// 执行控制器
void CCtrlSoap::Perform(const char* pLocalFile, const char* pAct, const char* pMethod, const char* pContext, IKCRequestRespond& re, ICtrlNodeData&, IKCController::IAttachParm&)
{
    WKWebServiceSOAP11BindingProxy wk;
    wk.soap_endpoint = "http://111.202.226.69:8001/ZZSDZFP_TBKP_SPBM/services/WKWebService?wsdl";
    _ns1__doService request;
    _ns1__doServiceResponse respond;
    if (wk.doService(&request, respond) == SOAP_OK)
        cout << respond.return_ << endl;
    else
        wk.soap_stream_fault(std::cerr);
    wk.destroy();
}
