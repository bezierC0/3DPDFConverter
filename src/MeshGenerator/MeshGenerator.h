#ifndef MESHGENERATOR_H
#define MESHGENERATOR_H

#include "ExportTypes.h"

#include "MeshGenerator_global.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generates a triangulated mesh from the given STEP model
 */
MESHGENERATOR_EXPORT void GenerateMesh(HStepModel model, MeshSettings settings, HMeshData* outMesh, ExportResult* outResult);

/**
 * @brief Frees the resources associated with the mesh data
 */
MESHGENERATOR_EXPORT void FreeMesh(HMeshData mesh);

#ifdef __cplusplus
}
#endif

#endif // MESHGENERATOR_H
