cmake_minimum_required(VERSION 3.14)
project(QRBuilder)

set(CMAKE_CXX_STANDARD 17)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)

file(GLOB IMGUI_SOURCES
    imgui/*.cpp
    Stb/*.h
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_sdlrenderer3.cpp
)

add_executable(QRBuilder MACOSX_BUNDLE
    main.cpp
    qrcodegen/qrcodegen.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(QRBuilder PRIVATE
    qrcodegen
    imgui
    imgui/backends
)

set(QR_BASE_LIB_PATH "/Users/vulcain/Documents/builder-dep/")

target_link_directories(QRBuilder PRIVATE ${QR_BASE_LIB_PATH})

set(SDL3_LIB_PATH "${QR_BASE_LIB_PATH}libSDL3.0.dylib")
set(SDL3_IMG_LIB_PATH "${QR_BASE_LIB_PATH}libSDL3_image.0.2.4.dylib")


target_link_libraries(QRBuilder PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
)

set(MACOSX_BUNDLE_ICON_FILE "icon.icns")

set_source_files_properties(${CMAKE_SOURCE_DIR}/resources/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

set_target_properties(QRBuilder PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.in)

set(TARGET_DIR "$<TARGET_FILE_DIR:QRBuilder>")
set(DEST_LIB_DIR "${TARGET_DIR}/../Resources/lib")
set(DEST_LIB_SDL_PATH "${DEST_LIB_DIR}/SDL3.dylib")
set(DEST_LIB_SDL_IMG_PATH "${DEST_LIB_DIR}/SDL3_image.dylib")
set(APP_ICON "${CMAKE_SOURCE_DIR}/resources/icon.icns")

add_custom_command(TARGET QRBuilder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DEST_LIB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_LIB_PATH}" "${DEST_LIB_SDL_PATH}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL3_IMG_LIB_PATH}" "${DEST_LIB_SDL_IMG_PATH}"
)

add_custom_command(TARGET QRBuilder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Correction du rpath avec install_name_tool..."
    COMMAND install_name_tool -id "@rpath/SDL3.dylib" "${DEST_LIB_SDL_PATH}"
    COMMAND install_name_tool -id "@rpath/SDL3_image.dylib" "${DEST_LIB_SDL_IMG_PATH}"
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
