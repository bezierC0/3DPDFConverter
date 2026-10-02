#ifndef THREEDPDFCONVERTER_H
#define THREEDPDFCONVERTER_H

#include <stdint.h>
#include <stddef.h>

#if defined(_WIN32)
#if defined(THREEDPDFCONVERTER_LIBRARY)
#define THREEDPDFCONVERTER_API __declspec(dllexport)
#else
#define THREEDPDFCONVERTER_API __declspec(dllimport)
#endif
#else
#define THREEDPDFCONVERTER_API __attribute__((visibility("default")))
#endif

#define THREEDPDFCONVERTER_API_VERSION 1u

#ifdef __cplusplus
extern "C" {
#endif

typedef void* ConverterHandle;

typedef enum ConverterResultCode
{
    CONVERTER_RESULT_SUCCESS = 0,
    CONVERTER_RESULT_INVALID_ARGUMENT = 1,
    CONVERTER_RESULT_CAD_READ_ERROR = 2,
    CONVERTER_RESULT_MESH_ERROR = 3,
    CONVERTER_RESULT_PRC_ERROR = 4,
    CONVERTER_RESULT_PDF_ERROR = 5,
    CONVERTER_RESULT_BUSY = 6,
    CONVERTER_RESULT_CANCELLED = 7,
    CONVERTER_RESULT_UNKNOWN_ERROR = 99
} ConverterResultCode;

typedef enum ConverterLogLevel
{
    CONVERTER_LOG_INFO = 0,
    CONVERTER_LOG_WARNING = 1,
    CONVERTER_LOG_ERROR = 2
} ConverterLogLevel;

typedef enum ConverterStage
{
    CONVERTER_STAGE_STARTING = 0,
    CONVERTER_STAGE_READING_CAD = 1,
    CONVERTER_STAGE_GENERATING_MESH = 2,
    CONVERTER_STAGE_WRITING_PRC = 3,
    CONVERTER_STAGE_WRITING_PDF = 4,
    CONVERTER_STAGE_COMPLETED = 5
} ConverterStage;

typedef int32_t (*ConverterProgressCallback)(
    void* userData,
    int32_t stage,
    double progress,
    const char* messageUtf8);

typedef void (*ConverterLogCallback)(
    void* userData,
    int32_t level,
    const char* messageUtf8);

typedef enum ConverterProjectionMode
{
    CONVERTER_PROJECTION_PERSPECTIVE = 0,
    CONVERTER_PROJECTION_ORTHOGRAPHIC = 1
} ConverterProjectionMode;

typedef struct ConverterOptions
{
    uint32_t structSize;
    uint32_t apiVersion;
    double meshDeflection;
    double meshAngleRadians;
    uint32_t relativeMesh;
    uint32_t backgroundColorRgb;
    int32_t projectionMode;
    double fieldOfViewDegrees;
    double cameraRollDegrees;
    double orbitRadius;
    uint32_t keepTemporaryPrc;
    uint32_t reserved[16];
} ConverterOptions;

typedef struct ConverterResult
{
    uint32_t structSize;
    int32_t code;
    char messageUtf8[1024];
    uint32_t reserved[16];
} ConverterResult;

THREEDPDFCONVERTER_API uint32_t Converter_GetApiVersion(void);

THREEDPDFCONVERTER_API void Converter_GetDefaultOptions(
    ConverterOptions* options);

THREEDPDFCONVERTER_API ConverterHandle Converter_Create(void);

THREEDPDFCONVERTER_API int32_t Converter_SetCallbacks(
    ConverterHandle handle,
    ConverterProgressCallback progressCallback,
    ConverterLogCallback logCallback,
    void* userData);

THREEDPDFCONVERTER_API void Converter_RequestCancel(ConverterHandle handle);

THREEDPDFCONVERTER_API int32_t Converter_ConvertUtf8(
    ConverterHandle handle,
    const char* inputPathUtf8,
    const char* outputPathUtf8,
    const ConverterOptions* options,
    ConverterResult* result);

#if defined(_WIN32)
THREEDPDFCONVERTER_API int32_t Converter_ConvertWide(
    ConverterHandle handle,
    const wchar_t* inputPath,
    const wchar_t* outputPath,
    const ConverterOptions* options,
    ConverterResult* result);
#endif

THREEDPDFCONVERTER_API void Converter_Destroy(ConverterHandle handle);

#ifdef __cplusplus
}
#endif

#endif // THREEDPDFCONVERTER_H
