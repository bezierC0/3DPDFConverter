#include "3DPDFConverter.h"

#include "ConversionCallbacks.h"
#include "ExportFacade.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <new>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <Windows.h>
#endif

namespace
{
struct ConverterContext
{
    std::atomic_flag busy = ATOMIC_FLAG_INIT;
    std::atomic_bool cancelRequested { false };
    std::mutex callbackMutex;
    ConverterProgressCallback progressCallback = nullptr;
    ConverterLogCallback logCallback = nullptr;
    void* callbackUserData = nullptr;
};

struct CallbackSnapshot
{
    ConverterContext* context = nullptr;
    ConverterProgressCallback progressCallback = nullptr;
    ConverterLogCallback logCallback = nullptr;
    void* userData = nullptr;
};

void reportProgress(void* userData, std::int32_t stage, double progress, const char* message)
{
    auto& callbacks = *static_cast<CallbackSnapshot*>(userData);
    if (callbacks.progressCallback == nullptr)
    {
        return;
    }

    try
    {
        const std::int32_t cancel = callbacks.progressCallback(
            callbacks.userData, stage, progress, message);
        if (cancel != 0 && stage != CONVERTER_STAGE_COMPLETED)
        {
            callbacks.context->cancelRequested.store(true);
        }
    }
    catch (...)
    {
        callbacks.context->cancelRequested.store(true);
    }
}

void reportLog(void* userData, std::int32_t level, const char* message)
{
    auto& callbacks = *static_cast<CallbackSnapshot*>(userData);
    if (callbacks.logCallback == nullptr)
    {
        return;
    }

    try
    {
        callbacks.logCallback(callbacks.userData, level, message);
    }
    catch (...)
    {
        callbacks.context->cancelRequested.store(true);
    }
}

bool isCancellationRequested(void* userData)
{
    const auto& callbacks = *static_cast<CallbackSnapshot*>(userData);
    return callbacks.context->cancelRequested.load();
}

void copyMessage(char* destination, std::size_t destinationSize, const char* source)
{
    if (destination == nullptr || destinationSize == 0)
    {
        return;
    }

    const char* text = source != nullptr ? source : "";
#if defined(_MSC_VER)
    strncpy_s(destination, destinationSize, text, _TRUNCATE);
#else
    std::strncpy(destination, text, destinationSize - 1);
    destination[destinationSize - 1] = '\0';
#endif
}

void initializeResult(ConverterResult* result)
{
    if (result == nullptr)
    {
        return;
    }

    std::memset(result, 0, sizeof(*result));
    result->structSize = sizeof(*result);
    result->code = CONVERTER_RESULT_UNKNOWN_ERROR;
}

ConverterResultCode mapResultCode(ResultCode code)
{
    switch (code)
    {
    case RESULT_SUCCESS: return CONVERTER_RESULT_SUCCESS;
    case RESULT_INVALID_INPUT: return CONVERTER_RESULT_INVALID_ARGUMENT;
    case RESULT_OCC_ERROR: return CONVERTER_RESULT_CAD_READ_ERROR;
    case RESULT_MESH_ERROR: return CONVERTER_RESULT_MESH_ERROR;
    case RESULT_PRC_ERROR: return CONVERTER_RESULT_PRC_ERROR;
    case RESULT_PDF_ERROR: return CONVERTER_RESULT_PDF_ERROR;
    default: return CONVERTER_RESULT_UNKNOWN_ERROR;
    }
}

bool validateOptions(const ConverterOptions& options, ConverterResult* result)
{
    const bool projectionIsValid =
        options.projectionMode == CONVERTER_PROJECTION_PERSPECTIVE ||
        options.projectionMode == CONVERTER_PROJECTION_ORTHOGRAPHIC;
    if (options.meshDeflection <= 0.0 || options.meshAngleRadians <= 0.0 ||
        options.backgroundColorRgb > 0xFFFFFFu || !projectionIsValid ||
        options.fieldOfViewDegrees <= 0.0 || options.fieldOfViewDegrees >= 180.0 ||
        options.orbitRadius < 0.0)
    {
        result->code = CONVERTER_RESULT_INVALID_ARGUMENT;
        copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "One or more converter options are outside the supported range.");
        return false;
    }
    return true;
}

class BusyGuard
{
public:
    explicit BusyGuard(ConverterContext& context)
        : m_context(context), m_acquired(!m_context.busy.test_and_set())
    {
    }

    ~BusyGuard()
    {
        if (m_acquired)
        {
            m_context.busy.clear();
        }
    }

