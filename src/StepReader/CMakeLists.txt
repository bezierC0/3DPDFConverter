# StepReader - STEP file reading module
add_library(StepReader SHARED)

target_sources(StepReader PRIVATE
    StepReader.h
    StepReader_global.h
    StepReader.cpp
)

target_include_directories(StepReader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(StepReader PUBLIC
    CoreTypes
    Logger
    ${OCCT_LIBRARIES_TO_LINK}
)

target_compile_definitions(StepReader PRIVATE
    STEPREADER_LIBRARY
)

set_target_properties(StepReader PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
