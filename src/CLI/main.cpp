#include <iostream>
#include "ExportFacade.h"

int main(int argc, char* argv[]) {
    std::cout << "3DPDFConverter CLI" << std::endl;
    std::cout << "This module is currently a stub." << std::endl;
    
    // Example of how it will be called
    if (argc > 2) {
        ExportRequest request = {};
        request.stepFilePath = argv[1];
        request.pdfFilePath = argv[2];
        request.meshSettings.deflection = 0.001;
        request.pdfSettings.pageSize = "A4";

        ExportResult result;
        ExportStepToPdf(request, &result);

        if (result.code == RESULT_SUCCESS) {
            std::cout << "Export successful!" << std::endl;
        } else {
            std::cerr << "Export failed: " << result.errorMessage << std::endl;
        }
    } else {
        std::cout << "Usage: 3DPDFConverterCLI <input.step> <output.pdf>" << std::endl;
    }

    return 0;
}
