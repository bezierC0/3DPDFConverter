# Logger - Logging system module
add_library(Logger SHARED)

target_sources(Logger PRIVATE
    Logger.cpp
    Logger.h
    Logger_global.h
)

target_include_directories(Logger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(Logger PUBLIC
    CoreTypes
)

target_compile_definitions(Logger PRIVATE
    LOGGER_LIBRARY
)

set_target_properties(Logger PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
