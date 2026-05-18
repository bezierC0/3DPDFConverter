# Main GUI Application (CoreApi + Qt GUI)
message(STATUS "Configuring GUI application...")

# CoreApi library (legacy)
set(COREAPI_CPP_SOURCES)
file(GLOB COREAPI_CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core_api/*.cpp")
set(COREAPI_H_HEADERS)
file(GLOB COREAPI_H_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/core_api/*.h")

if(COREAPI_CPP_SOURCES)
    add_library(CoreApi SHARED
        ${COREAPI_CPP_SOURCES}
        ${COREAPI_H_HEADERS}
    )

    target_include_directories(CoreApi PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/core_api
    )

    target_link_libraries(CoreApi PUBLIC
        ${OCCT_LIBRARIES_TO_LINK}
    )

    target_compile_definitions(CoreApi PRIVATE
        CADCORE_LIBRARY
    )

    set_target_properties(CoreApi PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endif()

# GUI Application
set(TS_FILES)
file(GLOB TS_FILES ${CMAKE_SOURCE_DIR}/resource/i18n/*.ts)

set(SOURCES)
file(GLOB SOURCES
    *.cpp
    ui/*.cpp
)

set(HEADERS)
file(GLOB HEADERS
    *.h
    ui/*.h
)

set(QM_FILES)
configure_file(${CMAKE_SOURCE_DIR}/resource/translations.qrc ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc COPYONLY)

option(UPDATE_TRANSLATIONS "Creating/updating translation files from source" OFF)
if(UPDATE_TRANSLATIONS)
    message(STATUS "Creating/updating translation files from source")
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES})
else()
    message(STATUS "Compiling existing translation files")
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

if(SOURCES)
    add_executable(3DPDFConverterGUI
        ${SOURCES}
        ${HEADERS}
        ${CMAKE_SOURCE_DIR}/resource/resource.qrc
        ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc
        ${QM_FILES}
    )

    if(WIN32)
        target_sources(3DPDFConverterGUI PRIVATE ${CMAKE_SOURCE_DIR}/resource/version.rc.in)
        target_compile_definitions(3DPDFConverterGUI PRIVATE QT_OPENGL_DESKTOP)
    endif()

    target_include_directories(3DPDFConverterGUI PRIVATE
        ${CMAKE_SOURCE_DIR}/external/SARibbon
        ${CMAKE_SOURCE_DIR}/src/ExportFacade
        ${CMAKE_SOURCE_DIR}/src/CoreTypes
        ${CMAKE_SOURCE_DIR}/src/Logger
        ${CMAKE_CURRENT_SOURCE_DIR}/core_api
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
    )

    target_link_libraries(3DPDFConverterGUI PRIVATE
        CoreApi
        ExportFacade
        Logger
        Qt5::Widgets
        Qt5::Core
    )

    set_target_properties(3DPDFConverterGUI PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )

    # Copy DLLs on Windows
    if(WIN32)
        # Find libPRC and libharu DLLs
        file(GLOB 3DPDF_USE_LIBPRC_DLLS "${CMAKE_SOURCE_DIR}/3rdparty/libPRC/bin/*.dll")
        file(GLOB 3DPDF_USE_LIBHARU_DLLS "${CMAKE_SOURCE_DIR}/3rdparty/libharu-2.3.0/bin/*.dll")
        file(GLOB 3DPDF_ZLIB_DLLS "${CMAKE_SOURCE_DIR}/3rdparty/zlib/bin/*.dll")
        
        set(ALL_DLLS_TO_COPY 
            ${OpenCASCADE_TOOLKITS_DLLS} 
            ${OpenCASCADE_3RDPARTY_DLLS}
            ${3DPDF_USE_LIBPRC_DLLS}
            ${3DPDF_USE_LIBHARU_DLLS}
            ${3DPDF_ZLIB_DLLS}
        )
        if(ALL_DLLS_TO_COPY)
            add_custom_command(
                TARGET 3DPDFConverterGUI POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        ${ALL_DLLS_TO_COPY}
                        $<TARGET_FILE_DIR:3DPDFConverterGUI>
                COMMAND_EXPAND_LISTS
                COMMENT "Copying OpenCASCADE and 3rd-party runtime DLLs"
            )
        endif()
    endif()
endif()
