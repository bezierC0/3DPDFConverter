# CoreTypes Library

## Overview

CoreTypes is a header-only library that provides shared data structures, enumerations, and result codes for inter-module communication in the STEP to 3D PDF Converter project.

## Purpose

This library serves as the foundation for all module interfaces, ensuring:
- **Module Independence**: No dependencies on external libraries (OpenCASCADE, Qt, etc.)
- **Binary Compatibility**: POD (Plain Old Data) structures for cross-compiler compatibility
- **Type Safety**: Strongly-typed enumerations for configuration options
- **Clean API Boundaries**: Standard C++ types only (std::string, std::vector, primitives)

## Requirements Validation

This library validates the following requirements:
- **Requirement 6.1**: POD structures for ExportSettings, MeshSettings, PdfSettings
- **Requirement 6.2**: ResultCode enumeration
- **Requirement 6.3**: ErrorMessage as std::string
- **Requirement 6.4**: No OpenCASCADE, Qt, or third-party library types
- **Requirement 6.5**: Header-only or minimal DLL with no runtime dependencies
- **Requirement 6.6**: PageSize enumeration
- **Requirement 6.7**: LightingMode enumeration
- **Requirement 6.8**: ViewAngle enumeration

## Usage

### Including the Library

```cpp
#include "CoreTypes.h"

using namespace CoreTypes;
```

### CMake Integration

As an INTERFACE library (header-only):

```cmake
# In your CMakeLists.txt
add_subdirectory(src/CoreTypes)

# Link against CoreTypes
target_link_libraries(YourTarget PRIVATE CoreTypes)
```

## Data Types

### Enumerations

#### ResultCode
Operation result codes for error handling:
- `Success` (0): Operation completed successfully
- `FileNotFound` (1): Input file does not exist
- `InvalidFormat` (2): File format is invalid or corrupted
- `MeshGenerationFailed` (3): Triangulation failed
- `PrcExportFailed` (4): PRC generation failed
- `PdfExportFailed` (5): PDF creation failed
- `UnknownError` (99): Unexpected error occurred

#### PageSize
PDF page size options:
- `A4`: ISO A4 (210 × 297 mm)
- `A3`: ISO A3 (297 × 420 mm)
- `Letter`: US Letter (8.5 × 11 inches)
- `Legal`: US Legal (8.5 × 14 inches)

#### LightingMode
3D viewport lighting modes:
- `Artwork`: Soft artistic lighting
- `CAD`: Technical CAD lighting
- `Headlamp`: Camera-mounted light
- `None`: No lighting

#### ViewAngle
Default camera view angles:
- `Front`: Front view
- `Back`: Back view
- `Top`: Top view
- `Bottom`: Bottom view
- `Left`: Left view
- `Right`: Right view
- `Isometric`: Isometric view

### Structures

#### MeshSettings
Mesh generation configuration (POD structure):

```cpp
struct MeshSettings {
    double deflection = 0.001;      // Linear deflection (mm)
    double angle = 0.5;              // Angular deflection (radians)
    bool relative = false;           // Use relative deflection
    bool inParallel = true;          // Enable parallel processing
};
```

**Example:**
```cpp
MeshSettings meshSettings;
meshSettings.deflection = 0.01;
meshSettings.angle = 0.3;
meshSettings.relative = true;
```

#### PdfSettings
PDF export configuration (POD structure):

```cpp
struct PdfSettings {
    PageSize pageSize = PageSize::A4;
    LightingMode lighting = LightingMode::CAD;
    ViewAngle defaultView = ViewAngle::Isometric;
    uint32_t backgroundColor = 0xFFFFFF;  // RGB color (0xRRGGBB)
};
```

**Example:**
```cpp
PdfSettings pdfSettings;
pdfSettings.pageSize = PageSize::A3;
pdfSettings.lighting = LightingMode::Headlamp;
pdfSettings.defaultView = ViewAngle::Front;
pdfSettings.backgroundColor = 0x000000;  // Black background
```

#### ExportSettings
Complete export configuration (POD structure):

```cpp
struct ExportSettings {
    MeshSettings meshSettings;
    PdfSettings pdfSettings;
};
```

**Example:**
```cpp
ExportSettings settings;
settings.meshSettings.deflection = 0.005;
settings.pdfSettings.pageSize = PageSize::Letter;
```

#### OperationResult
Operation result with error details:

```cpp
struct OperationResult {
    ResultCode code;
    std::string message;
    
    bool IsSuccess() const;
};
```

**Example:**
```cpp
OperationResult result;
result.code = ResultCode::Success;
result.message = "";

if (result.IsSuccess()) {
    // Handle success
} else {
    // Handle error: result.message contains details
}
```

## Testing

A verification test is provided to ensure all types compile and work correctly.

### Building the Test

```bash
cmake -DBUILD_CORETYPES_TEST=ON ..
cmake --build .
```

### Running the Test

```bash
./test_coretypes
```

Or compile directly:

```bash
g++ -std=c++17 -I src/CoreTypes src/CoreTypes/test_coretypes.cpp -o test_coretypes
./test_coretypes
```

## Design Principles

1. **Header-Only**: No compilation required, just include the header
2. **POD Structures**: Binary compatible across compiler boundaries
3. **No Dependencies**: Only standard C++ library (std::string, std::cstdint)
4. **Type Safety**: Strongly-typed enumerations (enum class)
5. **Default Values**: Sensible defaults for all configuration options

## Module Dependencies

CoreTypes has **zero dependencies** on:
- OpenCASCADE
- Qt
- libPRC
- libHaru
- Any other third-party libraries

This ensures it can be used by all modules without introducing circular dependencies.

## Future Extensions

When adding new types to CoreTypes:
1. Ensure they are POD structures or standard C++ types
2. Provide default values for all fields
3. Document the purpose and usage
4. Update the verification test
5. Maintain zero external dependencies

## License

See the project LICENSE file for details.
