#ifndef PRCEXPORTER_H
#define PRCEXPORTER_H

#include "ExportTypes.h"

#include "PrcExporter_global.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Converts triangulated mesh data into PRC binary format
 */
PRCEXPORTER_EXPORT void WritePrc(HMeshData mesh, HPrcData* outPrc, ExportResult* outResult);

/**
 * @brief Frees the resources associated with the PRC data
 */
PRCEXPORTER_EXPORT void FreePrc(HPrcData prc);

#ifdef __cplusplus
}
#endif

#endif // PRCEXPORTER_H
