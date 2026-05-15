#ifndef CORETYPES_EXPORT_H
#define CORETYPES_EXPORT_H

// Platform-specific DLL export/import macros
#if defined(_WIN32) || defined(_WIN64)
    #define DLL_EXPORT __declspec(dllexport)
    #define DLL_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) && __GNUC__ >= 4
    #define DLL_EXPORT __attribute__((visibility("default")))
    #define DLL_IMPORT __attribute__((visibility("default")))
#else
    #define DLL_EXPORT
    #define DLL_IMPORT
#endif

// Module-specific export macros
// CoreTypes is header-only, no export needed
#define CORETYPES_API

// Logger module
#ifdef LOGGER_LIBRARY
    #define LOGGER_API DLL_EXPORT
#else
    #define LOGGER_API DLL_IMPORT
#endif

// StepReader module
#ifdef STEPREADER_LIBRARY
    #define STEPREADER_API DLL_EXPORT
#else
    #define STEPREADER_API DLL_IMPORT
#endif

// MeshGenerator module
#ifdef MESHGENERATOR_LIBRARY
    #define MESHGENERATOR_API DLL_EXPORT
#else
    #define MESHGENERATOR_API DLL_IMPORT
#endif

// PrcExporter module
#ifdef PRCEXPORTER_LIBRARY
    #define PRCEXPORTER_API DLL_EXPORT
#else
    #define PRCEXPORTER_API DLL_IMPORT
#endif

// PdfExporter module
#ifdef PDFEXPORTER_LIBRARY
    #define PDFEXPORTER_API DLL_EXPORT
#else
    #define PDFEXPORTER_API DLL_IMPORT
#endif

// ExportFacade module
#ifdef EXPORTFACADE_LIBRARY
    #define EXPORTFACADE_API DLL_EXPORT
#else
    #define EXPORTFACADE_API DLL_IMPORT
#endif

#endif // CORETYPES_EXPORT_H
