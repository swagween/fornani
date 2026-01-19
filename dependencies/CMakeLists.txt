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

# juke
FetchContent_Declare(
        juke
        GIT_REPOSITORY https://github.com/swagween/jukebox
        # GIT_TAG v1.0.0
        GIT_TAG 95c8b702aa02b1c5147e0999d2d49020abeecf9f
)
FetchContent_MakeAvailable(juke)

# djson
FetchContent_Declare(
        djson
        GIT_REPOSITORY https://github.com/karnkaul/djson
        GIT_TAG v3.1.5
)
FetchContent_MakeAvailable(djson)

# ccmath
set(CCMATH_ENABLE_EXTENSIONS ON)
set(CCMATH_DISABLE_ERRNO ON)
set(CCMATH_STRICT_WARNINGS OFF)
FetchContent_Declare(
        ccmath
        GIT_REPOSITORY https://github.com/Rinzii/ccmath
        GIT_TAG 15260da # IanP: This specific commit id is stable but more recent than main
)
FetchContent_MakeAvailable(ccmath)

# Used for assertions and crash handling

FetchContent_Declare(
        cpptrace
        GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace
        GIT_TAG v0.7.5
)
FetchContent_MakeAvailable(cpptrace)

FetchContent_Declare(
        libassert
        GIT_REPOSITORY https://github.com/jeremy-rifkin/libassert
        GIT_TAG v2.1.4
)
FetchContent_MakeAvailable(libassert)



# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24+
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

# Replace the old steamworks_archive with a new fetch from your GitHub release:
FetchContent_Declare(
        steamworks_sdk
        URL https://github.com/swagween/fornani/releases/download/Steamworks_v1.61.0/SteamworksSDK-v1.61.0_x64.zip
)
FetchContent_MakeAvailable(steamworks_sdk)

# Create an INTERFACE library to hold include paths & linking info
add_library(steamworks INTERFACE)
target_include_directories(steamworks SYSTEM INTERFACE
        $<BUILD_INTERFACE:${steamworks_sdk_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# On Windows, link the 64-bit library & define _CRT_SECURE_NO_WARNINGS
if(WIN32)
    target_link_libraries(steamworks INTERFACE
            "${steamworks_sdk_SOURCE_DIR}/lib/steam/steam_api64.lib"
    )
    target_compile_definitions(steamworks INTERFACE _CRT_SECURE_NO_WARNINGS)

    # Keep track of the DLL location in case you want to copy it next to your exe
    set(STEAMWORKS_DYNLIB_FILE
            "${steamworks_sdk_SOURCE_DIR}/bin/steam/steam_api64.dll"
            CACHE STRING "Steamworks DLL location"
    )
    set(STEAMWORKS_DYNLIB_FILE_NAME
            "steam_api64.dll"
            CACHE STRING "Steamworks DLL nam"
    )
elseif(APPLE)
    # On Unix like systems, link the .so
    target_link_libraries(steamworks INTERFACE
            "${steamworks_sdk_SOURCE_DIR}/lib/steam/libsteam_api.dylib"
    )
    set(STEAMWORKS_DYNLIB_FILE
            "${steamworks_sdk_SOURCE_DIR}/lib/steam/libsteam_api.dylib"
            CACHE STRING "Steamworks DLL location"
    )
    set(STEAMWORKS_DYNLIB_FILE_NAME
            "libsteam_api.dylib"
            CACHE STRING "Steamworks DLL nam"
    )
elseif(UNIX)
    # On Unix like systems, link the .so
    target_link_libraries(steamworks INTERFACE
            "${steamworks_sdk_SOURCE_DIR}/bin/steam/libsteam_api.so"
    )
endif()

# (Optional) Provide a function to copy the Steamworks DLL next to an .exe
# Call this from your main project once you know the name of your executable.
function(copy_steamworks_dll target)
    if(WIN32 OR APPLE)
        add_custom_command(
                TARGET ${target}
                COMMENT "Copying Steamworks DLL next to ${target}"
                PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${STEAMWORKS_DYNLIB_FILE} $<TARGET_FILE_DIR:${target}>/${STEAMWORKS_DYNLIB_FILE_NAME}
                VERBATIM
        )
    endif()
endfunction()

add_library(fornani-dependencies INTERFACE)
add_library(fornani::deps ALIAS fornani-dependencies)

target_link_libraries(fornani-dependencies INTERFACE
        djson::djson
        ccmath::ccmath
        ImGui-SFML::ImGui-SFML
        SFML::System
        SFML::Graphics
        SFML::Window
        juke::juke
        steamworks

        cpptrace::cpptrace
        libassert::assert
)

if (WIN32 AND NOT FORNANI_PRODUCTION_BUILD AND NOT DEFINED FORNANI_DISABLE_DBGHELP)
    target_link_libraries(fornani-dependencies INTERFACE
            Dbghelp
    )
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang|IntelLLVM|AppleClang")
    if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC") # clang-cl
        add_compile_options(fornani-dependencies INTERFACE /W0)
    endif ()
    add_compile_options(fornani-dependencies INTERFACE -w)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options(fornani-dependencies INTERFACE /W0)
endif ()
