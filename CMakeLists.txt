
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if (NOT DEFINED FORNANI_ROOT_DIR)
  set(FORNANI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "Fornani root directory")
endif ()

project(fornani
        LANGUAGES CXX
        VERSION 1.0
)

set(FORNANI_STEAM_APP_ID_DEFAULT 3218760 CACHE STRING "The Steam app ID assigned to the game, leave to 480 for testing")

option(
        FORNANI_DEV_BUILD
        "If enabled, writes a steam_appid.txt file next to the executable,
        that will avoid the game from relaunching itself from Steam. DISABLE IN PRODUCTION"
        ON
)
option(FORNANI_BUILD_EDITOR "Build the editor. This should never be enabled in production!" OFF)
option(FORNANI_ENABLE_DEV_ID "Enables dev ID for steam. This should never be enabled in production!" OFF)
option(FORNANI_ENABLE_PROFILER "Enables tracy profiling. This should never be enabled in production!" OFF)
option(
        FORNANI_ENABLE_VERBOSE_LOGGING
        "Enable verbose logging that will output source location information.
        You should only have this enabled if you are debugging!
        This should never be enabled in production!"
        OFF
)

include(cmake/func/AddTargetSource.cmake)
include(cmake/GenerateDataTemplates.cmake)

add_subdirectory(dependencies)
add_subdirectory(game)
if (FORNANI_BUILD_EDITOR)
  message(STATUS "FORNANI: Building editor!")
  add_subdirectory(editor)
endif ()
message(STATUS "FORNANI: Building launcher!")
add_subdirectory(launcher)

if (FORNANI_DEV_BUILD)
  message(STATUS "FORNANI: FORNANI_DEV_BUILD is ON. Will write steam_appid.txt next to executable!")
endif ()

if (FORNANI_ENABLE_PROFILER)
  message(STATUS "FORNANI: FORNANI_ENABLE_PROFILER is ON. Profiling will now be enabled in the executable!")
endif ()

if (FORNANI_ENABLE_VERBOSE_LOGGING)
  message(STATUS "FORNANI: FORNANI_ENABLE_VERBOSE_LOGGING is ON. Logger will now output verbose logging information!")
endif ()

if (FORNANI_ENABLE_PROFILER)
  target_compile_definitions(fornani PRIVATE TRACY_ENABLE)
  target_compile_definitions(fornani PRIVATE FORNANI_PROFILE)
endif ()

# TODO: This is kinda janky and is only a stop gap. At some point we will have cmake handle packaging and this will have to change.

# Generate save files
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/save/file_0.template" "${FORNANI_ROOT_DIR}/resources/data/save/file_0.json" COPYONLY)
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/save/file_1.template" "${FORNANI_ROOT_DIR}/resources/data/save/file_1.json" COPYONLY)
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/save/file_2.template" "${FORNANI_ROOT_DIR}/resources/data/save/file_2.json" COPYONLY)
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/save/new_game.template" "${FORNANI_ROOT_DIR}/resources/data/save/new_game.json" COPYONLY)

# Generate config files
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/config/control_map.template" "${FORNANI_ROOT_DIR}/resources/data/config/control_map.json" COPYONLY)
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/config/defaults.template" "${FORNANI_ROOT_DIR}/resources/data/config/defaults.json" COPYONLY)
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/config/settings.template" "${FORNANI_ROOT_DIR}/resources/data/config/settings.json" COPYONLY)
configure_file("${FORNANI_ROOT_DIR}/cmake/templates/data/config/version.template" "${FORNANI_ROOT_DIR}/resources/data/config/version.json" COPYONLY)

