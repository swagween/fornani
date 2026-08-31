
cmake_minimum_required(VERSION 3.5...3.29)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT DEFINED FORNANI_ROOT_DIR)
    set(FORNANI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Fornani root directory")
endif ()

project(fornani
    VERSION 4.4.2
    LANGUAGES CXX
)

set(FORNANI_CHANNEL "alpha")
set(version_file "${CMAKE_CURRENT_BINARY_DIR}/game/include/fornani/app/build_version.hpp")
configure_file(game/src/app/build_version.hpp.in "${version_file}" @ONLY)

set(FORNANI_STEAM_APP_ID_DEFAULT 3218760 CACHE STRING "The default Steam app ID")
set(FORNANI_STEAM_APP_ID_DEV 480 CACHE STRING "The developer Steam app ID (Spacewar)")

option(FORNANI_PRODUCTION_BUILD "If enabled, will build the game in production mode" OFF)
option(
        FORNANI_DEV_BUILD
        "If enabled, writes a steam_appid.txt file next to the executable
        that will stop the game from relaunching itself through Steam. DISABLE IN PRODUCTION"
        ON
)
option(FORNANI_BUILD_EDITOR "Build the editor. This should never be enabled in production!" ON)
option(FORNANI_ENABLE_DEV_ID "Enables dev ID for steam. This should never be enabled in production!" OFF)
option(
        FORNANI_ENABLE_VERBOSE_LOGGING
        "Enable verbose logging that will output source location information.
        You should only have this enabled if you are debugging!
        This should never be enabled in production!"
        OFF
)

include(cmake/helpers/OptionMessages.cmake)

include(cmake/func/AddTargetSource.cmake)
include(cmake/GenerateDataTemplates.cmake)

add_subdirectory(dependencies)
add_subdirectory(game)
if (FORNANI_BUILD_EDITOR)
    add_subdirectory(editor)
endif ()
add_subdirectory(launcher)


if (FORNANI_ENABLE_PROFILER AND NOT FORNANI_PRODUCTION_BUILD)
    target_compile_definitions(fornani PRIVATE FORNANI_PROFILE)
endif ()

if (FORNANI_DISABLE_DBGHELP)
    target_compile_definitions(fornani PRIVATE NANI_DISABLE_ADVANCED_CRASH_REPORTING)
endif ()

if (FORNANI_PRODUCTION_BUILD)
    target_compile_definitions(fornani PRIVATE FORNANI_PRODUCTION)
endif ()

if (WIN32)
    target_link_options(fornani PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/HEAP:536870912>
        $<$<CXX_COMPILER_ID:Clang>:/HEAP:536870912>
        $<$<CXX_COMPILER_ID:GNU>:-Wl,--heap,536870912>
    )
endif()

    target_compile_options(fornani PRIVATE
        $<$<CONFIG:Debug>:/RTC1>
    )

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN AND MSVC)
    add_compile_options(/fsanitize=address /Zi)
    add_link_options(/INFERASANLIBS /DEBUG)
endif()

