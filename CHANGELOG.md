# Changelog

All notable changes to 3DPDFConverter are documented in this file.

## [0.1.0] - 2026-10-02

Initial Windows x64 prerelease.

### Added

- Versioned C API with UTF-8 and Windows wide-character paths.
- STEP reading and tessellation through OpenCASCADE.
- PRC generation and embedding in 3D PDF files.
- Command-line options and JSON configuration.
- Progress, logging, cancellation, and busy-state handling.
- Windows end-to-end conversion smoke test.

### Known limitations

- Complete PRC/PDF conversion is currently supported only on Windows x64.
- STEP assembly names, colors, and product structure are not preserved.
- Generated PDFs do not yet contain a static poster image.
- PRC compression and advanced line rendering are not implemented.
