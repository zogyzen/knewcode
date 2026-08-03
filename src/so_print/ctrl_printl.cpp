#include "ctrl_printl.h"

CCtrlPrint *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* dir, const char* name)
    {
        if (nullptr == g_ctrl) g_ctrl = new CCtrlPrint(dir, name);
        return *g_ctrl;
    }
    void CALL_TYPE UninitActor(IKCController&)
    {
        delete g_ctrl;
        g_ctrl = nullptr;
    }
}


////////////////////////////////////////////////////////////////////////////////
// CCtrlExcel类
CCtrlPrint::CCtrlPrint(string sPth, string sName) : m_path(sPth + sName), m_argv(m_path.data())
    , m_qtApp(m_argc, &m_argv), m_thrdQtExec([&](){ m_qtApp.exec(); })
{
}

CCtrlPrint::~CCtrlPrint()
{
}

// 执行控制器
void CCtrlPrint::Perform(ICtrlData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    // const char* pLocalFile = objCtrlD.LocalFile();
    // const char* pAct = objCtrlD.ActName();
    // const char* pMethod = objCtrlD.Method();
    // const char* pContext = objCtrlD.Context();
    IKCRequestRespond& re = objCtrlD.Re();
    try
    {
        QMetaObject::invokeMethod(qApp, [=](){
            try
            {
                QPrinter printer(QPrinter::HighResolution);
                printer.setOutputFormat(QPrinter::PdfFormat);
                printer.setOutputFileName("document.pdf");

                QTextDocument document;
                document.setHtml("Hello, world!");
                document.print(&printer);
            }
            catch (QException &ex)
            {
                qDebug() << ex.what() << Qt::endl;
            }
            catch(std::exception& e)
            {
                cout << e.what() << endl;
                qCritical() << e.what();
            }
            catch (...)
            {
                cout << "未知错误" << endl;
            }
        });

        re.OutputHTML("");
    }
    catch(std::exception& e)
    {
        re.ActionData().WriteLogFatal(re.ActionData().getHint("Action_page_"), __CURR_CODE_PLACE_C__, e.what());
    }
    catch (...)
    {
        re.ActionData().WriteLogFatal(re.ActionData().getHint("Action_page_"), __CURR_CODE_PLACE_C__);
    }
}
