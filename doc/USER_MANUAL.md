# 3DPDFConverter User Manual

## 1. Overview

3DPDFConverter converts STEP CAD files into PRC-based 3D PDF documents. It supports two integration styles:

- Use `3DPDFConverterCLI.exe` for scripts, batch jobs, external tools, and process isolation.
- Use `3DPDFConverter.dll` and `3DPDFConverter.h` for direct application integration.

Both interfaces use the same conversion implementation. The optional Qt GUI is a validation tool and is not part of the conversion core.

## 2. Current capabilities

- Read STEP files with OpenCASCADE.
- Tessellate B-Rep faces with configurable linear and angular deflection.
- Write triangle data into a PRC stream.
- Embed PRC data into an A4 PDF page.
- Configure background color, projection, field of view, camera roll, and orbit radius.
- Derive the default camera center and radius from the model bounds.
- Accept Unicode input and output paths through the Windows wide-character API and CLI.
- Accept command-line settings or a JSON settings file.

Current limitations:

- STEP is the only implemented CAD input format.
- The output contains tessellated geometry with one material; STEP assembly names, colors, and product structure are not preserved yet.
- PRC compression and advanced line rendering are not implemented.
- PRC-based PDF generation is currently verified on Windows only.
- Static PDF rendering may appear blank because the document does not yet contain a poster image. A 3D-PDF-capable reader is required to activate and inspect the model.

## 3. Build configuration

### 3.1 Main CMake options

| Option | Default | Description |
|---|---:|---|
| `BUILD_CONVERTER_SHARED` | `ON` | Builds the `3DPDFConverter` shared library. |
| `BUILD_CONVERTER_CLI` | `ON` | Builds the command-line application. |
| `BUILD_GUI_VALIDATOR` | `OFF` | Builds the optional Qt GUI validation application. |
| `BUILD_TESTS` | `OFF` | Builds the API and conversion smoke test. |
| `IS_COPY_DLLS` | `ON` | Copies OpenCASCADE and third-party DLLs after building the CLI. |

The CLI requires the shared library. CMake reports an error if `BUILD_CONVERTER_CLI=ON` and `BUILD_CONVERTER_SHARED=OFF` are requested together.

### 3.2 Windows build

Run the build from an x64 Visual Studio developer environment:

```powershell
cmake -S . -B build `
  -DOpenCASCADE_DIR=C:/Libs/OCCT/occt-7.8.0-vc14-64 `
  -DOpenCASCADE_3RDPARTY_DIR=C:/Libs/OCCT/3rdparty-vc14-64 `
  -DBUILD_CONVERTER_SHARED=ON `
  -DBUILD_CONVERTER_CLI=ON `
  -DBUILD_GUI_VALIDATOR=OFF `
  -DIS_COPY_DLLS=ON

cmake --build build --config Release -- -j1
```

### 3.3 Build the GUI validator

Qt is required only when the GUI validator is enabled:

```powershell
cmake -S . -B build `
  -DBUILD_GUI_VALIDATOR=ON `
  -DQt5_DIR=C:/Qt/5.15.17/msvc2019-x86_64/lib/cmake/Qt5
```

The converter library and CLI do not link Qt.

### 3.4 Runtime DLL deployment

The deployment behavior follows the method used by the verified Qt_OCC project:

```cmake
option(IS_COPY_DLLS "Copy OpenCASCADE and third-party runtime DLLs" ON)
```

When enabled, the build copies OpenCASCADE, its selected third-party runtime DLLs, libPRC, libHaru, and zlib beside the CLI by using `cmake -E copy_if_different`.

This is a reliability-first deployment mode and is not a minimum-size package. With the currently configured OpenCASCADE 7.8 binary distribution, a verified Release output contained 108 EXE/DLL files totaling approximately 98.76 MiB. The OpenCASCADE `TKDESTEP` dependency chain includes XCAF and visualization libraries in this prebuilt distribution, which also pulls libraries such as FreeImage, FFmpeg, and OpenVR.

Use `IS_COPY_DLLS=OFF` when:

- running inside a configured developer environment;
- the host product already deploys the matching OpenCASCADE runtime;
- building an SDK package with separately managed dependencies.

A substantially smaller standalone package requires a separately built and trimmed OpenCASCADE distribution. It cannot be achieved reliably by omitting DLLs from this prebuilt runtime.

## 4. Command-line interface

### 4.1 Syntax

```text
3DPDFConverterCLI <input.step> <output.pdf> [options]
```

### 4.2 Options

| Option | Description |
|---|---|
| `--bg-color RRGGBB` | Sets the 3D view background color. |
| `--projection perspective\|orthographic` | Selects the PDF camera projection. |
| `--fov degrees` | Sets the perspective field of view. |
| `--roll degrees` | Sets the camera roll. |
| `--radius value` | Overrides the model-derived orbit radius. |
| `--deflection value` | Sets the positive linear mesh deflection. |
| `--angle radians` | Sets the positive angular mesh deflection. |
| `--relative-mesh` | Enables relative OpenCASCADE meshing. |
| `--keep-temp-prc` | Keeps the intermediate PRC beside the output PDF. |
| `--params file.json` | Loads schema version 1 JSON settings. |
| `--version` | Prints the converter API version. |
| `--help` | Prints command help. |

Options are processed from left to right. A later command-line option overrides a value loaded by an earlier `--params` option.

### 4.3 Examples

Basic conversion:

```powershell
3DPDFConverterCLI.exe model.step model.pdf
```

Configured conversion:

```powershell
3DPDFConverterCLI.exe model.step model.pdf `
  --projection orthographic `
  --bg-color DFE8FF `
  --deflection 0.02 `
  --angle 0.5
```

