# 3DPDFConverter - unified headless conversion library
add_library(3DPDFConverter SHARED)

target_sources(3DPDFConverter PRIVATE
    ConversionData.h
    ConversionCallbacks.h
    3DPDFConverter.cpp
    3DPDFConverter.h
    ${CMAKE_SOURCE_DIR}/src/StepReader/StepReader.cpp
    ${CMAKE_SOURCE_DIR}/src/MeshGenerator/MeshGenerator.cpp
    ${CMAKE_SOURCE_DIR}/src/PrcExporter/PrcExporter.cpp
    ${CMAKE_SOURCE_DIR}/src/PdfExporter/PdfExporter.cpp
    ${CMAKE_SOURCE_DIR}/src/ExportFacade/ExportFacade.cpp
)

target_include_directories(3DPDFConverter
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/CoreTypes
        ${CMAKE_SOURCE_DIR}/src/ExportFacade
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/StepReader
        ${CMAKE_SOURCE_DIR}/src/MeshGenerator
        ${CMAKE_SOURCE_DIR}/src/PrcExporter
        ${CMAKE_SOURCE_DIR}/src/PdfExporter
        ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/include
        ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/include/asymptote
        ${CMAKE_SOURCE_DIR}/3rdparty/libharu-2.3.0/include
)

target_link_libraries(3DPDFConverter PRIVATE
    CoreTypes
    ${OCCT_LIBRARIES_TO_LINK}
)

if(WIN32)
    target_link_libraries(3DPDFConverter PRIVATE
        optimized ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/libPRC.lib
        debug ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/libPRCd.lib
        optimized ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/asymptote.lib
        debug ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/asymptoted.lib
        optimized ${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlib.lib
        debug ${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlibd.lib
        ${CMAKE_SOURCE_DIR}/3rdparty/libharu-2.3.0/lib/libhpdf.lib
    )
endif()

target_compile_definitions(3DPDFConverter PRIVATE
    THREEDPDFCONVERTER_LIBRARY
    STEPREADER_LIBRARY
    MESHGENERATOR_LIBRARY
    PRCEXPORTER_LIBRARY
    PDFEXPORTER_LIBRARY
    EXPORTFACADE_LIBRARY
)

set_target_properties(3DPDFConverter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    OUTPUT_NAME "3DPDFConverter"
)
