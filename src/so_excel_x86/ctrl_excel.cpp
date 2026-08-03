#include "std.h"
#include "ctrl_excel.h"

CCtrlExcel *g_ctrl = nullptr;
extern "C"
{
    IKCController& CALL_TYPE InitActor(const char* dir, const char* name)
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
void CCtrlExcel::Perform(ICtrlNodeData& objCtrlD, IKCController::IAttachParm&)
{
    // 控制器参数
    // const char* pLocalFile = objCtrlD.LocalFile();
    // const char* pAct = objCtrlD.ActName();
    // const char* pMethod = objCtrlD.Method();
    // const char* pContext = objCtrlD.Context();
    IKCRequestRespond& re = objCtrlD.Re();
    try
    {
        // 名称
        posix_time::ptime pt = posix_time::microsec_clock::local_time();
        string sExcelName = posix_time::to_iso_string(pt), sSheetName = "Sheet", sContType = "document/excel";
        IKCJson& jsonRoot = re.ActionData().JsonRoot();
        IKCJson& jsonParms = jsonRoot.GetItem(c_RESTful_parms);
        if (jsonParms.IsValid())
        {
            sExcelName = jsonParms.GetStr(c_RESTful_ExcelName, sExcelName.c_str());
            sSheetName = jsonParms.GetStr(c_RESTful_SheetName, sSheetName.c_str());
        }
        re.AddResponseHeader("Content-Disposition", ("attachment; filename=" + sExcelName + ".xlsx").c_str());
        re.SetResponseContentType(sContType.c_str());
        // 生成Excel
        xlnt::workbook wb;
        xlnt::worksheet ws = wb.active_sheet();
        ws.title(sSheetName);
        auto rows = ws.rows();
        // 字段
        IKCJson& jsonFds = jsonRoot.GetItem(c_RESTful_feilds);
        int iColCount = jsonFds.GetItemCount();
        for (int c = 0; c < iColCount; ++c)
        {
            IKCJson& jsonFd = jsonFds.GetItem(c);
            string sName = jsonFd.GetName();
            if (sName.empty()) sName = jsonFd.GetStr("");
            //ws.cell(c + 1, 1).value(CUtilFunc::GbkToUtf8(sName));
            rows[0][c].value(sName);
        }
        jsonRoot.GC();
        // 值
        IKCJson& jsonVals = jsonRoot.GetItem(objCtrlD.ActionData().GetValsName());
        int iRowCount = jsonVals.GetItemCount();
        for (int r = 0; r < iRowCount; ++r)
        {
            IKCJson& jsonRow = jsonVals.GetItem(r);
            for (int c = 0; c < iColCount; ++c)
            {
                IKCJson& jsonVal = jsonRow.GetItem(c);
                //ws.cell(c + 1, r + 2).value(CUtilFunc::GbkToUtf8(jsonVal.GetStr("")));
                rows[r + 1][c].value(jsonVal.GetStr(""));
            }
            jsonVals.GC();
        }
        jsonRoot.GC();
        ws.freeze_panes(ws.cell(1, 2));
        //wb.save("d:/example.xlsx");
        std::vector<unsigned char> buf;
        wb.save(buf);
        wb.clear();
        re.OutputData(sContType.c_str(), reinterpret_cast<char*>(&buf[0]), buf.size());
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
