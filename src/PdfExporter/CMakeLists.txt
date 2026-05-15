# PdfExporter - PDF generation with 3D embedding module
add_library(PdfExporter SHARED)

target_sources(PdfExporter PRIVATE
    PdfExporter.h
    PdfExporter_global.h
    PdfExporter.cpp
)

target_include_directories(PdfExporter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/3rdparty/libharu-2.3.0/include
)

target_link_libraries(PdfExporter PUBLIC
    CoreTypes
    Logger
)

# Link libHaru library
if(WIN32)
    target_link_libraries(PdfExporter PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/libharu-2.3.0/lib/libhpdf.lib
    )
else()
    target_link_libraries(PdfExporter PRIVATE
        ${CMAKE_SOURCE_DIR}/3rdparty/libharu-2.3.0/lib/libhpdf.a
    )
endif()

target_compile_definitions(PdfExporter PRIVATE
    PDFEXPORTER_LIBRARY
)

set_target_properties(PdfExporter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
