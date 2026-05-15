# ExportFacade - Unified export interface module
add_library(ExportFacade SHARED)

target_sources(ExportFacade PRIVATE
    ExportFacade.h
    ExportFacade_global.h
    ExportFacade.cpp
)

target_include_directories(ExportFacade PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ExportFacade PUBLIC
    CoreTypes
    Logger
    StepReader
    MeshGenerator
    PrcExporter
    PdfExporter
)

target_compile_definitions(ExportFacade PRIVATE
    EXPORTFACADE_LIBRARY
)

set_target_properties(ExportFacade PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
