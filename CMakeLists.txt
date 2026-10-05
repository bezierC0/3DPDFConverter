cmake_minimum_required(VERSION 3.24)
project(Step3DPdfConverter VERSION 0.2.0 LANGUAGES CXX)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    # MSVC compiler flags
    add_compile_options(/W4 /WX- /permissive-)
    # Enable multi-processor compilation
    add_compile_options(/MP)
    # Disable specific warnings if needed
    # add_compile_options(/wd4251)  # DLL interface warnings
else()
    # GCC/Clang compiler flags
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Enable position-independent code for shared libraries
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Global output directories to ensure EXEs and DLLs are together
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

####################################
# Qt Configuration (for GUI)
####################################
option(BUILD_GUI_VALIDATOR "Build the optional Qt GUI used to validate generated 3D PDFs" OFF)
option(BUILD_CONVERTER_SHARED "Build the 3D PDF converter shared library" ON)
option(BUILD_CONVERTER_CLI "Build the 3D PDF converter command-line application" ON)
option(IS_COPY_DLLS "Copy OpenCASCADE and third-party runtime DLLs" ON)

if(BUILD_CONVERTER_CLI AND NOT BUILD_CONVERTER_SHARED)
    message(FATAL_ERROR "BUILD_CONVERTER_CLI requires BUILD_CONVERTER_SHARED")
endif()

set(RESOURCE_DIR "${CMAKE_SOURCE_DIR}/resource")

####################################
# Version info
####################################
if(WIN32)
    configure_file(
        ${RESOURCE_DIR}/version.rc.in
        ${CMAKE_BINARY_DIR}/version.rc
        @ONLY
    )
elseif(APPLE)
    # macOS Info.plist configuration
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.company.Step3DPdfConverter")
    set(MACOSX_BUNDLE_BUNDLE_NAME "Step3DPdfConverter")
    set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}")
    set(MACOSX_BUNDLE_ICON_FILE "icon.icns")
    set(MACOSX_BUNDLE_COPYRIGHT "Copyright (C) 2026")
endif()

####################################
# OpenCASCADE Configuration
####################################
set(OpenCASCADE_DIR "" CACHE PATH "OpenCASCADE Options: Path to OpenCASCADE directory")
set(OpenCASCADE_3RDPARTY_DIR "" CACHE PATH "OpenCASCADE Options: Path to OpenCASCADE 3RDPARTY directory")

if(WIN32)
    # Windows: Use manual configuration due to broken OpenCASCADEConfig.cmake
    include(${OpenCASCADE_DIR}/CMake/OpenCASCADEConfig.cmake)
else()
    # Linux/macOS: Use find_package
    find_package(OpenCASCADE REQUIRED)
endif()

set(OpenCASCADE_VERSION ${OpenCASCADE_MAJOR_VERSION}.${OpenCASCADE_MINOR_VERSION}.${OpenCASCADE_MAINTENANCE_VERSION})

message(STATUS "OpenCASCADE version: ${OpenCASCADE_VERSION}")
message(STATUS "OpenCASCADE binary: ${OpenCASCADE_BINARY_DIR}")
message(STATUS "OpenCASCADE library: ${OpenCASCADE_LIBRARY_DIR}")
message(STATUS "OpenCASCADE include: ${OpenCASCADE_INCLUDE_DIR}")
message(STATUS "OpenCASCADE 3rdparty: ${OpenCASCADE_3RDPARTY_DIR}")

# Define minimal OpenCASCADE toolkit set based on actual project usage:
# - STEP/IGES XDE import/export (CadProcessor, StepReader)
# - Meshing/topology checks/fixing (MeshGenerator, CadProcessor)
# - 3D visualization widgets (AIS/V3d/OpenGl)
set(OpenCASCADE_TOOLKITS
    # Foundation
    TKernel TKMath
    # Modeling data + topology/mesh/healing
    TKG2d TKG3d TKGeomBase TKGeomAlgo TKBRep TKTopAlgo TKMesh TKShHealing TKBool TKPrim TKHLR
    # Visualization
    TKService TKV3d TKOpenGl
    # XDE application framework
    TKCDF TKLCAF TKCAF TKVCAF TKXCAF
    # Data exchange base
    TKXSBase
    TKBO
)

