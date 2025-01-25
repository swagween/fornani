
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

option(FORNANI_DEV_BUILD "If enabled, writes a steam_appid.txt file next to the executable, that will avoid the game from relaunching itself from Steam. DISABLE IN PRODUCTION" ON)
option(FORNANI_BUILD_EDITOR "Build the editor" OFF) # TODO: Bring editor into project
option(FORNANI_ENABLE_DEV_ID "Enables dev ID for steam" OFF)
option(FORNANI_ENABLE_PROFILER "Enables tracy profiling" ON)

include(cmake/func/AddTargetSource.cmake)
include(cmake/GenerateDataTemplates.cmake)

add_subdirectory(dependencies)
add_subdirectory(game)
if (FORNANI_BUILD_EDITOR)
  message(STATUS "FORNANI: Building editor!")
  add_subdirectory(editor)
else ()
  message(STATUS "FORNANI: Building launcher!")
  add_subdirectory(launcher)
endif ()

if (FORNANI_DEV_BUILD)
  message(STATUS "FORNANI: FORNANI_DEV_BUILD is ON. Will write steam_appid.txt next to executable!")
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
  add_compile_options(fornani INTERFACE /W4056 /W4146 /W4018 /W4056)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(fornani INTERFACE -Wno-sign-conversion)
endif ()

if (FORNANI_ENABLE_PROFILER)
  target_compile_definitions(fornani PRIVATE TRACY_ENABLE)
  target_compile_definitions(fornani PRIVATE FORNANI_PROFILE)
endif ()


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

