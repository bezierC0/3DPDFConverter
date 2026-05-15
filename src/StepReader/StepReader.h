#ifndef STEPREADER_H
#define STEPREADER_H

#include "ExportTypes.h"

#include "StepReader_global.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Reads a STEP file and returns an opaque handle to the OCC BRep model.
 * @param filePath The path to the STEP file.
 * @param outModel Pointer to receive the model handle.
 * @param outResult Result status buffer.
 */
STEPREADER_EXPORT void LoadStep(const char* filePath, HStepModel* outModel, ExportResult* outResult);

/**
 * @brief Frees the resources associated with the model handle.
 * @param model The handle to free.
 */
STEPREADER_EXPORT void FreeStep(HStepModel model);

#ifdef __cplusplus
}
#endif

#endif // STEPREADER_H
