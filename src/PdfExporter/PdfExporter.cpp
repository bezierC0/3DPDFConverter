#include "PdfExporter.h"
#include "CoreTypes.h"
#include "ConversionData.h"
#include <exception>
#include <string.h>

#if defined(THREEDPDFCONVERTER_HAS_PRC_PDF_BACKEND)
#include <hpdf.h>
#include <hpdf_u3d.h>
#include <string>
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <cmath>
#include <atomic>
#include <chrono>
#include <sstream>

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

struct HpdfErrorState {
    HPDF_STATUS errorNumber = HPDF_OK;
    HPDF_STATUS detailNumber = 0;
};

std::filesystem::path makeTemporaryPrcPath(const std::filesystem::path& outputPath, bool keepTemporaryPrc) {
    if (keepTemporaryPrc) {
        std::filesystem::path keptPath = outputPath;
        keptPath += ".prc";
        return keptPath;
    }

    static std::atomic<unsigned long long> sequence { 0 };
    const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
    std::ostringstream fileName;
    fileName << "3dpdfconverter-" << std::hex << timestamp << '-' << sequence.fetch_add(1) << ".prc";
    return std::filesystem::temp_directory_path() / fileName.str();
}

void throwIfHpdfFailed(const HpdfErrorState& errorState) {
    if (errorState.errorNumber == HPDF_OK) return;
    throw std::runtime_error(
        "libharu error: 0x" + std::to_string(static_cast<unsigned>(errorState.errorNumber)) +
        " detail: " + std::to_string(static_cast<unsigned>(errorState.detailNumber)));
}
}

extern "C" {

static void hpdfErrorHandler(HPDF_STATUS errorNumber, HPDF_STATUS detailNumber, void* userData) {
    auto* errorState = static_cast<HpdfErrorState*>(userData);
    if (errorState != nullptr) {
        errorState->errorNumber = errorNumber;
        errorState->detailNumber = detailNumber;
    }
}

void EmbedPrcToPdf(HPrcData prc, const char* outPdfPath, PdfSettings settings, ExportResult* outResult) {
    if (!prc || !outPdfPath || !outResult) return;

    HPDF_Doc pdf = nullptr;
    std::string tempPrcPath;
    HpdfErrorState hpdfError;
    
    try {
        PrcDataWrapper* prcData = static_cast<PrcDataWrapper*>(prc);
        
        const std::filesystem::path pdfPath(outPdfPath);
        tempPrcPath = makeTemporaryPrcPath(pdfPath, settings.keepTempPrc).string();
        
        {
            std::ofstream tempOut(tempPrcPath, std::ios::binary);
            if (!tempOut) throw std::runtime_error("Could not create temp PRC file.");
            tempOut.write(prcData->buffer.data(), prcData->buffer.size());
        }

        pdf = HPDF_New(hpdfErrorHandler, &hpdfError);
        if (!pdf) throw std::runtime_error("HPDF_New failed.");

        HPDF_U3D u3d = HPDF_LoadU3DFromFile(pdf, tempPrcPath.c_str());
        throwIfHpdfFailed(hpdfError);
        if (!u3d) throw std::runtime_error("HPDF_LoadU3DFromFile failed.");

        HPDF_Dict view = HPDF_Create3DView(pdf->mmgr, "Default");
        throwIfHpdfFailed(hpdfError);
        if (!view) throw std::runtime_error("HPDF_Create3DView failed.");

        HPDF_REAL cx = 0.0f;
        HPDF_REAL cy = 0.0f;
        HPDF_REAL cz = 0.0f;
        HPDF_REAL modelRadius = 200.0f;
        if (prcData->hasBounds) {
            const double dx = prcData->maximumBounds[0] - prcData->minimumBounds[0];
            const double dy = prcData->maximumBounds[1] - prcData->minimumBounds[1];
            const double dz = prcData->maximumBounds[2] - prcData->minimumBounds[2];
            cx = static_cast<HPDF_REAL>((prcData->minimumBounds[0] + prcData->maximumBounds[0]) * 0.5);
            cy = static_cast<HPDF_REAL>((prcData->minimumBounds[1] + prcData->maximumBounds[1]) * 0.5);
            cz = static_cast<HPDF_REAL>((prcData->minimumBounds[2] + prcData->maximumBounds[2]) * 0.5);
            modelRadius = static_cast<HPDF_REAL>(std::sqrt(dx * dx + dy * dy + dz * dz));
            if (modelRadius < 1.0e-3f) modelRadius = 100.0f;
        }

        HPDF_3DView_SetCamera(view,
            cx, cy, cz,          // centre of orbit (coo)
            static_cast<HPDF_REAL>(settings.cameraToCenter[0]),
            static_cast<HPDF_REAL>(settings.cameraToCenter[1]),
            static_cast<HPDF_REAL>(settings.cameraToCenter[2]),
            static_cast<HPDF_REAL>(settings.orbitRadius > 0.0 ? settings.orbitRadius : modelRadius),
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
        throwIfHpdfFailed(hpdfError);

        if (HPDF_SaveToFile(pdf, outPdfPath) != HPDF_OK)
            throw std::runtime_error("HPDF_SaveToFile failed.");
        throwIfHpdfFailed(hpdfError);

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

#else // Stub for unsupported platforms

extern "C" {

void EmbedPrcToPdf(HPrcData prc, const char* outPdfPath, PdfSettings settings, ExportResult* outResult) {
    if (!prc || !outPdfPath || !outResult) return;
    outResult->code = RESULT_PDF_ERROR;
    CoreTypes::SafeStrCopy(outResult->errorMessage, "PDF generation is not supported on this platform.");
}

} // extern "C"

#endif // THREEDPDFCONVERTER_HAS_PRC_PDF_BACKEND