# Add version-specific toolkits
if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
    # OCCT 7.8+: unified DE toolkits used by STEP/IGES CAF readers/writers
    list(APPEND OpenCASCADE_TOOLKITS TKDE TKDEIGES TKDESTEP)
else()
    # OCCT < 7.8: legacy STEP/IGES toolkits
    list(APPEND OpenCASCADE_TOOLKITS TKIGES TKXDEIGES)
    list(APPEND OpenCASCADE_TOOLKITS TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXDESTEP)
endif()

# Build library list
set(OCCT_LIBRARIES_TO_LINK "")
if(WIN32)
    foreach(OccTK ${OpenCASCADE_TOOLKITS})
        list(APPEND OCCT_LIBRARIES_TO_LINK "${OccTK}.lib")
    endforeach()
    list(APPEND OCCT_LIBRARIES_TO_LINK "OpenGL32.lib")
else()
    set(OCCT_LIBRARIES_TO_LINK ${OpenCASCADE_TOOLKITS})
    if(APPLE)
        find_package(OpenGL REQUIRED)
        list(APPEND OCCT_LIBRARIES_TO_LINK ${OPENGL_LIBRARIES})
    else()
        list(APPEND OCCT_LIBRARIES_TO_LINK "GL")
    endif()
endif()

# Find OpenCASCADE DLLs on Windows
if(WIN32)
    # Match the deployment strategy verified by the Qt_OCC project.
    file(GLOB OpenCASCADE_TOOLKITS_DLLS "${OpenCASCADE_BINARY_DIR}/*.dll")
    list(LENGTH OpenCASCADE_TOOLKITS_DLLS OCCT_TOOLKIT_DLL_COUNT)
    message(STATUS "Found ${OCCT_TOOLKIT_DLL_COUNT} OpenCASCADE toolkit DLLs to copy")

    # 3RDPARTY DLLs
    file(GLOB FREETYPE_DIRS "${OpenCASCADE_3RDPARTY_DIR}/freetype*")
    file(GLOB TCLTK_DIRS "${OpenCASCADE_3RDPARTY_DIR}/tcltk*")
    file(GLOB FREEIMAGE_DIRS "${OpenCASCADE_3RDPARTY_DIR}/freeimage*")
    file(GLOB TBB_DIRS "${OpenCASCADE_3RDPARTY_DIR}/tbb*")
    file(GLOB JEMALLOC_DIRS "${OpenCASCADE_3RDPARTY_DIR}/jemalloc*")
    file(GLOB FFMPEG_DIRS "${OpenCASCADE_3RDPARTY_DIR}/ffmpeg*")

    set(3RDPARTY_MODULE_DIRS
        ${FREETYPE_DIRS}
        ${TCLTK_DIRS}
        ${FREEIMAGE_DIRS}
        ${TBB_DIRS}
        ${JEMALLOC_DIRS}
        ${FFMPEG_DIRS}
    )

    set(OpenCASCADE_3RDPARTY_DLLS "")
    foreach(3RDPARTY_MODULE_DIR ${3RDPARTY_MODULE_DIRS})
        set(CURRENT_3RDPARTY_MODULE_BIN_DIR "${3RDPARTY_MODULE_DIR}/bin")
        if(EXISTS "${CURRENT_3RDPARTY_MODULE_BIN_DIR}")
            file(GLOB DLLS_IN_BIN "${CURRENT_3RDPARTY_MODULE_BIN_DIR}/*.dll")
            list(APPEND OpenCASCADE_3RDPARTY_DLLS ${DLLS_IN_BIN})
        endif()
    endforeach()

    file(GLOB 3RDPARTY_MODULE_OPENVR_DIR "${OpenCASCADE_3RDPARTY_DIR}/openvr*")
    file(GLOB OPENVR_DLLS_IN_BIN "${3RDPARTY_MODULE_OPENVR_DIR}/bin/win64/*.dll")
    list(APPEND OpenCASCADE_3RDPARTY_DLLS ${OPENVR_DLLS_IN_BIN})
    list(REMOVE_DUPLICATES OpenCASCADE_3RDPARTY_DLLS)

    list(LENGTH OpenCASCADE_3RDPARTY_DLLS OCCT_3RDPARTY_DLL_COUNT)
    message(STATUS "Found ${OCCT_3RDPARTY_DLL_COUNT} 3rdparty DLLs")
