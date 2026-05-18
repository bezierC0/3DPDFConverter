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

namespace {
struct PageSizePts {
    HPDF_REAL width;
    HPDF_REAL height;
};

PageSizePts resolvePageSize(const PdfSettings& settings) {
    if (settings.pageSize && strcmp(settings.pageSize, "A3") == 0) return { 841.89f, 1190.55f };
    if (settings.pageSize && strcmp(settings.pageSize, "Letter") == 0) return { 612.0f, 792.0f };
    if (settings.pageSize && strcmp(settings.pageSize, "Legal") == 0) return { 612.0f, 1008.0f };
    if (settings.pageSize && strcmp(settings.pageSize, "Custom") == 0 &&
        settings.customPageWidthPt > 0.0 && settings.customPageHeightPt > 0.0) {
        return { static_cast<HPDF_REAL>(settings.customPageWidthPt), static_cast<HPDF_REAL>(settings.customPageHeightPt) };
    }
    return { 595.0f, 842.0f };
}

const char* resolveLighting(const PdfSettings& settings) {
    return (settings.defaultLighting && settings.defaultLighting[0] != '\0') ? settings.defaultLighting : "White";
}
}

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
            static_cast<HPDF_REAL>(settings.cameraToCenter[0]),
            static_cast<HPDF_REAL>(settings.cameraToCenter[1]),
            static_cast<HPDF_REAL>(settings.cameraToCenter[2]),
            static_cast<HPDF_REAL>(settings.orbitRadius > 0.0 ? settings.orbitRadius : roo),
            static_cast<HPDF_REAL>(settings.rollDeg));

        if (settings.projectionMode == PDF_PROJ_ORTHOGRAPHIC) {
            HPDF_3DView_SetOrthogonalProjection(view, 1.0f);
        } else {
            const HPDF_REAL fov = static_cast<HPDF_REAL>(settings.fovDeg > 0.0 ? settings.fovDeg : 30.0);
            HPDF_3DView_SetPerspectiveProjection(view, fov);
        }

        // Extract background color from settings (0xRRGGBB)
        float r = ((settings.backgroundColorRGB >> 16) & 0xFF) / 255.0f;
        float g = ((settings.backgroundColorRGB >> 8) & 0xFF) / 255.0f;
        float b = (settings.backgroundColorRGB & 0xFF) / 255.0f;

        HPDF_3DView_SetBackgroundColor(view, r, g, b);
        HPDF_3DView_SetLighting(view, resolveLighting(settings));

        HPDF_U3D_Add3DView(u3d, view);
        HPDF_U3D_SetDefault3DView(u3d, "Default");

        HPDF_Page page = HPDF_AddPage(pdf);
        const PageSizePts pageSize = resolvePageSize(settings);
        HPDF_Page_SetWidth(page, pageSize.width);
        HPDF_Page_SetHeight(page, pageSize.height);

        HPDF_Rect rect = {
            static_cast<HPDF_REAL>(settings.annotLeftPt > 0.0 ? settings.annotLeftPt : 50.0),
            static_cast<HPDF_REAL>(settings.annotBottomPt > 0.0 ? settings.annotBottomPt : 100.0),
            static_cast<HPDF_REAL>(settings.annotRightPt > 0.0 ? settings.annotRightPt : (pageSize.width - 50.0f)),
            static_cast<HPDF_REAL>(settings.annotTopPt > 0.0 ? settings.annotTopPt : (pageSize.height - 50.0f))
        };
        HPDF_Page_Create3DAnnot(page, rect, u3d);

        if (HPDF_SaveToFile(pdf, outPdfPath) != HPDF_OK)
            throw std::runtime_error("HPDF_SaveToFile failed.");

        HPDF_Free(pdf);
        pdf = nullptr;
        
        if (!settings.keepTempPrc) {
            std::error_code ec;
            std::filesystem::remove(tempPrcPath, ec);
        }

        outResult->code = RESULT_SUCCESS;
        outResult->errorMessage[0] = '\0';
    } 
    catch (const std::exception& e) {
        if (pdf) HPDF_Free(pdf);
        if (!tempPrcPath.empty() && !settings.keepTempPrc) {
            std::error_code ec;
            std::filesystem::remove(tempPrcPath, ec);
        }
        
        outResult->code = RESULT_PDF_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.what());
    }
    catch (...) {
        if (pdf) HPDF_Free(pdf);
        if (!tempPrcPath.empty() && !settings.keepTempPrc) {
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
