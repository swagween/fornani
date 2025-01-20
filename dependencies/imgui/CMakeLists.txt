cmake_minimum_required(VERSION 3.18)

# Fetch imgui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.91.6
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_SOURCE_DIR ${imgui_SOURCE_DIR} CACHE STRING "ImGui source directory")

project(imgui VERSION 1.91.6)

include(FetchContent)



# Find OpenGL
find_package(OpenGL REQUIRED)

# Add the imgui library target
add_library(imgui)

# Set include directories
target_include_directories(imgui SYSTEM PUBLIC
        ${IMGUI_SOURCE_DIR}
        ${IMGUI_SOURCE_DIR}/backends
)

# Specify source files for the library
target_sources(imgui PRIVATE
        ${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.h
        ${IMGUI_SOURCE_DIR}/imgui.cpp
        ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
        ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
        ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
        ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
)

# Link required libraries, including OpenGL
target_link_libraries(imgui PRIVATE
        OpenGL::GL
)

# Compiler-specific flags
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(imgui PRIVATE
          -Wno-conversion
  )
endif()
