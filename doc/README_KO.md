# 3DPDFConverter

[English](../README.md) | [中文](README_CN.md) | [日本語](README_JP.md) | [한국어](README_KO.md) | [русский](README_RU.md)

이것은 C++, OpenCASCADE 및 libHaru를 기반으로 하는 모듈형 3D PDF 생성 도구 플랫폼입니다.

## 주요 기능
*   **STEP 파싱**: STEP/STP 형식의 파일을 읽습니다.
*   **메시 생성**: OpenCASCADE의 `BRepMesh_IncrementalMesh` 알고리즘을 사용하여 삼각형 메시를 생성합니다.
*   **PRC 변환**: 3D 데이터 교환을 위해 메시 데이터를 PRC 형식으로 변환합니다.
*   **PDF 내보내기**: libHaru를 사용하여 PRC 데이터를 포함하고 표준 3D PDF를 출력합니다.
*   **높은 분리도**: 핵심 기능은 여러 DLL 모듈(StepReader, MeshGenerator, PrcExporter, PdfExporter)로 분리되어 있으며, 다른 언어 및 툴체인과 쉽게 통합할 수 있는 안전한 C 스타일 인터페이스를 제공합니다.

## 모듈 구조
- `CoreTypes`: DLL 간 통신을 위한 순수 C 구조체 및 인터페이스 핸들을 정의합니다.
- `Logger`: 스레드로부터 안전한 로깅 컴포넌트.
- `StepReader`: STEP 모델을 로드하고 파싱합니다.
- `MeshGenerator`: STEP BRep을 메시로 삼각 분할합니다.
- `PrcExporter`: PRC 바이너리 스트림을 생성합니다.
- `PdfExporter`: 최종 PDF 파일을 생성하고 3D 뷰를 포함합니다.
- `ExportFacade`: 원클릭 내보내기를 위한 통합 래퍼 인터페이스를 제공합니다.
- `CLI`: 명령줄 실행 진입점 (`3DPDFConverterCLI.exe`).

## 빌드 지침
이 프로젝트는 의존성 관리를 위해 CMake와 vcpkg를 사용합니다:
```bash
# 프로젝트 생성 (GTest, libharu 등 의존성 포함)
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=[vcpkg 경로]/scripts/buildsystems/vcpkg.cmake

# 빌드
cmake --build build
```