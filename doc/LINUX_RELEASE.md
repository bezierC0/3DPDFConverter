# Linux x64 Release Guide

This guide covers the Linux x64 package produced by GitHub Actions and the manual GitHub Release procedure.

## Package contents

The Linux artifact is named:

```text
3DPDFConverter-0.2.0-linux-x64.tar.gz
```

It contains the converter shared library, replaceable Asymptote PRC shared library, command-line application, public headers, changelog, and license notices. OpenCASCADE, libHaru, and their runtime dependencies are provided by the Linux distribution and are not bundled.

## Runtime requirements

On Ubuntu, install the runtime packages that provide OpenCASCADE, libHaru, OpenGL, TBB, and zlib. Package names can vary between Ubuntu releases, so use the packages matching the CI dependency list in `.github/workflows/build.yml`.

## Verify the CI artifact

1. Open the successful workflow run for the release commit.
2. Confirm that `build-linux`, `build-windows`, and `build-macos` passed.
3. Download the `3DPDFConverter-0.2.0-linux-x64` artifact.
4. Extract the outer GitHub artifact ZIP.
5. Copy `3DPDFConverter-0.2.0-linux-x64.tar.gz` to a Linux x64 system.
6. Extract the package into a temporary directory.
7. Run the following commands from that directory:

```bash
./bin/3DPDFConverterCLI --version
ldd ./bin/3DPDFConverterCLI
./bin/3DPDFConverterCLI sample.step output.pdf
head -c 5 output.pdf
```

The version command must report `0.2.0`, `ldd` must not report missing libraries, the conversion command must succeed, and the final command must print `%PDF-`.

## Manual GitHub Release

Create the release only after the tag workflow and the manual package verification pass.

1. Merge the Linux release changes into `main`.
2. Create and push the `v0.2.0` tag from the verified commit.
3. Wait for the tag workflow to pass.
4. Open the repository Releases page and select **Draft a new release**.
5. Choose the existing `v0.2.0` tag. Do not create another tag.
6. Set the title to `3DPDFConverter v0.2.0`.
7. Copy the `0.2.0` section from `CHANGELOG.md` into the release description.
8. Upload both release packages:
   - `3DPDFConverter-0.2.0-windows-x64.zip`
   - `3DPDFConverter-0.2.0-linux-x64.tar.gz`
9. Select **Set as a pre-release**.
10. Do not select **Set as the latest release**.
11. Publish the release.

If code or packaging changes are required after the tag is created, do not move or replace the tag. Make the fix and publish a new version instead.