    bool acquired() const
    {
        return m_acquired;
    }

private:
    ConverterContext& m_context;
    bool m_acquired;
};

#if defined(_WIN32)
std::string narrowWindowsPath(const std::filesystem::path& path)
{
    const std::wstring widePath = path.wstring();
    const int requiredSize = WideCharToMultiByte(
        CP_ACP, WC_NO_BEST_FIT_CHARS, widePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (requiredSize <= 0)
    {
        throw std::runtime_error("Failed to convert the temporary Windows path.");
    }

    std::string narrowPath(static_cast<std::size_t>(requiredSize), '\0');
    BOOL usedDefaultCharacter = FALSE;
    if (WideCharToMultiByte(
            CP_ACP,
            WC_NO_BEST_FIT_CHARS,
            widePath.c_str(),
            -1,
            narrowPath.data(),
            requiredSize,
            nullptr,
            &usedDefaultCharacter) <= 0 || usedDefaultCharacter)
    {
        throw std::runtime_error("The temporary Windows path cannot be represented by the active code page.");
    }
    narrowPath.pop_back();
    return narrowPath;
}

std::filesystem::path shortWindowsPath(const std::filesystem::path& path)
{
    const DWORD requiredSize = GetShortPathNameW(path.c_str(), nullptr, 0);
    if (requiredSize == 0)
    {
        throw std::runtime_error("Failed to resolve the short Windows temporary path.");
    }

    std::wstring shortPath(requiredSize, L'\0');
    const DWORD writtenSize = GetShortPathNameW(path.c_str(), shortPath.data(), requiredSize);
    if (writtenSize == 0 || writtenSize >= requiredSize)
    {
        throw std::runtime_error("Failed to resolve the short Windows temporary path.");
    }
    shortPath.resize(writtenSize);
    return std::filesystem::path(shortPath);
}

class TemporaryFiles
{
public:
    ~TemporaryFiles()
    {
        for (const auto& path : m_paths)
        {
            std::error_code error;
            std::filesystem::remove(path, error);
        }
    }

    void add(const std::filesystem::path& path)
    {
        m_paths.push_back(path);
    }

private:
    std::vector<std::filesystem::path> m_paths;
};
#endif
}

