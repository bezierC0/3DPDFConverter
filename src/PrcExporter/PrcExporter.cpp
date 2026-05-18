#include "PrcExporter.h"
#include "CoreTypes.h"
#include <exception>
#include <string.h>

#ifdef _WIN32
#include <oPRCFile.h>
#include <vector>
#include <array>
#include <sstream>

// Same definition as in MeshGenerator.cpp
struct MeshDataWrapper {
    struct MeshPart {
        std::vector<std::array<double, 3>> vertices;
        std::vector<std::array<uint32_t, 3>> indices;
    };
    std::vector<MeshPart> parts;
};

struct PrcDataWrapper {
    std::string buffer;
};

extern "C" {

void WritePrc(HMeshData mesh, PrcSettings /*prcSettings*/, MaterialSettings materialSettings, HPrcData* outPrc, ExportResult* outResult) {
    if (!mesh || !outPrc || !outResult) return;

    try {
        MeshDataWrapper* meshData = static_cast<MeshDataWrapper*>(mesh);
        
        std::ostringstream out(std::ios::binary);
        oPRCFile prcFile(out);
        prcFile.begingroup("model");

        PRCmaterial mat(
            RGBAColour(materialSettings.ambient[0], materialSettings.ambient[1], materialSettings.ambient[2], materialSettings.ambient[3]),
            RGBAColour(materialSettings.diffuse[0], materialSettings.diffuse[1], materialSettings.diffuse[2], materialSettings.diffuse[3]),
            RGBAColour(materialSettings.emissive[0], materialSettings.emissive[1], materialSettings.emissive[2], materialSettings.emissive[3]),
            RGBAColour(materialSettings.specular[0], materialSettings.specular[1], materialSettings.specular[2], materialSettings.specular[3]),
            materialSettings.alpha,
            materialSettings.shininess
        );

        for (const auto& part : meshData->parts) {
            if (part.vertices.empty() || part.indices.empty()) continue;
            
            prcFile.addTriangles(
                static_cast<uint32_t>(part.vertices.size()),
                reinterpret_cast<const double(*)[3]>(part.vertices.data()),
                static_cast<uint32_t>(part.indices.size()),
                reinterpret_cast<const uint32_t(*)[3]>(part.indices.data()),
                mat,
                0, nullptr, nullptr,
                0, nullptr, nullptr,
                0, nullptr, nullptr,
                0, nullptr, nullptr,
                1.0);
        }

        prcFile.endgroup();
        prcFile.finish();

        PrcDataWrapper* wrapper = new PrcDataWrapper();
        wrapper->buffer = out.str();

        *outPrc = static_cast<HPrcData>(wrapper);
        outResult->code = RESULT_SUCCESS;
        outResult->errorMessage[0] = '\0';
    } 
    catch (const std::exception& e) {
        outResult->code = RESULT_PRC_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, e.what());
        *outPrc = nullptr;
    }
    catch (...) {
        outResult->code = RESULT_UNKNOWN_ERROR;
        CoreTypes::SafeStrCopy(outResult->errorMessage, "Unknown exception caught during PRC generation.");
        *outPrc = nullptr;
    }
}

void FreePrc(HPrcData prc) {
    if (prc) {
        PrcDataWrapper* wrapper = static_cast<PrcDataWrapper*>(prc);
        delete wrapper;
    }
}

} // extern "C"

#else // Stub for non-Windows

extern "C" {

void WritePrc(HMeshData mesh, PrcSettings /*prcSettings*/, MaterialSettings /*materialSettings*/, HPrcData* outPrc, ExportResult* outResult) {
    if (!mesh || !outPrc || !outResult) return;
    outResult->code = RESULT_PRC_ERROR;
    CoreTypes::SafeStrCopy(outResult->errorMessage, "PRC generation is only supported on Windows.");
    *outPrc = nullptr;
}

void FreePrc(HPrcData prc) {
    // Nothing to free
}

} // extern "C"

#endif // _WIN32
