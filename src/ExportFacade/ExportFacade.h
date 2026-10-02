#ifndef EXPORTFACADE_H
#define EXPORTFACADE_H

#include "ExportTypes.h"

#include "ExportFacade_global.h"

#ifdef __cplusplus
struct ConversionCallbacks;
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Exports a STEP file directly to a 3D PDF by coordinating the underlying modules
 */
EXPORTFACADE_EXPORT void ExportStepToPdf(ExportRequest request, ExportResult* outResult);

#ifdef __cplusplus
}

void ExportStepToPdfWithCallbacks(
    const ExportRequest& request,
    ExportResult* outResult,
    const ConversionCallbacks* callbacks);
#endif

#endif // EXPORTFACADE_H
