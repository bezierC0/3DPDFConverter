# 3DPDFConverter

[English](../README.md) | [中文](README_CN.md) | [日本語](README_JP.md) | [한국어](README_KO.md) | [русский](README_RU.md)

这是一个基于 C++、OpenCASCADE 和 libHaru 的模块化 3D PDF 生成工具平台。

## 功能特性
*   **STEP 解析**：读取 STEP/STP 格式文件。
*   **网格化**：使用 OpenCASCADE 的 `BRepMesh_IncrementalMesh` 算法生成三角网格。
*   **PRC 转换**：将网格转化为 PRC 格式用于 3D 数据交换。
*   **PDF 导出**：使用 libHaru 将 PRC 数据嵌入并输出为标准的 3D PDF。
*   **高度解耦**：核心功能分离为多个 DLL 模块（StepReader, MeshGenerator, PrcExporter, PdfExporter），提供纯 C 风格的安全接口，易于集成到不同语言和工具链。

## 模块结构
- `CoreTypes`: 定义了跨 DLL 通信的纯 C 结构体与接口句柄。
- `Logger`: 线程安全的日志组件。
- `StepReader`: 加载并解析 STEP 模型。
- `MeshGenerator`: 将 STEP BRep 网格化为三角面片。
- `PrcExporter`: 生成 PRC 二进制流。
- `PdfExporter`: 生成最终的 PDF 文件并嵌入 3D 视图。
- `ExportFacade`: 对外提供一键式导出的统一包装接口。
- `CLI`: 命令行运行入口 (`3DPDFConverterCLI.exe`)。

## 编译方法
本项目采用 CMake 和 vcpkg 管理依赖：
```bash
# 生成项目（包含 GTest, libharu 等依赖）
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=[你的 vcpkg 路径]/scripts/buildsystems/vcpkg.cmake

# 编译
cmake --build build
```