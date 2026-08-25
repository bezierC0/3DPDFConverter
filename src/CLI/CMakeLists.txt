# CLI - Command-line interface application
add_executable(3DPDFConverterCLI)

target_sources(3DPDFConverterCLI PRIVATE
    main.cpp
)

target_include_directories(3DPDFConverterCLI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

find_path(RAPIDJSON_INCLUDE_DIR
    NAMES rapidjson/document.h
    HINTS ${OpenCASCADE_3RDPARTY_DIR}
    PATH_SUFFIXES include rapidjson-1.1.0/include
    REQUIRED
)
target_include_directories(3DPDFConverterCLI PRIVATE ${RAPIDJSON_INCLUDE_DIR})

if(MSVC)
    target_compile_definitions(3DPDFConverterCLI PRIVATE
        _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
    )
endif()

target_link_libraries(3DPDFConverterCLI PRIVATE
    3DPDFConverter
    CoreTypes
)

if(WIN32)
    target_link_libraries(3DPDFConverterCLI PRIVATE Shell32)
endif()

set_target_properties(3DPDFConverterCLI PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Use the same post-build deployment approach as the verified Qt_OCC project.
if(WIN32 AND IS_COPY_DLLS)
    file(GLOB CONVERTER_LIBPRC_DLLS "${CMAKE_SOURCE_DIR}/3rdparty/libPRC/bin/*.dll")
    file(GLOB CONVERTER_LIBHARU_DLLS "${CMAKE_SOURCE_DIR}/3rdparty/libharu-2.3.0/bin/*.dll")
    file(GLOB CONVERTER_ZLIB_DLLS "${CMAKE_SOURCE_DIR}/3rdparty/zlib/bin/*.dll")
    set(ALL_CONVERTER_DLLS_TO_COPY
        ${OpenCASCADE_TOOLKITS_DLLS}
        ${OpenCASCADE_3RDPARTY_DLLS}
        ${CONVERTER_LIBPRC_DLLS}
        ${CONVERTER_LIBHARU_DLLS}
        ${CONVERTER_ZLIB_DLLS}
    )
    list(REMOVE_DUPLICATES ALL_CONVERTER_DLLS_TO_COPY)

    if(ALL_CONVERTER_DLLS_TO_COPY)
        add_custom_command(
            TARGET 3DPDFConverterCLI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${ALL_CONVERTER_DLLS_TO_COPY}
                    $<TARGET_FILE_DIR:3DPDFConverterCLI>
            COMMAND_EXPAND_LISTS
            COMMENT "Copying OpenCASCADE and third-party runtime DLLs"
        )
    endif()
endif()