extern "C"
{
uint32_t Converter_GetApiVersion(void)
{
    return THREEDPDFCONVERTER_API_VERSION;
}

void Converter_GetDefaultOptions(ConverterOptions* options)
{
    if (options == nullptr)
    {
        return;
    }

    std::memset(options, 0, sizeof(*options));
    options->structSize = sizeof(*options);
    options->apiVersion = THREEDPDFCONVERTER_API_VERSION;
    options->meshDeflection = 0.01;
    options->meshAngleRadians = 0.5;
    options->backgroundColorRgb = 0xFFFFFF;
    options->projectionMode = CONVERTER_PROJECTION_PERSPECTIVE;
    options->fieldOfViewDegrees = 30.0;
}

ConverterHandle Converter_Create(void)
{
    return new (std::nothrow) ConverterContext();
}

int32_t Converter_SetCallbacks(
    ConverterHandle handle,
    ConverterProgressCallback progressCallback,
    ConverterLogCallback logCallback,
    void* userData)
{
    if (handle == nullptr)
    {
        return CONVERTER_RESULT_INVALID_ARGUMENT;
    }

    auto& context = *static_cast<ConverterContext*>(handle);
    std::lock_guard<std::mutex> lock(context.callbackMutex);
    context.progressCallback = progressCallback;
    context.logCallback = logCallback;
    context.callbackUserData = userData;
    return CONVERTER_RESULT_SUCCESS;
}

void Converter_RequestCancel(ConverterHandle handle)
{
    if (handle != nullptr)
    {
        static_cast<ConverterContext*>(handle)->cancelRequested.store(true);
    }
}

int32_t Converter_ConvertUtf8(
    ConverterHandle handle,
    const char* inputPathUtf8,
    const char* outputPathUtf8,
    const ConverterOptions* options,
    ConverterResult* result)
{
    initializeResult(result);
    if (handle == nullptr || inputPathUtf8 == nullptr || inputPathUtf8[0] == '\0' ||
        outputPathUtf8 == nullptr || outputPathUtf8[0] == '\0' || result == nullptr)
    {
        if (result != nullptr)
        {
            result->code = CONVERTER_RESULT_INVALID_ARGUMENT;
            copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "Invalid converter handle or file path.");
        }
        return CONVERTER_RESULT_INVALID_ARGUMENT;
    }

    ConverterOptions effectiveOptions;
    Converter_GetDefaultOptions(&effectiveOptions);
    if (options != nullptr)
    {
        if (options->structSize != sizeof(ConverterOptions) ||
            options->apiVersion != THREEDPDFCONVERTER_API_VERSION)
        {
            result->code = CONVERTER_RESULT_INVALID_ARGUMENT;
            copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "Unsupported converter options structure or API version.");
            return result->code;
        }
        effectiveOptions = *options;
    }
    if (!validateOptions(effectiveOptions, result))
    {
        return result->code;
    }

    auto& context = *static_cast<ConverterContext*>(handle);
    BusyGuard busyGuard(context);
    if (!busyGuard.acquired())
    {
        result->code = CONVERTER_RESULT_BUSY;
        copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "The converter handle is already processing another request.");
        return result->code;
    }
    context.cancelRequested.store(false);

    CallbackSnapshot callbackSnapshot;
    callbackSnapshot.context = &context;
    {
        std::lock_guard<std::mutex> lock(context.callbackMutex);
        callbackSnapshot.progressCallback = context.progressCallback;
        callbackSnapshot.logCallback = context.logCallback;
        callbackSnapshot.userData = context.callbackUserData;
    }
    ConversionCallbacks callbacks;
    callbacks.userData = &callbackSnapshot;
    callbacks.reportProgress = &reportProgress;
    callbacks.reportLog = &reportLog;
    callbacks.isCancellationRequested = &isCancellationRequested;

    reportProgress(&callbackSnapshot, CONVERTER_STAGE_STARTING, 0.0, "Starting conversion.");
    reportLog(&callbackSnapshot, CONVERTER_LOG_INFO, "Starting conversion.");

    ExportRequest request = {};
    request.apiVersion = 2;
    request.stepFilePath = inputPathUtf8;
    request.pdfFilePath = outputPathUtf8;
    request.meshSettings.qualityPreset = MESH_QUALITY_CUSTOM;
    request.meshSettings.deflection = effectiveOptions.meshDeflection;
    request.meshSettings.angle = effectiveOptions.meshAngleRadians;
    request.meshSettings.relativeMesh = effectiveOptions.relativeMesh != 0;
    request.materialSettings.ambient[0] = 0.2;
    request.materialSettings.ambient[1] = 0.2;
    request.materialSettings.ambient[2] = 0.2;
    request.materialSettings.ambient[3] = 1.0;
    request.materialSettings.diffuse[0] = 0.6;
    request.materialSettings.diffuse[1] = 0.65;
    request.materialSettings.diffuse[2] = 0.8;
    request.materialSettings.diffuse[3] = 1.0;
    request.materialSettings.specular[0] = 0.8;
    request.materialSettings.specular[1] = 0.8;
    request.materialSettings.specular[2] = 0.8;
    request.materialSettings.specular[3] = 1.0;
    request.materialSettings.alpha = 1.0;
    request.materialSettings.shininess = 32.0;
    request.pdfSettings.pageSize = "A4";
    request.pdfSettings.backgroundColorRGB = effectiveOptions.backgroundColorRgb;
    request.pdfSettings.defaultLighting = "CAD";
    request.pdfSettings.defaultView = "Isometric";
    request.pdfSettings.projectionMode = effectiveOptions.projectionMode == CONVERTER_PROJECTION_ORTHOGRAPHIC
        ? PDF_PROJ_ORTHOGRAPHIC
        : PDF_PROJ_PERSPECTIVE;
    request.pdfSettings.fovDeg = effectiveOptions.fieldOfViewDegrees;
    request.pdfSettings.rollDeg = effectiveOptions.cameraRollDegrees;
    request.pdfSettings.orbitRadius = effectiveOptions.orbitRadius;
    request.pdfSettings.cameraToCenter[2] = 1.0;
    request.pdfSettings.annotLeftPt = 50.0;
    request.pdfSettings.annotBottomPt = 100.0;
    request.pdfSettings.annotRightPt = 545.0;
    request.pdfSettings.annotTopPt = 792.0;
    request.pdfSettings.keepTempPrc = effectiveOptions.keepTemporaryPrc != 0;

    std::error_code existenceError;
    const bool outputExistedBefore = std::filesystem::exists(outputPathUtf8, existenceError);
    ExportResult legacyResult = {};
    ExportStepToPdfWithCallbacks(request, &legacyResult, &callbacks);
    if (context.cancelRequested.load())
    {
        if (!outputExistedBefore)
        {
            std::error_code removeError;
            std::filesystem::remove(outputPathUtf8, removeError);
        }
        result->code = CONVERTER_RESULT_CANCELLED;
        copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "Conversion cancelled.");
        return result->code;
    }
    result->code = mapResultCode(legacyResult.code);
    copyMessage(result->messageUtf8, sizeof(result->messageUtf8), legacyResult.errorMessage);
    if (result->code != CONVERTER_RESULT_SUCCESS)
    {
        reportLog(&callbackSnapshot, CONVERTER_LOG_ERROR, result->messageUtf8);
    }
    return result->code;
}

