#include "PdfExporter.h"
#include "CoreTypes.h"
#include <exception>
#include <string.h>

#ifdef _WIN32
#include <hpdf.h>
#include <hpdf_u3d.h>
#include <string>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <cmath>

struct PrcDataWrapper {
    std::string buffer;
};

extern "C" {

static void hpdfErrorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void* /*user_data*/) {
    throw std::runtime_error(
        "libharu error: 0x" + std::to_string(static_cast<unsigned>(error_no)) +
        "  detail: "        + std::to_string(static_cast<unsigned>(detail_no)));
}

void EmbedPrcToPdf(HPrcData prc, const char* outPdfPath, PdfSettings settings, ExportResult* outResult) {
    if (!prc || !outPdfPath || !outResult) return;

    HPDF_Doc pdf = nullptr;
    std::string tempPrcPath;
    
    try {
        PrcDataWrapper* prcData = static_cast<PrcDataWrapper*>(prc);
        
        // Write PRC to temp file
        std::filesystem::path pdfP(outPdfPath);
        tempPrcPath = (pdfP.parent_path() / "temp_model_prc.prc").string();
        
        {
            std::ofstream tempOut(tempPrcPath, std::ios::binary);
            if (!tempOut) throw std::runtime_error("Could not create temp PRC file.");
            tempOut.write(prcData->buffer.data(), prcData->buffer.size());
        }

        pdf = HPDF_New(hpdfErrorHandler, nullptr);
        if (!pdf) throw std::runtime_error("HPDF_New failed.");

        HPDF_U3D u3d = HPDF_LoadU3DFromFile(pdf, tempPrcPath.c_str());
        if (!u3d) throw std::runtime_error("HPDF_LoadU3DFromFile failed.");

        HPDF_Dict view = HPDF_Create3DView(pdf->mmgr, "Default");
        if (!view) throw std::runtime_error("HPDF_Create3DView failed.");

        // Some basic view bounds calculation or defaults
        const HPDF_REAL cx = 0, cy = 0, cz = 0;
        const HPDF_REAL roo = 200.0f; 

        HPDF_3DView_SetCamera(view,
            cx, cy, cz,          // centre of orbit (coo)
            0, 0, 1,             // c2c vector (camera-to-centre direction)
            roo,                 // radius of orbit
            0.0f);               // roll angle

        HPDF_3DView_SetPerspectiveProjection(view, 30.0f); 

        // Extract background color from settings (0xRRGGBB)
        float r = ((settings.backgroundColorRGB >> 16) & 0xFF) / 255.0f;
        float g = ((settings.backgroundColorRGB >> 8) & 0xFF) / 255.0f;
        float b = (settings.backgroundColorRGB & 0xFF) / 255.0f;

        HPDF_3DView_SetBackgroundColor(view, r, g, b);
        HPDF_3DView_SetLighting(view, "White");

        HPDF_U3D_Add3DView(u3d, view);
        HPDF_U3D_SetDefault3DView(u3d, "Default");

        HPDF_Page page = HPDF_AddPage(pdf);
        HPDF_Page_SetWidth(page, 595.0f);  // A4 pts
        HPDF_Page_SetHeight(page, 842.0f);

        HPDF_Rect rect = { 50.0f, 100.0f, 545.0f, 792.0f };
        HPDF_Page_Create3DAnnot(page, rect, u3d);

        if (HPDF_SaveToFile(pdf, outPdfPath) != HPDF_OK)
            throw std::runtime_error("HPDF_SaveToFile failed.");

        HPDF_Free(pdf);
        pdf = nullptr;
        
        std::error_code ec;
        std::filesystem::remove(tempPrcPath, ec);

        outResult->code = RESULT_SUCCESS;
        outResult->errorMessage[0] = '\0';
    } 
    catch (const std::exception& e) {
        if (pdf) HPDF_Free(pdf);
        if (!tempPrcPath.empty()) {
            std::error_code ec;
            std::filesystem::remove(tempPrcPath, ec);
        }
        
        outResult->code = RESULT_PDF_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.what());
    }
    catch (...) {
        if (pdf) HPDF_Free(pdf);
        if (!tempPrcPath.empty()) {
            std::error_code ec;
            std::filesystem::remove(tempPrcPath, ec);
        }

        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, "Unknown exception caught during PDF generation.");
    }
}

} // extern "C"

#else // Stub for non-Windows

extern "C" {

void EmbedPrcToPdf(HPrcData prc, const char* outPdfPath, PdfSettings settings, ExportResult* outResult) {
    if (!prc || !outPdfPath || !outResult) return;
    outResult->code = RESULT_PDF_ERROR;
    CoreTypes::SafeStrCopy(outResult->errorMessage, "PDF generation is only supported on Windows.");
}

} // extern "C"

#endif // _WIN32

