#include "3DPDFConverter.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <cstdint>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>

#if defined(_WIN32)
#include <Windows.h>
#include <shellapi.h>
#endif

namespace
{
std::int32_t printProgress(void*, std::int32_t, double progress, const char* message)
{
    const int percent = static_cast<int>(progress * 100.0 + 0.5);
    std::cerr << '[' << percent << "%] " << message << '\n';
    return 0;
}

void printUsage()
{
    std::cout
        << "Usage: 3DPDFConverterCLI <input.step> <output.pdf> [options]\n"
        << "Options:\n"
        << "  --bg-color <RRGGBB>\n"
        << "  --projection <perspective|orthographic>\n"
        << "  --fov <degrees>\n"
        << "  --roll <degrees>\n"
        << "  --radius <value>\n"
        << "  --deflection <value>\n"
        << "  --angle <radians>\n"
        << "  --relative-mesh\n"
        << "  --keep-temp-prc\n"
        << "  --params <settings.json>\n"
        << "  --version\n"
        << "  --help\n";
}

std::uint32_t parseHexRgb(const std::string& value)
{
    if (value.size() != 6)
    {
        throw std::invalid_argument("RGB color must contain exactly six hexadecimal digits.");
    }

    std::size_t consumed = 0;
    const unsigned long parsed = std::stoul(value, &consumed, 16);
    if (consumed != value.size() || parsed > 0xFFFFFFul)
    {
        throw std::invalid_argument("Invalid RGB color.");
    }
    return static_cast<std::uint32_t>(parsed);
}

double parsePositiveDouble(const char* option, const char* value)
{
    std::size_t consumed = 0;
    const std::string text(value);
    const double parsed = std::stod(text, &consumed);
    if (consumed != text.size() || parsed <= 0.0)
    {
        throw std::invalid_argument(std::string(option) + " requires a positive number.");
    }
    return parsed;
}

double parseDouble(const char* option, const char* value)
{
    std::size_t consumed = 0;
    const std::string text(value);
    const double parsed = std::stod(text, &consumed);
    if (consumed != text.size())
    {
        throw std::invalid_argument(std::string(option) + " requires a number.");
    }
    return parsed;
}

const char* requireValue(int argc, char* argv[], int& index)
{
    if (index + 1 >= argc)
    {
        throw std::invalid_argument(std::string("Missing value for ") + argv[index] + '.');
    }
    return argv[++index];
}

void requireKnownMembers(
    const rapidjson::Value& object,
    const std::unordered_set<std::string>& knownMembers,
    const char* objectName)
{
    for (auto member = object.MemberBegin(); member != object.MemberEnd(); ++member)
    {
        if (knownMembers.find(member->name.GetString()) == knownMembers.end())
        {
            throw std::invalid_argument(
                std::string("Unknown JSON field in ") + objectName + ": " + member->name.GetString());
        }
    }
}

void loadOptionsJson(const char* filePath, ConverterOptions& options)
{
    std::ifstream input(filePath, std::ios::binary);
    if (!input)
    {
        throw std::invalid_argument(std::string("Could not open JSON settings file: ") + filePath);
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string json = buffer.str();
    rapidjson::Document document;
    document.Parse(json.c_str(), json.size());
    if (document.HasParseError())
    {
        throw std::invalid_argument(
            std::string("Invalid JSON at offset ") + std::to_string(document.GetErrorOffset()) +
            ": " + rapidjson::GetParseError_En(document.GetParseError()));
    }
    if (!document.IsObject())
    {
        throw std::invalid_argument("The JSON settings root must be an object.");
    }

    requireKnownMembers(document, { "schemaVersion", "mesh", "pdf", "keepTemporaryPrc" }, "root");
    if (document.HasMember("schemaVersion"))
    {
        if (!document["schemaVersion"].IsUint() || document["schemaVersion"].GetUint() != 1)
        {
            throw std::invalid_argument("Only JSON schemaVersion 1 is supported.");
        }
    }

    if (document.HasMember("mesh"))
    {
        const rapidjson::Value& mesh = document["mesh"];
        if (!mesh.IsObject())
        {
            throw std::invalid_argument("JSON field mesh must be an object.");
        }
        requireKnownMembers(mesh, { "deflection", "angleRadians", "relative" }, "mesh");
        if (mesh.HasMember("deflection"))
        {
            if (!mesh["deflection"].IsNumber() || mesh["deflection"].GetDouble() <= 0.0)
                throw std::invalid_argument("mesh.deflection must be a positive number.");
            options.meshDeflection = mesh["deflection"].GetDouble();
        }
        if (mesh.HasMember("angleRadians"))
        {
            if (!mesh["angleRadians"].IsNumber() || mesh["angleRadians"].GetDouble() <= 0.0)
                throw std::invalid_argument("mesh.angleRadians must be a positive number.");
            options.meshAngleRadians = mesh["angleRadians"].GetDouble();
        }
        if (mesh.HasMember("relative"))
        {
            if (!mesh["relative"].IsBool())
                throw std::invalid_argument("mesh.relative must be a Boolean value.");
            options.relativeMesh = mesh["relative"].GetBool() ? 1u : 0u;
        }
    }

    if (document.HasMember("pdf"))
    {
        const rapidjson::Value& pdf = document["pdf"];
        if (!pdf.IsObject())
        {
            throw std::invalid_argument("JSON field pdf must be an object.");
        }
        requireKnownMembers(pdf, { "background", "projection", "fieldOfViewDegrees", "rollDegrees", "orbitRadius" }, "pdf");
        if (pdf.HasMember("background"))
        {
            if (!pdf["background"].IsString())
                throw std::invalid_argument("pdf.background must be an RRGGBB string.");
            options.backgroundColorRgb = parseHexRgb(pdf["background"].GetString());
        }
        if (pdf.HasMember("projection"))
        {
            if (!pdf["projection"].IsString())
                throw std::invalid_argument("pdf.projection must be a string.");
            const std::string projection(pdf["projection"].GetString());
            if (projection == "perspective") options.projectionMode = CONVERTER_PROJECTION_PERSPECTIVE;
            else if (projection == "orthographic") options.projectionMode = CONVERTER_PROJECTION_ORTHOGRAPHIC;
            else throw std::invalid_argument("pdf.projection must be perspective or orthographic.");
        }
        if (pdf.HasMember("fieldOfViewDegrees"))
        {
            if (!pdf["fieldOfViewDegrees"].IsNumber() || pdf["fieldOfViewDegrees"].GetDouble() <= 0.0)
                throw std::invalid_argument("pdf.fieldOfViewDegrees must be a positive number.");
            options.fieldOfViewDegrees = pdf["fieldOfViewDegrees"].GetDouble();
        }
        if (pdf.HasMember("rollDegrees"))
        {
            if (!pdf["rollDegrees"].IsNumber())
                throw std::invalid_argument("pdf.rollDegrees must be a number.");
            options.cameraRollDegrees = pdf["rollDegrees"].GetDouble();
        }
        if (pdf.HasMember("orbitRadius"))
        {
            if (!pdf["orbitRadius"].IsNumber() || pdf["orbitRadius"].GetDouble() <= 0.0)
                throw std::invalid_argument("pdf.orbitRadius must be a positive number.");
            options.orbitRadius = pdf["orbitRadius"].GetDouble();
        }
    }

    if (document.HasMember("keepTemporaryPrc"))
    {
        if (!document["keepTemporaryPrc"].IsBool())
            throw std::invalid_argument("keepTemporaryPrc must be a Boolean value.");
        options.keepTemporaryPrc = document["keepTemporaryPrc"].GetBool() ? 1u : 0u;
    }
}

int exitCodeFor(std::int32_t code)
{
    switch (code)
    {
    case CONVERTER_RESULT_SUCCESS: return 0;
    case CONVERTER_RESULT_INVALID_ARGUMENT: return 1;
    case CONVERTER_RESULT_CAD_READ_ERROR: return 3;
    case CONVERTER_RESULT_MESH_ERROR: return 4;
    case CONVERTER_RESULT_PRC_ERROR: return 5;
    case CONVERTER_RESULT_PDF_ERROR: return 6;
    case CONVERTER_RESULT_BUSY: return 7;
    default: return 99;
    }
}
}

int main(int argc, char* argv[])
{
    if (argc >= 2)
    {
        const std::string firstArgument(argv[1]);
        if (firstArgument == "--help" || firstArgument == "-h")
        {
            printUsage();
            return 0;
        }
        if (firstArgument == "--version")
        {
            std::cout << "3DPDFConverter API " << Converter_GetApiVersion() << '\n';
            return 0;
        }
    }

    if (argc < 3)
    {
        printUsage();
        return 1;
    }

    ConverterOptions options = {};
    Converter_GetDefaultOptions(&options);

    try
    {
        for (int index = 3; index < argc; ++index)
        {
            const std::string argument(argv[index]);
            if (argument == "--bg-color")
            {
                options.backgroundColorRgb = parseHexRgb(requireValue(argc, argv, index));
            }
            else if (argument == "--projection")
            {
                const std::string value(requireValue(argc, argv, index));
                if (value == "perspective")
                {
                    options.projectionMode = CONVERTER_PROJECTION_PERSPECTIVE;
                }
                else if (value == "orthographic")
                {
                    options.projectionMode = CONVERTER_PROJECTION_ORTHOGRAPHIC;
                }
                else
                {
                    throw std::invalid_argument("Projection must be perspective or orthographic.");
                }
            }
            else if (argument == "--fov")
            {
                options.fieldOfViewDegrees = parsePositiveDouble("--fov", requireValue(argc, argv, index));
            }
            else if (argument == "--roll")
            {
                options.cameraRollDegrees = parseDouble("--roll", requireValue(argc, argv, index));
            }
            else if (argument == "--radius")
            {
                options.orbitRadius = parsePositiveDouble("--radius", requireValue(argc, argv, index));
            }
            else if (argument == "--deflection")
            {
                options.meshDeflection = parsePositiveDouble("--deflection", requireValue(argc, argv, index));
            }
            else if (argument == "--angle")
            {
                options.meshAngleRadians = parsePositiveDouble("--angle", requireValue(argc, argv, index));
            }
            else if (argument == "--relative-mesh")
            {
                options.relativeMesh = 1;
            }
            else if (argument == "--keep-temp-prc")
            {
                options.keepTemporaryPrc = 1;
            }
            else if (argument == "--params")
            {
                loadOptionsJson(requireValue(argc, argv, index), options);
            }
            else
            {
                throw std::invalid_argument("Unknown option: " + argument);
            }
        }
    }
    catch (const std::exception& error)
    {
        std::cerr << "Argument error: " << error.what() << '\n';
        return 1;
    }

    ConverterHandle converter = Converter_Create();
    if (converter == nullptr)
    {
        std::cerr << "Failed to create the converter.\n";
        return 99;
    }
    Converter_SetCallbacks(converter, &printProgress, nullptr, nullptr);

    ConverterResult result = {};
#if defined(_WIN32)
    int wideArgumentCount = 0;
    wchar_t** wideArguments = CommandLineToArgvW(GetCommandLineW(), &wideArgumentCount);
    if (wideArguments == nullptr || wideArgumentCount < 3)
    {
        if (wideArguments != nullptr)
        {
            LocalFree(wideArguments);
        }
        Converter_Destroy(converter);
        std::cerr << "Failed to read the Unicode command line.\n";
        return 99;
    }
    const std::int32_t code = Converter_ConvertWide(
        converter,
        wideArguments[1],
        wideArguments[2],
        &options,
        &result);
    LocalFree(wideArguments);
#else
    const std::int32_t code = Converter_ConvertUtf8(
        converter,
        argv[1],
        argv[2],
        &options,
        &result);
#endif
    Converter_Destroy(converter);

    if (code == CONVERTER_RESULT_SUCCESS)
    {
        std::cout << "Export successful\n";
        return 0;
    }

    std::cerr << "Export failed: " << result.messageUtf8 << '\n';
    return exitCodeFor(code);
}
