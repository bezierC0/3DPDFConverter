# 3DPDFConverter

Current release: **0.2.0 prerelease**. This version adds Linux x64 conversion and packaging with an end-to-end CI test.

3DPDFConverter converts STEP files into PRC-based 3D PDF documents. The conversion engine is delivered as a shared library, while a thin command-line application provides a simple process-based interface. An optional Qt GUI is available only for development and validation.

## Outputs

- `3DPDFConverter.dll`: versioned C API for application integration.
- `3DPDFConverterCLI.exe`: command-line interface that calls the same shared library.
- `3DPDFConverterGUI.exe`: optional Qt validation application.

The converter core does not depend on Qt. OpenCASCADE is used for STEP reading and meshing, libPRC is used to create PRC data, and libHaru embeds the PRC stream into PDF.

The shared-library API supports progress and log callbacks plus cooperative cancellation.

## Default build

```powershell
cmake -S . -B build `
  -DOpenCASCADE_DIR=C:/Libs/OCCT/occt-7.8.0-vc14-64 `
  -DOpenCASCADE_3RDPARTY_DIR=C:/Libs/OCCT/3rdparty-vc14-64 `
  -DBUILD_CONVERTER_SHARED=ON `
  -DBUILD_CONVERTER_CLI=ON `
  -DBUILD_GUI_VALIDATOR=OFF

cmake --build build --config Release
```

Runtime DLL copying follows the deployment method already verified by the Qt_OCC project. Set `IS_COPY_DLLS=ON` to copy the OpenCASCADE and third-party runtime DLLs beside the CLI. Set it to `OFF` when the host application already provides the runtime environment.

## Quick CLI example

```powershell
3DPDFConverterCLI.exe input.step output.pdf `
  --projection orthographic `
  --bg-color DFE8FF `
  --deflection 0.02
```

JSON settings are supported with `--params`:

```powershell
3DPDFConverterCLI.exe input.step output.pdf --params settings.json
```

See [English User Manual](doc/USER_MANUAL.md) for the complete build, CLI, JSON, DLL API, deployment, and validation instructions.

## Current platform status

The public API and core layout are designed for Windows, Linux, and macOS. Windows uses the bundled libPRC and modified libHaru binaries. Linux builds Asymptote PRC 3.15 from a pinned source archive and uses the system libHaru package. macOS still uses the unsupported-platform exporter stubs.

See [Linux Release Guide](doc/LINUX_RELEASE.md) for Linux runtime requirements, package verification, and the manual GitHub Release procedure.
