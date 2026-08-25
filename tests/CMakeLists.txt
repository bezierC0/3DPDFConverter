add_executable(ConverterApiTest
    ConverterApiTest.cpp
)

target_link_libraries(ConverterApiTest PRIVATE
    3DPDFConverter
)

set_target_properties(ConverterApiTest PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

add_test(
    NAME ConverterApiSmoke
    COMMAND ConverterApiTest
        ${CMAKE_CURRENT_SOURCE_DIR}/testdata/001_sheet_bend.step
        ${CMAKE_CURRENT_BINARY_DIR}/converter_api_smoke.pdf
)
