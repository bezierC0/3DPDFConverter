#ifndef CORETYPES_EXPORTTYPES_H
#define CORETYPES_EXPORTTYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handles for passing complex objects safely across DLL boundaries
typedef void* HStepModel;
typedef void* HMeshData;
typedef void* HPrcData;

/**
 * @brief Result codes for all operations across the DLL boundaries.
 */
typedef enum {
    RESULT_SUCCESS = 0,
    RESULT_INVALID_INPUT = 1,
    RESULT_OCC_ERROR = 2,
    RESULT_MESH_ERROR = 3,
    RESULT_PRC_ERROR = 4,
    RESULT_PDF_ERROR = 5,
    RESULT_UNKNOWN_ERROR = 99
} ResultCode;

/**
 * @brief Common error message buffer structure
 */
typedef struct {
    ResultCode code;
    char errorMessage[256];
} ExportResult;

/**
 * @brief Mesh generation settings
 */
typedef struct {
    double deflection;
    double angle;
    bool relativeMesh;
} MeshSettings;

/**
 * @brief PDF export settings
 */
typedef struct {
    const char* pageSize;
    uint32_t backgroundColorRGB; // Format: 0xRRGGBB
    const char* defaultLighting;
    const char* defaultView;
} PdfSettings;

/**
 * @brief Complete export request from the GUI / CLI
 */
typedef struct {
    const char* stepFilePath;
    const char* pdfFilePath;
    MeshSettings meshSettings;
    PdfSettings pdfSettings;
} ExportRequest;

#ifdef __cplusplus
}
#endif

#endif // CORETYPES_EXPORTTYPES_H
