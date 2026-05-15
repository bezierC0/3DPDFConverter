# 上下文
文件名：3d_pdf_task.md
创建于：2026-05-15T07:40:35+09:00
创建者：AI
关联协议：RIPER-5 + Multidimensional + Agent Protocol 

# 任务描述
开发一个基于 C++ 的桌面工具：
* 输入 STEP / STP 3D CAD 文件
* 使用 OpenCASCADE 读取与三角化
* 使用 PRC 格式嵌入 PDF
* 使用 libHaru 输出 3D PDF
* 提供简单 GUI 配置输出参数
* 核心功能完全 DLL 化
* UI 与核心逻辑完全解耦

# 项目概述
3DPDFConverter: 这是一个基于 OpenCASCADE 和 libHaru 开发的 3D PDF 生成工具，核心要求是高度解耦和模块化，形成 6 个主要的模块 DLL (CoreTypes, StepReader, MeshGenerator, PrcExporter, PdfExporter, ExportFacade) 及一个 Qt GUI。

---
*以下部分由 AI 在协议执行过程中维护*
---

# 分析 (由 RESEARCH 模式填充)
这是一个大型 C++ 架构设计与实现任务。工作区目录已经存在基础代码结构 (src/ 目录包含了部分空目录，如 CLI, CoreTypes, ExportFacade, Logger 等)，并有前置模板 (Occt_Qt_Sample)。
必须严格按照架构先行（Phase 1）要求来操作。
目前的工作是将各个独立模块的架构、接口设计以及数据流规划清晰，并确保遵守无暴露 OCC 类型、纯 C 接口 (`extern "C"`)、无 STL ABI 依赖等规范。

架构设计已完成（详情见后续步骤）。
依赖图：UI -> ExportFacade -> (StepReader, MeshGenerator, PrcExporter, PdfExporter)
各底层模块仅依赖 CoreTypes。所有模块不对外暴露 OCC 及其它第三方库符号。

# 提议的解决方案 (由 INNOVATE 模式填充)
1. **ABI 安全的接口设计**：采用 C 风格句柄（Opaque Handles）与展平数据（Flattened Data）结构，替代 C++ 纯虚接口。在 CoreTypes 定义如 `typedef void* HStepModel;`，并使用纯 POD 结构传递参数，确保极佳的跨语言绑定兼容性和 STL ABI 安全。
2. **RAII 封装层**：在 ExportFacade 中为各子 DLL 的纯 C 接口实现 C++ RAII 包装，隐藏动态库（LoadLibrary 或隐式链接）的生命周期管理，以简化前端 GUI 的调用逻辑。
3. **CMake 隔离**：在 CMakeLists 级别实现严格的依赖隔离，如 `target_link_libraries(StepReader PRIVATE TKSTEP)`，确保上层应用不会意外引入底层依赖的宏或符号。
4. **统一异常捕获边界**：所有 DLL 导出函数内部均具有 `try-catch` 屏障，防止 C++ 异常跨越 DLL 边界。所有错误通过统一的 `ResultCode` 返回。

# 实施计划 (由 PLAN 模式生成)
实施检查清单：
1. 更新项目根目录的 CMakeLists.txt，添加子模块支持和 vcpkg 挂载逻辑。
2. 创建或更新 src/CoreTypes/ExportTypes.h，定义 ExportRequest, ResultCode 等核心类型。
3. 创建或更新 src/CoreTypes/CMakeLists.txt，生成静态/接口库或纯头文件库。
4. 创建 src/StepReader/StepReader.h 和 cpp，实现 C 风格的加载接口。
5. 创建 src/StepReader/CMakeLists.txt，链接 OpenCASCADE 并编译为 DLL。
6. 向用户征求反馈。

# 当前执行步骤 (由 EXECUTE 模式在开始执行某步骤时更新)
> 正在执行: "步骤 6"

# 任务进度 (由 EXECUTE 模式在每步完成后追加)
*   [2026-05-15T07:59:47+09:00]
    *   步骤：1-5. 创建基础项目配置、CoreTypes 的 C 接口封装以及 StepReader 的 DLL 结构
    *   修改：`CMakeLists.txt` (已存在合适配置), `vcpkg.json` (添加了 libharu), `src/CoreTypes/ExportTypes.h`, `src/CoreTypes/CMakeLists.txt`, `src/StepReader/StepReader.h/.cpp`, `src/StepReader/CMakeLists.txt`
    *   更改摘要：完成了 Phase 2 基础类型与第一个底层引擎的骨架搭建。
    *   原因：执行实施计划。
    *   阻碍：无
*   [2026-05-15T08:30:41+09:00]
    *   步骤：补充剩余底层模块的 C 接口与 CMakeLists 配置
    *   修改：`MeshGenerator`, `PrcExporter`, `PdfExporter`, `ExportFacade` 目录下的 `.h` / `.cpp` 和 `CMakeLists.txt`
    *   更改摘要：完成了所有被 CMake 依赖的空子模块的骨架构建，使其能够被 MSVC 成功编译和链接为对应的 DLL，解决了 `F7` 全局构建时的链接错误。
    *   原因：应对用户在 `F7` 编译时的链接错误反馈，并继续 Phase 2 计划。
    *   阻碍：无
*   [2026-05-15T08:38:46+09:00]
    *   步骤：更新清理附带的文档
    *   修改：`README.md`, `BUILD_STRUCTURE.md`, `doc/README_CN.md`
*   [2026-05-15T09:00:59+09:00]
    *   步骤：恢复原版大中型 3D 视图窗口，集成 3D PDF 导出按钮
    *   修改：根目录 `CMakeLists.txt`, `src/MainWindow.h`, `src/MainWindow.cpp`, `.vscode/launch.json`
    *   更改摘要：根据用户选择的“选项 B”，恢复了原 `Occt_Qt_Sample` 的所有视图框架。为其添加了链接到 `ExportFacade` DLL 的依赖，并在 UI 工具栏上添加了“Export 3D PDF”按钮。
    *   原因：用户需要在真实的装配体浏览环境中测试并选择模型导出，而不是用临时创建的空壳界面。
    *   阻碍：无
    *   用户确认状态：待确认

# 最终审查 (由 REVIEW 模式填充)
[待填充]
