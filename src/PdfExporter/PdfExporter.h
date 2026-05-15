#ifndef PDFEXPORTER_H
#define PDFEXPORTER_H

#include "ExportTypes.h"

#include "PdfExporter_global.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Embeds PRC data into a PDF file
 */
PDFEXPORTER_EXPORT void EmbedPrcToPdf(HPrcData prc, const char* outPdfPath, PdfSettings settings, ExportResult* outResult);

#ifdef __cplusplus
}
#endif

#endif // PDFEXPORTER_H
