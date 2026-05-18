# PrcExporter - PRC format export module
add_library(PrcExporter SHARED)

target_sources(PrcExporter PRIVATE
    PrcExporter.h
    PrcExporter_global.h
    PrcExporter.cpp
)

target_include_directories(PrcExporter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/include
    ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/include/asymptote
)

target_link_libraries(PrcExporter PUBLIC
    CoreTypes
    Logger
)

# Link libPRC and Asymptote library
if(WIN32)
    target_link_libraries(PrcExporter PRIVATE
        optimized ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/libPRC.lib
        debug ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/libPRCd.lib
        optimized ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/asymptote.lib
        debug ${CMAKE_SOURCE_DIR}/3rdparty/libPRC/lib/asymptoted.lib
        optimized ${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlib.lib
        debug ${CMAKE_SOURCE_DIR}/3rdparty/zlib/lib/zlibd.lib
    )
else()
    # libPRC is stubbed on non-Windows platforms, no precompiled libs are required to link
endif()

target_compile_definitions(PrcExporter PRIVATE
    PRCEXPORTER_LIBRARY
)

set_target_properties(PrcExporter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
