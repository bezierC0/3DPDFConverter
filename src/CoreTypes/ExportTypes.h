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

typedef enum {
    MESH_QUALITY_LOW = 0,
    MESH_QUALITY_MEDIUM = 1,
    MESH_QUALITY_HIGH = 2,
    MESH_QUALITY_CUSTOM = 3
} MeshQualityPreset;

typedef enum {
    PDF_VIEW_ISO = 0,
    PDF_VIEW_FRONT = 1,
    PDF_VIEW_BACK = 2,
    PDF_VIEW_LEFT = 3,
    PDF_VIEW_RIGHT = 4,
    PDF_VIEW_TOP = 5,
    PDF_VIEW_BOTTOM = 6,
    PDF_VIEW_CUSTOM = 7
} PdfViewPreset;

typedef enum {
    PDF_PROJ_PERSPECTIVE = 0,
    PDF_PROJ_ORTHOGRAPHIC = 1
} PdfProjectionMode;

typedef enum {
    PDF_RENDER_SOLID = 0,
    PDF_RENDER_SOLID_EDGES = 1,
    PDF_RENDER_WIREFRAME = 2
} PdfRenderMode;

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
    MeshQualityPreset qualityPreset;
    double deflection;
    double angle;
    bool relativeMesh;
    double creaseAngle;
} MeshSettings;

typedef struct {
    double compression;
    double granularity;
    bool closed;
    bool tess;
    bool doBreak;
    bool noBreak;
} PrcSettings;

typedef struct {
    double ambient[4];
    double diffuse[4];
    double emissive[4];
    double specular[4];
    double alpha;
    double shininess;
} MaterialSettings;

typedef struct {
    double lineWidth;
    uint32_t lineColorRGB;
    double lineAlpha;
} LineStyleSettings;

/**
 * @brief PDF export settings
 */
typedef struct {
    const char* pageSize;
    double customPageWidthPt;
    double customPageHeightPt;
    double annotLeftPt;
    double annotBottomPt;
    double annotRightPt;
    double annotTopPt;
    uint32_t backgroundColorRGB; // Format: 0xRRGGBB
    const char* defaultLighting;
    const char* defaultView;
    PdfProjectionMode projectionMode;
    PdfViewPreset viewPreset;
    PdfRenderMode renderMode;
    double fovDeg;
    double rollDeg;
    double orbitRadius;
    double cameraToCenter[3];
    bool keepTempPrc;
} PdfSettings;

typedef struct {
    const char* title;
    const char* author;
    const char* subject;
    const char* keywords;
} PdfMetadata;

/**
 * @brief Complete export request from the GUI / CLI
 */
typedef struct {
    const char* stepFilePath;
    const char* pdfFilePath;
    MeshSettings meshSettings;
    PrcSettings prcSettings;
    MaterialSettings materialSettings;
    LineStyleSettings lineStyleSettings;
    PdfSettings pdfSettings;
    PdfMetadata pdfMetadata;
    bool keepTempPrc;
    uint32_t apiVersion;
    uint32_t reserved[8];
} ExportRequest;

#ifdef __cplusplus
}
#endif

#endif // CORETYPES_EXPORTTYPES_H
