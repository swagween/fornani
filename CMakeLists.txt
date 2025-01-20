
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if (NOT FORNANI_ROOT_DIR)
    set(FORNANI_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
endif()

project(fornani
  LANGUAGES CXX
  VERSION 1.0
)

option(FORNANI_BUILD_EDITOR "Build the editor" OFF)

include(cmake/func/AddTargetSource.cmake)
include(cmake/GenerateDataTemplates.cmake)

add_subdirectory(dependencies)
add_subdirectory(game)
if (FORNANI_BUILD_EDITOR)
    add_subdirectory(editor)
else ()
    add_subdirectory(launcher)
endif ()
