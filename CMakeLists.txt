
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if (NOT FORNANI_ROOT_DIR)
    set(FORNANI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

project(fornani
  LANGUAGES CXX
  VERSION 1.0
)

option(FORNANI_BUILD_EDITOR "Build the editor" OFF) # TODO: Bring editor into project

include(cmake/func/AddTargetSource.cmake)
include(cmake/GenerateDataTemplates.cmake)

add_subdirectory(dependencies)
add_subdirectory(game)
if (FORNANI_BUILD_EDITOR)
    add_subdirectory(editor)
else ()
    add_subdirectory(launcher)
endif ()

if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(${PROJECT_NAME} INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(${PROJECT_NAME} INTERFACE -Wno-sign-conversion)
endif()


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

