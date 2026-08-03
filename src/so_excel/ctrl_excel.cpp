#include "ctrl_excel.h"

CCtrlExcel *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* /*dir*/, const char* name)
    {
        if (nullptr == g_ctrl) g_ctrl = new CCtrlExcel(name);
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
CCtrlExcel::CCtrlExcel(string sName) : m_name(sName)
{
}
CCtrlExcel::~CCtrlExcel()
{
}

// 执行控制器
void CCtrlExcel::Perform(ICtrlApiData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    try
    {
        // 名称
        posix_time::ptime pt = posix_time::microsec_clock::local_time();
        string sExcelName = posix_time::to_iso_string(pt), sSheetName = "Sheet", sContType = "document/excel";
        const IKCJson& jsonRequest = objCtrlD.JsonRequest();
        if (jsonRequest.IsValid())
        {
            sExcelName = jsonRequest.GetStr("ExcelName", sExcelName.c_str());
            sSheetName = jsonRequest.GetStr("SheetName", sSheetName.c_str());
        }
        // 生成Excel
        QXlsx::Document xlsxDoc;
        QXlsx::Format xlsxFormat1, xlsxFormat2;
        xlsxFormat1.setHorizontalAlignment(QXlsx::Format::AlignHCenter);
        xlsxFormat1.setFontSize(12);
        xlsxFormat1.setFontBold(true);
        xlsxFormat2.setHorizontalAlignment(QXlsx::Format::AlignRight);
        xlsxFormat2.setFontSize(10);
        // 字段
        const IKCJson& jsonFds = jsonRequest.GetItem("feilds");
        int iColCount = jsonFds.GetItemCount();
        for (int c = 0; c < iColCount; ++c)
        {
            const IKCJson& jsonFd = jsonFds.GetItem(c);
            string sName = jsonFd.GetName();
            if (sName.empty()) sName = jsonFd.GetStr("");
            xlsxDoc.write(1, c + 1, sName.c_str(), xlsxFormat1);
        }
        // 值
        const IKCJson& jsonVals = jsonRequest.GetItem(objCtrlD.ActionData().GetValsName());
        int iRowCount = jsonVals.GetItemCount();
        for (int r = 0; r < iRowCount; ++r)
        {
            const IKCJson& jsonRow = jsonVals.GetItem(r);
            for (int c = 0; c < iColCount; ++c)
            {
                const IKCJson& jsonVal = jsonRow.GetItem(c);
                xlsxDoc.write(r + 2, c + 1, jsonVal.GetStr(""), xlsxFormat2);
            }
            jsonVals.GC();
        }
        //xlsxDoc.saveAs("d:/example.xlsx");
        QBuffer buf;
        xlsxDoc.saveAs(&buf);
        // 输出
        auto &parmRespond = objCtrlD.JsonRespond().ParmInOut();
        parmRespond.SetHeader("Content-Disposition", ("attachment; filename=" + sExcelName + ".xlsx").c_str());
        parmRespond.SetContent(buf.data(), buf.size());
        parmRespond.SetContentType(sContType.c_str());
    }
    catch(std::exception& e)
    {
        objCtrlD.ActionData().WriteLogFatal(objCtrlD.ActionData().GetHint("Action_page_"), __CURR_CODE_PLACE_C__, e.what());
    }
    catch (...)
    {
        objCtrlD.ActionData().WriteLogFatal(objCtrlD.ActionData().GetHint("Action_page_"), __CURR_CODE_PLACE_C__);
    }
}