Unicode paths are supported by the Windows CLI:

```powershell
3DPDFConverterCLI.exe "☑あう１①②③えイ.step" "☑あう１①②③えイ.pdf"
```

### 4.4 Exit codes

| Exit code | Meaning |
|---:|---|
| `0` | Conversion succeeded. |
| `1` | Invalid argument or JSON setting. |
| `3` | CAD input could not be read. |
| `4` | Meshing failed. |
| `5` | PRC generation failed. |
| `6` | PDF generation failed. |
| `7` | The converter handle was already busy. |
| `99` | Unknown internal or platform error. |

## 5. JSON settings

The CLI uses RapidJSON and rejects unknown fields instead of silently ignoring them.

```json
{
  "schemaVersion": 1,
  "mesh": {
    "deflection": 0.02,
    "angleRadians": 0.5,
    "relative": false
  },
  "pdf": {
    "background": "DFE8FF",
    "projection": "orthographic",
    "fieldOfViewDegrees": 30.0,
    "rollDegrees": 0.0,
    "orbitRadius": 200.0
  },
  "keepTemporaryPrc": false
}
```

`orbitRadius` may be omitted to use the radius calculated from the model bounds.

## 6. Shared-library API

### 6.1 Public ABI rules

The public header is `src/Converter/3DPDFConverter.h`. It exposes a C ABI and does not expose OpenCASCADE, Qt, STL containers, or C++ classes.

The public structures use:

- fixed-width integer fields;
- `structSize` for structure validation;
- `apiVersion` for compatibility checks;
- reserved zero-initialized fields for future expansion;
- an opaque `ConverterHandle` for lifecycle ownership.

Exceptions never cross the shared-library boundary.

The API also provides:

- progress notifications at the CAD read, mesh, PRC, and PDF stage boundaries;
- informational and error log callbacks;
- cooperative cancellation from a progress callback or another thread.

### 6.2 C/C++ example

```cpp
#include <3DPDFConverter.h>
#include <stdio.h>

int main(void)
{
    ConverterOptions options = {0};
    Converter_GetDefaultOptions(&options);
    options.meshDeflection = 0.02;
    options.projectionMode = CONVERTER_PROJECTION_ORTHOGRAPHIC;

    ConverterHandle converter = Converter_Create();
    if (converter == NULL)
        return 99;

    ConverterResult result = {0};
    int32_t code = Converter_ConvertUtf8(
        converter,
        "input.step",
        "output.pdf",
        &options,
        &result);

    Converter_Destroy(converter);

    if (code != CONVERTER_RESULT_SUCCESS)
        fprintf(stderr, "Conversion failed: %s\n", result.messageUtf8);

    return code == CONVERTER_RESULT_SUCCESS ? 0 : 1;
}
```

On Windows, use `Converter_ConvertWide` for paths that may contain characters outside the active system code page:

```cpp
int32_t code = Converter_ConvertWide(
    converter,
    L"☑あう１①②③えイ.step",
    L"☑あう１①②③えイ.pdf",
    &options,
    &result);
```

Each converter handle accepts one active request at a time. Separate handles may be used for independent concurrent requests.

### 6.3 Progress, logging, and cancellation

Register callbacks before starting a conversion:

```cpp
static int32_t onProgress(
    void* userData,
    int32_t stage,
    double progress,
    const char* messageUtf8)
{
    printf("%3.0f%% %s\n", progress * 100.0, messageUtf8);

    const int* shouldCancel = (const int*)userData;
    return *shouldCancel != 0 ? 1 : 0;
}

static void onLog(
    void* userData,
    int32_t level,
    const char* messageUtf8)
{
    (void)userData;
    fprintf(level == CONVERTER_LOG_ERROR ? stderr : stdout, "%s\n", messageUtf8);
}

int shouldCancel = 0;
Converter_SetCallbacks(converter, onProgress, onLog, &shouldCancel);
```

A nonzero progress-callback return value requests cancellation. Another thread may call:

```cpp
Converter_RequestCancel(converter);
```

Cancellation is cooperative and is checked between major conversion stages. A third-party CAD, meshing, PRC, or PDF operation that is already running must return before cancellation can finish. A cancelled request returns `CONVERTER_RESULT_CANCELLED`, and the converter removes the requested output PDF.

Callbacks execute synchronously on the conversion thread. The application must keep the callback functions and `userData` alive until `Converter_ConvertUtf8` or `Converter_ConvertWide` returns. Do not destroy a handle while a conversion is active. Callback exceptions are caught by the C++ implementation and converted into a cancellation request, but callbacks should not throw across the C ABI.

## 7. Validation

Enable and run the smoke test:

```powershell
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

The automated test verifies API versioning, default options, handle lifecycle, progress and log delivery, callback cancellation, a real STEP conversion, and creation of a non-empty PDF.

The PDF must also be checked in a reader that supports PRC 3D annotations. Confirm that:

- the 3D annotation activates;
- the model is visible and centered;
- face orientation is correct;
- projection and background settings are applied;
- rotation, zoom, and model interaction work.

Static rendering tools do not activate 3D annotations and may render the page as blank.

## 8. Cross-platform design

The intended outputs are:

| Platform | Library | CLI |
|---|---|---|
| Windows | `3DPDFConverter.dll` | `3DPDFConverterCLI.exe` |
| Linux | `lib3DPDFConverter.so` | `3DPDFConverterCLI` |
| macOS | `lib3DPDFConverter.dylib` | `3DPDFConverterCLI` |

The UTF-8 C API is the portable interface. The wide-character API is Windows-specific. The current non-Windows PRC and PDF implementation remains disabled until compatible libPRC and libHaru builds are supplied and verified.
