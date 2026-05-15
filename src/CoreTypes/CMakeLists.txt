# CoreTypes - Header-only library for shared data structures
# Provides POD structures, enumerations, and result codes for inter-module communication

cmake_minimum_required(VERSION 3.19)

# Create INTERFACE library (header-only)
add_library(CoreTypes INTERFACE)

# Specify include directory for consumers of this library
target_include_directories(CoreTypes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Require C++17 standard
target_compile_features(CoreTypes INTERFACE cxx_std_17)

# Install headers (optional, for package distribution)
install(FILES CoreTypes.h ExportTypes.h
    DESTINATION include/CoreTypes
)

# Optional: Build test executable to verify CoreTypes
option(BUILD_CORETYPES_TEST "Build CoreTypes verification test" OFF)
if(BUILD_CORETYPES_TEST)
    add_executable(test_coretypes test_coretypes.cpp)
    target_link_libraries(test_coretypes PRIVATE CoreTypes)
endif()
