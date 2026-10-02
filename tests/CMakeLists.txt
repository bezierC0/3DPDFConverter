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

set(CONVERTER_TEST_STEP "${CMAKE_CURRENT_SOURCE_DIR}/testdata/001_sheet_bend.step")
if(NOT EXISTS "${CONVERTER_TEST_STEP}")
    message(FATAL_ERROR "Converter smoke-test STEP file is missing: ${CONVERTER_TEST_STEP}")
endif()

add_test(
    NAME ConverterApiSmoke
    COMMAND ConverterApiTest
        ${CONVERTER_TEST_STEP}
        ${CMAKE_CURRENT_BINARY_DIR}/converter_api_smoke.pdf
)
