#include "3DPDFConverter.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace
{
struct CallbackState
{
    int progressCount = 0;
    int logCount = 0;
    bool cancelAtMesh = false;
};

std::int32_t onProgress(void* userData, std::int32_t stage, double, const char*)
{
    auto& state = *static_cast<CallbackState*>(userData);
    ++state.progressCount;
    return state.cancelAtMesh && stage == CONVERTER_STAGE_GENERATING_MESH ? 1 : 0;
}

void onLog(void* userData, std::int32_t, const char*)
{
    ++static_cast<CallbackState*>(userData)->logCount;
}
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Expected an input STEP path and an output PDF path.\n";
        return 1;
    }

    if (Converter_GetApiVersion() != THREEDPDFCONVERTER_API_VERSION)
    {
        std::cerr << "The runtime API version does not match the public header.\n";
        return 2;
    }

    ConverterOptions options = {};
    Converter_GetDefaultOptions(&options);
    if (options.structSize != sizeof(ConverterOptions) ||
        options.apiVersion != THREEDPDFCONVERTER_API_VERSION)
    {
        std::cerr << "The default options structure is invalid.\n";
        return 3;
    }

    ConverterHandle converter = Converter_Create();
    if (converter == nullptr)
    {
        std::cerr << "Failed to create a converter handle.\n";
        return 4;
    }

    CallbackState callbackState;
    if (Converter_SetCallbacks(converter, &onProgress, &onLog, &callbackState) !=
        CONVERTER_RESULT_SUCCESS)
    {
        Converter_Destroy(converter);
        std::cerr << "Failed to register callbacks.\n";
        return 5;
    }

    const std::filesystem::path inputPath(argv[1]);
    const std::filesystem::path outputPath(argv[2]);
    ConverterResult result = {};
    const std::int32_t code = Converter_ConvertUtf8(
        converter,
        inputPath.string().c_str(),
        outputPath.string().c_str(),
        &options,
        &result);

    if (code != CONVERTER_RESULT_SUCCESS)
    {
        Converter_Destroy(converter);
        std::cerr << "Conversion failed: " << result.messageUtf8 << '\n';
        return 6;
    }

    if (!std::filesystem::exists(outputPath) || std::filesystem::file_size(outputPath) == 0)
    {
        Converter_Destroy(converter);
        std::cerr << "The converter did not create a non-empty PDF.\n";
        return 7;
    }

    std::ifstream output(outputPath, std::ios::binary);
    char pdfHeader[5] = {};
    output.read(pdfHeader, sizeof(pdfHeader));
    if (output.gcount() != static_cast<std::streamsize>(sizeof(pdfHeader)) ||
        std::string(pdfHeader, sizeof(pdfHeader)) != "%PDF-")
    {
        Converter_Destroy(converter);
        std::cerr << "The converter output is not a PDF file.\n";
        return 8;
    }

    if (callbackState.progressCount == 0 || callbackState.logCount == 0)
    {
        Converter_Destroy(converter);
        std::cerr << "The converter did not invoke the registered callbacks.\n";
        return 9;
    }

    callbackState.cancelAtMesh = true;
    callbackState.progressCount = 0;
    callbackState.logCount = 0;
    const std::filesystem::path cancelledOutput = outputPath.parent_path() / "converter_api_cancelled.pdf";
    std::error_code removeError;
    std::filesystem::remove(cancelledOutput, removeError);
    ConverterResult cancelledResult = {};
    const std::int32_t cancelledCode = Converter_ConvertUtf8(
        converter,
        inputPath.string().c_str(),
        cancelledOutput.string().c_str(),
        &options,
        &cancelledResult);
    Converter_Destroy(converter);

    if (cancelledCode != CONVERTER_RESULT_CANCELLED || std::filesystem::exists(cancelledOutput))
    {
        std::cerr << "Callback cancellation did not stop the conversion cleanly.\n";
        return 10;
    }

    return 0;
}
