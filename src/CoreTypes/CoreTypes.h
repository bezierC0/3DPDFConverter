#ifndef CORETYPES_H
#define CORETYPES_H

#include <string>
#include <cstdint>

#include <string_view>
#include <algorithm>

namespace CoreTypes {

/**
 * @brief Safely copy a string to a fixed-size char buffer with null-termination.
 * 
 * Uses modern C++ methods (string_view, algorithm) to replace unsafe strncpy.
 */
template <size_t N>
inline void SafeStrCopy(char (&dest)[N], std::string_view src) {
    const size_t count = std::min(N - 1, src.size());
    if (count > 0) {
        std::copy_n(src.data(), count, dest);
    }
    dest[count] = '\0';
}

/**
 * @brief Result codes for all operations
 * 
 * Validates: Requirements 6.2
 */
enum class ResultCode {
    Success = 0,
    FileNotFound = 1,
    InvalidFormat = 2,
    MeshGenerationFailed = 3,
    PrcExportFailed = 4,
    PdfExportFailed = 5,
    UnknownError = 99
};

/**
 * @brief Page size options for PDF export
 * 
 * Validates: Requirements 6.6
 */
enum class PageSize {
    A4,
    A3,
    Letter,
    Legal
};

/**
 * @brief Lighting modes for 3D viewport in PDF
 * 
 * Validates: Requirements 6.7
 */
enum class LightingMode {
    Artwork,
    CAD,
    Headlamp,
    None
};

/**
 * @brief Default camera view angles for 3D viewport
 * 
 * Validates: Requirements 6.8
 */
enum class ViewAngle {
    Front,
    Back,
    Top,
    Bottom,
    Left,
    Right,
    Isometric
};

/**
 * @brief Mesh generation settings (POD structure)
 * 
 * Validates: Requirements 6.1
 */
struct MeshSettings {
    double deflection = 0.001;      ///< Linear deflection (mm)
    double angle = 0.5;              ///< Angular deflection (radians)
    bool relative = false;           ///< Use relative deflection
    bool inParallel = true;          ///< Enable parallel processing
};

/**
 * @brief PDF export settings (POD structure)
 * 
 * Validates: Requirements 6.1
 */
struct PdfSettings {
    PageSize pageSize = PageSize::A4;
    LightingMode lighting = LightingMode::CAD;
    ViewAngle defaultView = ViewAngle::Isometric;
    uint32_t backgroundColor = 0xFFFFFF;  ///< RGB color (0xRRGGBB)
};

/**
 * @brief Complete export configuration (POD structure)
 * 
 * Validates: Requirements 6.1
 */
struct ExportSettings {
    MeshSettings meshSettings;
    PdfSettings pdfSettings;
};

/**
 * @brief Operation result with error details
 * 
 * Validates: Requirements 6.2, 6.3
 */
struct OperationResult {
    ResultCode code;
    std::string message;
    
    /**
     * @brief Check if operation was successful
     * @return true if code is Success, false otherwise
     */
    bool IsSuccess() const { 
        return code == ResultCode::Success; 
    }
};

} // namespace CoreTypes

#endif // CORETYPES_H
