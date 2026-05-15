# Tests - Unit and integration tests
enable_testing()

# Find Google Test
find_package(GTest REQUIRED)

# Test executable
add_executable(AllTests)

target_sources(AllTests PRIVATE
    # Test files will be added in subsequent tasks
    # test_CoreTypes.cpp (task 2.2)
    # test_Logger.cpp (task 3.2)
    # test_StepReader.cpp (task 4.2)
    # test_MeshGenerator.cpp (task 5.2)
    # test_PrcExporter.cpp (task 6.2)
    # test_PdfExporter.cpp (task 7.2)
    # test_ExportFacade.cpp (task 9.2)
)

target_include_directories(AllTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(AllTests PRIVATE
    GTest::gtest
    GTest::gtest_main
    CoreTypes
    Logger
    StepReader
    MeshGenerator
    PrcExporter
    PdfExporter
    ExportFacade
)

set_target_properties(AllTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Register tests with CTest
include(GoogleTest)
gtest_discover_tests(AllTests)
