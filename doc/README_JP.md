# 3DPDFConverter

[English](../README.md) | [中文](README_CN.md) | [日本語](README_JP.md) | [한국어](README_KO.md) | [русский](README_RU.md)

これは、C++、OpenCASCADE、および libHaru に基づくモジュール化された 3D PDF 生成ツールプラットフォームです。

## 機能
*   **STEP 解析**: STEP/STP 形式のファイルを読み込みます。
*   **メッシュ化**: OpenCASCADE の `BRepMesh_IncrementalMesh` アルゴリズムを使用して三角形メッシュを生成します。
*   **PRC 変換**: 3D データ交換用にメッシュデータを PRC 形式に変換します。
*   **PDF エクスポート**: libHaru を使用して PRC データを埋め込み、標準の 3D PDF を出力します。
*   **高い分離性**: コア機能は複数の DLL モジュール (StepReader, MeshGenerator, PrcExporter, PdfExporter) に分離されており、異なる言語やツールチェーンに簡単に統合できる安全な C 言語インターフェースを提供します。

## モジュール構造
- `CoreTypes`: DLL 間通信用の純粋な C 構造体とインターフェースハンドルを定義します。
- `Logger`: スレッドセーフなロギングコンポーネント。
- `StepReader`: STEP モデルを読み込み、解析します。
- `MeshGenerator`: STEP BRep をメッシュに三角形化します。
- `PrcExporter`: PRC バイナリストリームを生成します。
- `PdfExporter`: 最終的な PDF ファイルを生成し、3D ビューを埋め込みます。
- `ExportFacade`: ワンクリックエクスポートのための統一されたラッパーインターフェースを提供します。
- `CLI`: コマンドライン実行エントリポイント (`3DPDFConverterCLI.exe`)。

## ビルド手順
このプロジェクトでは、依存関係の管理に CMake と vcpkg を使用しています。
```bash
# プロジェクトの生成 (GTest, libharu などの依存関係を含む)
cmake -S . -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=[vcpkgのパス]/scripts/buildsystems/vcpkg.cmake

# ビルド
cmake --build build
```