endif()

# Include and link directories
if(OpenCASCADE_INCLUDE_DIR)
    include_directories(${OpenCASCADE_INCLUDE_DIR})
else()
    message(WARNING "OpenCASCADE_INCLUDE_DIR is not set")
endif()

if(OpenCASCADE_LIBRARY_DIR)
    link_directories(${OpenCASCADE_LIBRARY_DIR})
else()
    message(WARNING "OpenCASCADE_LIBRARY_DIR is not set")
endif()

if(NOT OCCT_LIBRARIES_TO_LINK)
    message(FATAL_ERROR "Failed to build OpenCASCADE library list")
endif()

####################################
# Qt Configuration
####################################
if(BUILD_GUI_VALIDATOR)
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools)
    message(STATUS "Qt5 version: ${Qt5_VERSION}")
    message(STATUS "Qt5 directory: ${Qt5_DIR}")
endif()

####################################
# Module Subdirectories
####################################
message(STATUS "Configuring 3D PDF converter targets...")

# Converter library
add_subdirectory(src/CoreTypes)
if(BUILD_CONVERTER_SHARED)
    add_subdirectory(src/Converter)
endif()

# Client applications
if(BUILD_CONVERTER_CLI)
    add_subdirectory(src/CLI)
endif()
if(BUILD_GUI_VALIDATOR)
    add_subdirectory(src/GUI)
endif()

# Tests
option(BUILD_TESTS "Build unit and integration tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()



####################################
# Installation
####################################
include(GNUInstallDirs)

# Install converter library
if(BUILD_CONVERTER_SHARED)
    install(TARGETS 3DPDFConverter
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

if(UNIX AND NOT APPLE)
    install(TARGETS PRC
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# Install CLI executable
if(BUILD_CONVERTER_CLI)
    install(TARGETS 3DPDFConverterCLI
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

# Install headers
install(DIRECTORY src/CoreTypes/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Step3DPdfConverter
    FILES_MATCHING PATTERN "*.h"
)

install(FILES src/Converter/3DPDFConverter.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Step3DPdfConverter
)

install(FILES
    README.md
    CHANGELOG.md
    LICENSE
    THIRD_PARTY_NOTICES.md
    DESTINATION .
)

install(DIRECTORY licenses/
    DESTINATION licenses
)

if(WIN32)
    if(EXISTS "${OpenCASCADE_DIR}/LICENSE_LGPL_21.txt")
        install(FILES "${OpenCASCADE_DIR}/LICENSE_LGPL_21.txt"
            DESTINATION licenses
            RENAME OpenCASCADE-LGPL-2.1.txt
        )
    endif()
    if(EXISTS "${OpenCASCADE_DIR}/OCCT_LGPL_EXCEPTION.txt")
        install(FILES "${OpenCASCADE_DIR}/OCCT_LGPL_EXCEPTION.txt"
            DESTINATION licenses
            RENAME OpenCASCADE-LGPL-exception.txt
        )
    endif()

    set(CPACK_GENERATOR "ZIP")
    set(CPACK_PACKAGE_FILE_NAME "3DPDFConverter-${PROJECT_VERSION}-windows-x64")
elseif(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_PACKAGE_FILE_NAME "3DPDFConverter-${PROJECT_VERSION}-linux-x64")
endif()

if(WIN32 OR (UNIX AND NOT APPLE))
    set(CPACK_PACKAGE_NAME "3DPDFConverter")
    set(CPACK_PACKAGE_VENDOR "littleCatBaby")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "STEP to PRC-based 3D PDF converter")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    include(CPack)
endif()



####################################
# Summary
####################################
message(STATUS "")
message(STATUS "========================================")
message(STATUS "Configuration Summary")
message(STATUS "========================================")
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenCASCADE: ${OpenCASCADE_VERSION}")
message(STATUS "Converter shared library: ${BUILD_CONVERTER_SHARED}")
message(STATUS "Converter CLI: ${BUILD_CONVERTER_CLI}")
message(STATUS "Copy runtime DLLs: ${IS_COPY_DLLS}")
message(STATUS "GUI validator: ${BUILD_GUI_VALIDATOR}")
if(BUILD_GUI_VALIDATOR)
    message(STATUS "Qt5: ${Qt5_VERSION}")
endif()
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "========================================")
message(STATUS "")
