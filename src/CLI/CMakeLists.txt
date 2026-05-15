# CLI - Command-line interface application
add_executable(3DPDFConverterCLI)

target_sources(3DPDFConverterCLI PRIVATE
    main.cpp
)

target_include_directories(3DPDFConverterCLI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(3DPDFConverterCLI PRIVATE
    ExportFacade
    CoreTypes
    Logger
)

set_target_properties(3DPDFConverterCLI PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Copy OpenCASCADE and 3rd-party DLLs to the bin folder on Windows
if(WIN32)
    set(ALL_DLLS_TO_COPY ${OpenCASCADE_TOOLKITS_DLLS} ${OpenCASCADE_3RDPARTY_DLLS})
    if(ALL_DLLS_TO_COPY)
        add_custom_command(
            TARGET 3DPDFConverterCLI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${ALL_DLLS_TO_COPY}
                    $<TARGET_FILE_DIR:3DPDFConverterCLI>
            COMMAND_EXPAND_LISTS
            COMMENT "Copying OpenCASCADE and 3rd-party runtime DLLs to output directory"
        )
    endif()
endif()
