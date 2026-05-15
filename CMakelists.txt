cmake_minimum_required(VERSION 3.24)
project(Step3DPdfConverter VERSION 0.1.0 LANGUAGES CXX)

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
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

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

# Define OpenCASCADE toolkits
set(
    OpenCASCADE_TOOLKITS
    # FoundationClasses
    TKernel TKMath
    # ModelingData
    TKG2d TKG3d TKGeomBase TKBRep
    # ModelingAlgorithms
    TKMesh TKShHealing TKTopAlgo
    # Visualization
    TKOpenGl TKService TKV3d
    # ApplicationFramework
    TKCAF TKCDF TKLCAF TKVCAF
    # DataExchange
    TKXCAF TKXSBase
)

# Add version-specific toolkits
if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.8.0)
    list(APPEND OpenCASCADE_TOOLKITS TKDE TKDEIGES TKDESTEP)
else()
    list(APPEND OpenCASCADE_TOOLKITS TKIGES TKXDEIGES)
    list(APPEND OpenCASCADE_TOOLKITS TKSTEP TKSTEP209 TKSTEPAttr TKSTEPBase TKXDESTEP)
    if(OpenCASCADE_VERSION VERSION_GREATER_EQUAL 7.7.0)
        list(APPEND OpenCASCADE_TOOLKITS TKXDE)
    endif()
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
    file(GLOB OpenCASCADE_TOOLKITS_DLLS "${OpenCASCADE_BINARY_DIR}/*.dll")

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
find_package(Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools)

if(Qt5_FOUND)
    message(STATUS "Qt5 version: ${Qt5_VERSION}")
    message(STATUS "Qt5 directory: ${Qt5_DIR}")
else()
    message(FATAL_ERROR "Qt5 not found")
endif()

####################################
# Module Subdirectories
####################################
message(STATUS "Configuring modular DLL architecture...")

# Core modules (order matters for dependencies)
add_subdirectory(src/CoreTypes)
add_subdirectory(src/Logger)
add_subdirectory(src/StepReader)
add_subdirectory(src/MeshGenerator)
add_subdirectory(src/PrcExporter)
add_subdirectory(src/PdfExporter)
add_subdirectory(src/ExportFacade)

# Client applications
add_subdirectory(src/CLI)
add_subdirectory(src/GUI)

# Tests
option(BUILD_TESTS "Build unit and integration tests" OFF)
if(BUILD_TESTS)
#    add_subdirectory(tests)
endif()



####################################
# Installation
####################################
include(GNUInstallDirs)

# Install DLL modules
install(TARGETS Logger StepReader MeshGenerator PrcExporter PdfExporter ExportFacade
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install CLI executable
install(TARGETS 3DPDFConverterCLI
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY src/CoreTypes/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Step3DPdfConverter
    FILES_MATCHING PATTERN "*.h"
)



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
message(STATUS "Qt5: ${Qt5_VERSION}")
message(STATUS "Build tests: ${BUILD_TESTS}")
message(STATUS "========================================")
message(STATUS "")
