#include "ExportFacade.h"
#include "CoreTypes.h"
#include "ConversionCallbacks.h"
#include <exception>
#include <memory>
#include <string.h>

// These internal C entry points are compiled into the unified converter library.
extern "C" {
    void LoadStep(const char* filePath, HStepModel* outModel, ExportResult* outResult);
    void FreeStep(HStepModel model);
    
    void GenerateMesh(HStepModel model, MeshSettings settings, HMeshData* outMesh, ExportResult* outResult);
    void FreeMesh(HMeshData mesh);
    
    void WritePrc(HMeshData mesh, PrcSettings prcSettings, MaterialSettings materialSettings, HPrcData* outPrc, ExportResult* outResult);
    void FreePrc(HPrcData prc);
    
    void EmbedPrcToPdf(HPrcData prc, const char* outPdfPath, PdfSettings settings, ExportResult* outResult);
}

namespace
{
constexpr int STAGE_READING_CAD = 1;
constexpr int STAGE_GENERATING_MESH = 2;
constexpr int STAGE_WRITING_PRC = 3;
constexpr int STAGE_WRITING_PDF = 4;
constexpr int STAGE_COMPLETED = 5;

void reportProgress(const ConversionCallbacks* callbacks, int stage, double progress, const char* message)
{
    if (callbacks != nullptr && callbacks->reportProgress != nullptr)
    {
        callbacks->reportProgress(callbacks->userData, stage, progress, message);
    }
}

void reportLog(const ConversionCallbacks* callbacks, int level, const char* message)
{
    if (callbacks != nullptr && callbacks->reportLog != nullptr)
    {
        callbacks->reportLog(callbacks->userData, level, message);
    }
}

bool stopIfCancelled(const ConversionCallbacks* callbacks, ExportResult* result)
{
    if (callbacks == nullptr || callbacks->isCancellationRequested == nullptr ||
        !callbacks->isCancellationRequested(callbacks->userData))
    {
        return false;
    }

    result->code = RESULT_UNKNOWN_ERROR;
    CoreTypes::SafeStrCopy(result->errorMessage, "Conversion cancelled.");
    reportLog(callbacks, 0, "Conversion cancelled.");
    return true;
}
}

void ExportStepToPdfWithCallbacks(
    const ExportRequest& request,
    ExportResult* outResult,
    const ConversionCallbacks* callbacks)
{
    if (!outResult) return;

    try {
        HStepModel rawStepModel = nullptr;

        reportProgress(callbacks, STAGE_READING_CAD, 0.05, "Reading CAD input.");
        reportLog(callbacks, 0, "Reading CAD input.");
        if (stopIfCancelled(callbacks, outResult)) return;
        LoadStep(request.stepFilePath, &rawStepModel, outResult);
        if (outResult->code != RESULT_SUCCESS) return;
        std::unique_ptr<void, decltype(&FreeStep)> stepModel(rawStepModel, &FreeStep);

        reportProgress(callbacks, STAGE_GENERATING_MESH, 0.30, "Generating mesh.");
        reportLog(callbacks, 0, "Generating mesh.");
        if (stopIfCancelled(callbacks, outResult)) return;
        HMeshData rawMeshData = nullptr;
        GenerateMesh(stepModel.get(), request.meshSettings, &rawMeshData, outResult);
        if (outResult->code != RESULT_SUCCESS) return;
        std::unique_ptr<void, decltype(&FreeMesh)> meshData(rawMeshData, &FreeMesh);

        reportProgress(callbacks, STAGE_WRITING_PRC, 0.60, "Writing PRC data.");
        reportLog(callbacks, 0, "Writing PRC data.");
        if (stopIfCancelled(callbacks, outResult)) return;
        HPrcData rawPrcData = nullptr;
        WritePrc(meshData.get(), request.prcSettings, request.materialSettings, &rawPrcData, outResult);
        if (outResult->code != RESULT_SUCCESS) return;
        std::unique_ptr<void, decltype(&FreePrc)> prcData(rawPrcData, &FreePrc);

        reportProgress(callbacks, STAGE_WRITING_PDF, 0.85, "Writing 3D PDF output.");
        reportLog(callbacks, 0, "Writing 3D PDF output.");
        if (stopIfCancelled(callbacks, outResult)) return;
        EmbedPrcToPdf(prcData.get(), request.pdfFilePath, request.pdfSettings, outResult);
        if (outResult->code != RESULT_SUCCESS) return;
        if (stopIfCancelled(callbacks, outResult)) return;

        reportProgress(callbacks, STAGE_COMPLETED, 1.0, "Conversion completed.");
        reportLog(callbacks, 0, "Conversion completed.");
    }
    catch (const std::exception& e) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.what());
        reportLog(callbacks, 2, e.what());
    }
    catch (...) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, "Unknown exception caught in ExportFacade.");
        reportLog(callbacks, 2, "Unknown exception caught in ExportFacade.");
    }
}

extern "C" {

void ExportStepToPdf(ExportRequest request, ExportResult* outResult) {
    ExportStepToPdfWithCallbacks(request, outResult, nullptr);
}

} // extern "C"
