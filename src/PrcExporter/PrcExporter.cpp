#include "PrcExporter.h"
#include "CoreTypes.h"
#include <oPRCFile.h>
#include <exception>
#include <string.h>
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

void WritePrc(HMeshData mesh, HPrcData* outPrc, ExportResult* outResult) {
    if (!mesh || !outPrc || !outResult) return;

    try {
        MeshDataWrapper* meshData = static_cast<MeshDataWrapper*>(mesh);
        
        std::ostringstream out(std::ios::binary);
        oPRCFile prcFile(out);
        prcFile.begingroup("model");

        PRCmaterial mat(
            RGBAColour(0.2, 0.2, 0.2, 1.0),
            RGBAColour(0.6, 0.65, 0.8, 1.0),
            RGBAColour(0.0, 0.0, 0.0, 1.0),
            RGBAColour(0.8, 0.8, 0.8, 1.0),
            1.0,
            32.0
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
