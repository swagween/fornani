project(fornani-dependencies)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF)

# SFML
FetchContent_Declare(sfml
        GIT_REPOSITORY https://github.com/SFML/SFML
        GIT_TAG 3.0.0
)
FetchContent_MakeAvailable(sfml)

# Dear ImGui
add_subdirectory(imgui)

# Dear ImGui-SFML
set(IMGUI_SFML_FIND_SFML OFF)
add_subdirectory(imgui-sfml)

# djson
FetchContent_Declare(
        djson
        GIT_REPOSITORY https://github.com/karnkaul/djson
        GIT_TAG v2.1.1
)
FetchContent_MakeAvailable(djson)

# tracy
FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy
        GIT_TAG v0.11.1
)
FetchContent_MakeAvailable(tracy)

# ccmath
set(CCMATH_ENABLE_EXTENSIONS ON)
set(CCMATH_DISABLE_ERRNO ON)
set(CCMATH_STRICT_WARNINGS OFF)
FetchContent_Declare(
        ccmath
        GIT_REPOSITORY https://github.com/Rinzii/ccmath
        GIT_TAG 7507508 # IanP: This specific commit id is stable but more recent than main
)
FetchContent_MakeAvailable(ccmath)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24 with steamworks.
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif ()

# steamworks API
FetchContent_Declare(
        steamworks_archive
        URL https://github.com/aleokdev/fornani/releases/download/deck-test-1/steamworks_sdk_160.zip
)
FetchContent_MakeAvailable(steamworks_archive)

add_library(steamworks INTERFACE)
target_include_directories(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/public)
if (WIN32)
    target_link_libraries(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/redistributable_bin/win64/steam_api64.lib)
    if (MSVC)
        # Avoid hundreds of strncpy deprecation messages
        target_compile_definitions(steamworks INTERFACE _CRT_SECURE_NO_WARNINGS)
    endif ()

    set(steamworks_dll_file ${steamworks_archive_SOURCE_DIR}/redistributable_bin/win64/steam_api64.dll CACHE STRING "Steamworks DLL")
elseif (UNIX)
    target_link_libraries(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/redistributable_bin/linux64/libsteam_api.so)
endif ()

add_library(fornani-dependencies INTERFACE)
add_library(fornani::deps ALIAS fornani-dependencies)

target_link_libraries(fornani-dependencies INTERFACE
        djson::djson
        ccmath::ccmath
        ImGui-SFML::ImGui-SFML
        SFML::System
        SFML::Graphics
        SFML::Window
        SFML::Audio
        steamworks
        Tracy::TracyClient
)

if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(fornani-dependencies INTERFACE /W2)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(fornani-dependencies INTERFACE -Wno-sign-conversion)
endif()
