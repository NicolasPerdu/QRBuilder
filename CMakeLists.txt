cmake_minimum_required(VERSION 3.14)
project(QRBuilder)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL3 REQUIRED CONFIG)

if(WIN32)
    file(GLOB IMGUI_SOURCES
        imgui/*.cpp
        stb/*.h
        *.h
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_sdlrenderer3.cpp
    )

    add_executable(QRBuilder
        main.cpp
        qrcodegen/qrcodegen.cpp
        ${IMGUI_SOURCES}
        icon.rc
    )

    set_target_properties(QRBuilder PROPERTIES WIN32_EXECUTABLE TRUE)

else()
    file(GLOB IMGUI_SOURCES
        imgui/*.cpp
        stb/*.h
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_sdlrenderer3.cpp
    )

    set(EXECUTABLE_OPTIONS "")

    if(APPLE)
        set(EXECUTABLE_OPTIONS MACOSX_BUNDLE)
    endif()

    add_executable(QRBuilder ${EXECUTABLE_OPTIONS}
        main.cpp
        qrcodegen/qrcodegen.cpp
        ${IMGUI_SOURCES}
    )
endif()

target_include_directories(QRBuilder PRIVATE
    qrcodegen
    imgui
    imgui/backends
)

target_link_libraries(QRBuilder PRIVATE
    SDL3::SDL3
)

if(WIN32)
    foreach(flag_var
        CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_DEBUG)
        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endforeach()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(STATIC_MSVC_LIBS
            legacy_stdio_definitions.lib
            oldnames.lib
            libcmtd.lib
            libucrtd.lib
            libvcruntimed.lib
        )
    else()
        set(STATIC_MSVC_LIBS
            legacy_stdio_definitions.lib
            oldnames.lib
            libcmt.lib
            libucrt.lib
            libvcruntime.lib
        )
    endif()

    target_link_libraries(QRBuilder PRIVATE 
        SDL3::SDL3
        ${STATIC_MSVC_LIBS}
    )

    target_compile_options(QRBuilder PRIVATE /MT)
elseif(APPLE)
    set(QR_BASE_LIB_PATH "" CACHE PATH "Path to the dependencies")

    if(NOT QR_BASE_LIB_PATH)
        message(WARNING "QR_BASE_LIB_PATH is not defined !")
    else()
        message(STATUS "QR_BASE_LIB_PATH = ${QR_BASE_LIB_PATH}")
    endif()

    target_link_directories(QRBuilder PRIVATE ${QR_BASE_LIB_PATH})

    set(SDL3_LIB_PATH "${QR_BASE_LIB_PATH}/libSDL3.0.dylib")

    set(MACOSX_BUNDLE_ICON_FILE "icon.icns")

    set_source_files_properties(${CMAKE_SOURCE_DIR}/resources/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(QRBuilder PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in)

    set(TARGET_DIR "$<TARGET_FILE_DIR:QRBuilder>")
    set(DEST_LIB_DIR "${TARGET_DIR}/../Resources/lib")
    set(DEST_LIB_SDL_PATH "${DEST_LIB_DIR}/SDL3.dylib")
    set(APP_ICON "${CMAKE_SOURCE_DIR}/resources/icon.icns")

    add_custom_command(TARGET QRBuilder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_LIB_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_LIB_PATH}" "${DEST_LIB_SDL_PATH}"
    )

    add_custom_command(TARGET QRBuilder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Correction du rpath avec install_name_tool..."
        COMMAND install_name_tool -id "@rpath/SDL3.dylib" "${DEST_LIB_SDL_PATH}"
    )

    add_custom_command(TARGET QRBuilder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Correction du rpath avec install_name_tool..."
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/fix_rpath.sh $<TARGET_FILE:QRBuilder>
    )

    add_custom_command(TARGET QRBuilder POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${APP_ICON}"
            "$<TARGET_BUNDLE_DIR:QRBuilder>/Contents/Resources/icon.icns"
    )

endif()
