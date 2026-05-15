# MeshGenerator - Mesh triangulation module
add_library(MeshGenerator SHARED)

target_sources(MeshGenerator PRIVATE
    MeshGenerator.h
    MeshGenerator_global.h
    MeshGenerator.cpp
)

target_include_directories(MeshGenerator PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MeshGenerator PUBLIC
    CoreTypes
    Logger
    ${OCCT_LIBRARIES_TO_LINK}
)

target_compile_definitions(MeshGenerator PRIVATE
    MESHGENERATOR_LIBRARY
)

set_target_properties(MeshGenerator PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
