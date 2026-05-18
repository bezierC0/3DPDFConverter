# 3DPDFConverter

[English](README.md) | [中文](./doc/README_CN.md) | [日本語](./doc/README_JP.md) | [한국어](./doc/README_KO.md) | [русский](./doc/README_RU.md)

This is a modular 3D PDF generation tool platform based on C++, OpenCASCADE, and libHaru.

## Features
*   **STEP Parsing**: Read STEP/STP format files.
*   **Meshing**: Generate triangular meshes using OpenCASCADE's `BRepMesh_IncrementalMesh` algorithm.
*   **PRC Conversion**: Convert mesh data into PRC format for 3D data exchange.
*   **PDF Export**: Embed PRC data using libHaru and output standard 3D PDFs.
*   **Highly Decoupled**: Core functionalities are separated into multiple DLL modules (StepReader, MeshGenerator, PrcExporter, PdfExporter), providing safe C-style interfaces for easy integration across different languages and toolchains.

## Module Structure
- `CoreTypes`: Defines pure C structures and interface handles for cross-DLL communication.
- `Logger`: Thread-safe logging component.
- `StepReader`: Loads and parses STEP models.
- `MeshGenerator`: Triangulates STEP BRep into meshes.
- `PrcExporter`: Generates the PRC binary stream.
- `PdfExporter`: Generates the final PDF file and embeds the 3D view.
- `ExportFacade`: Provides a unified wrapper interface for one-click export.
- `CLI`: Command-line execution entry point (`3DPDFConverterCLI.exe`).

## Build Instructions
This project uses CMake and vcpkg for dependency management:
```bash
# Generate project (including dependencies like GTest, libharu)
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=[Your vcpkg path]/scripts/buildsystems/vcpkg.cmake

# Build
cmake --build build
```

## CLI Usage
`3DPDFConverterCLI` supports direct STEP to 3D PDF export with basic and advanced options.

```bash
3DPDFConverterCLI <input.step> <output.pdf> [options]
```

Options:
- `--help`, `-h`: Show help and exit
- `--page-size <A4|A3|Letter|Legal>`
- `--bg-color <RRGGBB>`
- `--projection <perspective|orthographic>`
- `--fov <deg>`
- `--roll <deg>`
- `--radius <value>`
- `--deflection <value>`
- `--compression <0..1>`
- `--granularity <0..1>`
- `--keep-temp-prc`

Example:

```bash
3DPDFConverterCLI part.step part.pdf --page-size A3 --projection orthographic --bg-color DFE8FF --deflection 0.005
```