#if defined(_WIN32)
int32_t Converter_ConvertWide(
    ConverterHandle handle,
    const wchar_t* inputPath,
    const wchar_t* outputPath,
    const ConverterOptions* options,
    ConverterResult* result)
{
    initializeResult(result);
    if (handle == nullptr || inputPath == nullptr || inputPath[0] == L'\0' ||
        outputPath == nullptr || outputPath[0] == L'\0' || result == nullptr)
    {
        if (result != nullptr)
        {
            result->code = CONVERTER_RESULT_INVALID_ARGUMENT;
            copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "Invalid converter handle or file path.");
        }
        return CONVERTER_RESULT_INVALID_ARGUMENT;
    }

    try
    {
        const std::filesystem::path sourcePath(inputPath);
        const std::filesystem::path destinationPath(outputPath);
        if (!std::filesystem::exists(sourcePath))
        {
            result->code = CONVERTER_RESULT_INVALID_ARGUMENT;
            copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "The input CAD file does not exist.");
            return result->code;
        }

        static std::atomic<unsigned long long> sequence { 0 };
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const std::string token = std::to_string(timestamp) + '-' + std::to_string(sequence.fetch_add(1));
        const std::filesystem::path temporaryDirectory = std::filesystem::temp_directory_path();
        const std::filesystem::path temporaryInput = temporaryDirectory / ("3dpdfconverter-" + token + ".step");
        const std::filesystem::path temporaryOutput = temporaryDirectory / ("3dpdfconverter-" + token + ".pdf");
        std::filesystem::path temporaryPrc = temporaryOutput;
        temporaryPrc += L".prc";

        TemporaryFiles temporaryFiles;
        temporaryFiles.add(temporaryInput);
        temporaryFiles.add(temporaryOutput);
        temporaryFiles.add(temporaryPrc);
        std::filesystem::copy_file(sourcePath, temporaryInput, std::filesystem::copy_options::overwrite_existing);

        const std::filesystem::path shortTemporaryDirectory = shortWindowsPath(temporaryDirectory);
        const std::string narrowTemporaryDirectory = narrowWindowsPath(shortTemporaryDirectory);
        const std::string narrowInput = narrowTemporaryDirectory + "\\" + temporaryInput.filename().string();
        const std::string narrowOutput = narrowTemporaryDirectory + "\\" + temporaryOutput.filename().string();

        ConverterOptions effectiveOptions;
        Converter_GetDefaultOptions(&effectiveOptions);
        if (options != nullptr)
        {
            effectiveOptions = *options;
        }
        const bool keepTemporaryPrc = effectiveOptions.keepTemporaryPrc != 0;

        const int32_t code = Converter_ConvertUtf8(
            handle, narrowInput.c_str(), narrowOutput.c_str(), &effectiveOptions, result);
        if (code != CONVERTER_RESULT_SUCCESS)
        {
            return code;
        }

        std::filesystem::copy_file(
            temporaryOutput, destinationPath, std::filesystem::copy_options::overwrite_existing);
        if (keepTemporaryPrc && std::filesystem::exists(temporaryPrc))
        {
            std::filesystem::path destinationPrc = destinationPath;
            destinationPrc += L".prc";
            std::filesystem::copy_file(
                temporaryPrc, destinationPrc, std::filesystem::copy_options::overwrite_existing);
        }
        return result->code;
    }
    catch (const std::exception& error)
    {
        result->code = CONVERTER_RESULT_UNKNOWN_ERROR;
        copyMessage(result->messageUtf8, sizeof(result->messageUtf8), error.what());
        return result->code;
    }
    catch (...)
    {
        result->code = CONVERTER_RESULT_UNKNOWN_ERROR;
        copyMessage(result->messageUtf8, sizeof(result->messageUtf8), "Unknown error while processing Windows paths.");
        return result->code;
    }
}
#endif

void Converter_Destroy(ConverterHandle handle)
{
    delete static_cast<ConverterContext*>(handle);
}
}
