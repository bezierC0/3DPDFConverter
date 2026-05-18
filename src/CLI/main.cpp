#include <cstring>
#include <iostream>
#include <string>

#include "ExportFacade.h"

namespace {
void printUsage() {
    std::cout
        << "Usage: 3DPDFConverterCLI <input.step> <output.pdf> [options]\n"
        << "Options:\n"
        << "  --page-size <A4|A3|Letter|Legal>\n"
        << "  --bg-color <RRGGBB>\n"
        << "  --projection <perspective|orthographic>\n"
        << "  --fov <deg>\n"
        << "  --roll <deg>\n"
        << "  --radius <value>\n"
        << "  --deflection <value>\n"
        << "  --compression <0..1>\n"
        << "  --granularity <0..1>\n"
        << "  --keep-temp-prc\n";
}

uint32_t parseHexRgb(const std::string& value) {
    return static_cast<uint32_t>(std::stoul(value, nullptr, 16));
}

MaterialSettings defaultMaterial() {
    MaterialSettings material = {};
    material.ambient[0] = 0.2; material.ambient[1] = 0.2; material.ambient[2] = 0.2; material.ambient[3] = 1.0;
    material.diffuse[0] = 0.6; material.diffuse[1] = 0.65; material.diffuse[2] = 0.8; material.diffuse[3] = 1.0;
    material.emissive[0] = 0.0; material.emissive[1] = 0.0; material.emissive[2] = 0.0; material.emissive[3] = 1.0;
    material.specular[0] = 0.8; material.specular[1] = 0.8; material.specular[2] = 0.8; material.specular[3] = 1.0;
    material.alpha = 1.0;
    material.shininess = 32.0;
    return material;
}
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        const std::string firstArg = argv[1];
        if (firstArg == "--help" || firstArg == "-h") {
            printUsage();
            return 0;
        }
    }

    if (argc < 3) {
        printUsage();
        return 1;
    }

    ExportRequest request = {};
    request.apiVersion = 2;
    request.stepFilePath = argv[1];
    request.pdfFilePath = argv[2];

    request.meshSettings.qualityPreset = MESH_QUALITY_CUSTOM;
    request.meshSettings.deflection = 0.01;
    request.meshSettings.angle = 0.5;
    request.meshSettings.relativeMesh = false;
    request.meshSettings.creaseAngle = 25.8419;

    request.prcSettings.compression = 0.0;
    request.prcSettings.granularity = 0.0;
    request.prcSettings.closed = false;
    request.prcSettings.tess = false;
    request.prcSettings.doBreak = true;
    request.prcSettings.noBreak = false;

    request.materialSettings = defaultMaterial();
    request.lineStyleSettings.lineWidth = 1.0;
    request.lineStyleSettings.lineColorRGB = 0x333333;
    request.lineStyleSettings.lineAlpha = 1.0;

    request.pdfSettings.pageSize = "A4";
    request.pdfSettings.backgroundColorRGB = 0xFFFFFF;
    request.pdfSettings.defaultLighting = "CAD";
    request.pdfSettings.defaultView = "Isometric";
    request.pdfSettings.projectionMode = PDF_PROJ_PERSPECTIVE;
    request.pdfSettings.viewPreset = PDF_VIEW_ISO;
    request.pdfSettings.renderMode = PDF_RENDER_SOLID;
    request.pdfSettings.fovDeg = 30.0;
    request.pdfSettings.rollDeg = 0.0;
    request.pdfSettings.orbitRadius = 200.0;
    request.pdfSettings.cameraToCenter[0] = 0.0;
    request.pdfSettings.cameraToCenter[1] = 0.0;
    request.pdfSettings.cameraToCenter[2] = 1.0;
    request.pdfSettings.annotLeftPt = 50.0;
    request.pdfSettings.annotBottomPt = 100.0;
    request.pdfSettings.annotRightPt = 545.0;
    request.pdfSettings.annotTopPt = 792.0;
    request.pdfSettings.keepTempPrc = false;

    request.pdfMetadata.title = "3DPDF Export";
    request.pdfMetadata.author = "3DPDFConverterCLI";
    request.pdfMetadata.subject = "CAD to 3D PDF";
    request.pdfMetadata.keywords = "3DPDF,PRC,CAD";

    for (int i = 3; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--page-size" && i + 1 < argc) request.pdfSettings.pageSize = argv[++i];
        else if (arg == "--bg-color" && i + 1 < argc) request.pdfSettings.backgroundColorRGB = parseHexRgb(argv[++i]);
        else if (arg == "--projection" && i + 1 < argc) {
            const std::string mode = argv[++i];
            request.pdfSettings.projectionMode = (mode == "orthographic") ? PDF_PROJ_ORTHOGRAPHIC : PDF_PROJ_PERSPECTIVE;
        } else if (arg == "--fov" && i + 1 < argc) request.pdfSettings.fovDeg = std::stod(argv[++i]);
        else if (arg == "--roll" && i + 1 < argc) request.pdfSettings.rollDeg = std::stod(argv[++i]);
        else if (arg == "--radius" && i + 1 < argc) request.pdfSettings.orbitRadius = std::stod(argv[++i]);
        else if (arg == "--deflection" && i + 1 < argc) request.meshSettings.deflection = std::stod(argv[++i]);
        else if (arg == "--compression" && i + 1 < argc) request.prcSettings.compression = std::stod(argv[++i]);
        else if (arg == "--granularity" && i + 1 < argc) request.prcSettings.granularity = std::stod(argv[++i]);
        else if (arg == "--keep-temp-prc") {
            request.keepTempPrc = true;
            request.pdfSettings.keepTempPrc = true;
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage();
            return 2;
        }
    }

    ExportResult result = {};
    ExportStepToPdf(request, &result);

    if (result.code == RESULT_SUCCESS) {
        std::cout << "Export successful\n";
        return 0;
    }

    std::cerr << "Export failed: " << result.errorMessage << "\n";
    return 3;
}
