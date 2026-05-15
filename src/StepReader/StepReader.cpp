#include "StepReader.h"
#include "CoreTypes.h"
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Message_ProgressIndicator.hxx>
#include <exception>
#include <string.h>
#include <iostream>

// Internal struct to hold the OCC BRep object, hidden from the ABI boundary
struct StepModelWrapper {
    TopoDS_Shape shape;
};

extern "C" {

void LoadStep(const char* filePath, HStepModel* outModel, ExportResult* outResult) {
    if (!filePath || !outModel || !outResult) return;

    try {
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath);
        
        if (status != IFSelect_RetDone) {
            outResult->code = RESULT_OCC_ERROR;
            CoreTypes::SafeStrCopy(outResult->errorMessage, "Failed to read STEP file.");
            *outModel = nullptr;
            return;
        }

        reader.TransferRoots();
        TopoDS_Shape shape = reader.OneShape();
        
        if (shape.IsNull()) {
            outResult->code = RESULT_OCC_ERROR;
            CoreTypes::SafeStrCopy(outResult->errorMessage, "STEP file contains no valid shape.");
            *outModel = nullptr;
            return;
        }

        // Allocate the wrapper struct on the heap and cast to the opaque handle
        StepModelWrapper* wrapper = new StepModelWrapper();
        wrapper->shape = shape;
        *outModel = static_cast<HStepModel>(wrapper);
        
        outResult->code = RESULT_SUCCESS;
        outResult->errorMessage[0] = '\0';
    } 
    catch (const std::exception& e) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.what());
        *outModel = nullptr;
    }
    catch (...) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, "Unknown exception caught during STEP reading.");
        *outModel = nullptr;
    }
}

void FreeStep(HStepModel model) {
    if (model) {
        StepModelWrapper* wrapper = static_cast<StepModelWrapper*>(model);
        delete wrapper;
    }
}

} // extern "C"
