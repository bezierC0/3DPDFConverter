#include "ExportFacade.h"
#include "CoreTypes.h"
#include <exception>
#include <string.h>

// We need to declare the external functions from other DLLs we will use, or include their headers.
// Since we used C ABI and separate directories, we can either include them directly or declare them.
// For simplicity in this scaffolding, we declare them.
extern "C" {
    void LoadStep(const char* filePath, HStepModel* outModel, ExportResult* outResult);
    void FreeStep(HStepModel model);
    
    void GenerateMesh(HStepModel model, MeshSettings settings, HMeshData* outMesh, ExportResult* outResult);
    void FreeMesh(HMeshData mesh);
    
    void WritePrc(HMeshData mesh, HPrcData* outPrc, ExportResult* outResult);
    void FreePrc(HPrcData prc);
    
    void EmbedPrcToPdf(HPrcData prc, const char* outPdfPath, PdfSettings settings, ExportResult* outResult);
}

extern "C" {

void ExportStepToPdf(ExportRequest request, ExportResult* outResult) {
    if (!outResult) return;

    try {
        HStepModel stepModel = nullptr;
        HMeshData meshData = nullptr;
        HPrcData prcData = nullptr;

        // 1. Load STEP
        LoadStep(request.stepFilePath, &stepModel, outResult);
        if (outResult->code != RESULT_SUCCESS) return;

        // 2. Generate Mesh
        GenerateMesh(stepModel, request.meshSettings, &meshData, outResult);
        if (outResult->code != RESULT_SUCCESS) {
            FreeStep(stepModel);
            return;
        }

        // 3. Write PRC
        WritePrc(meshData, &prcData, outResult);
        if (outResult->code != RESULT_SUCCESS) {
            FreeMesh(meshData);
            FreeStep(stepModel);
            return;
        }

        // 4. Embed into PDF
        EmbedPrcToPdf(prcData, request.pdfFilePath, request.pdfSettings, outResult);

        // Cleanup
        FreePrc(prcData);
        FreeMesh(meshData);
        FreeStep(stepModel);
    } 
    catch (const std::exception& e) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.what());
    }
    catch (...) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, "Unknown exception caught in ExportFacade.");
    }
}

} // extern "C"
