
cmake_minimum_required(VERSION 3.11)
project(fornani
  LANGUAGES CXX
  VERSION 1.0
)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME})

include(FetchContent)
FetchContent_Declare(
    djson
    GIT_REPOSITORY https://github.com/karnkaul/djson
    GIT_TAG v2.1.1
)
FetchContent_Declare(
        ccmath
        GIT_REPOSITORY https://github.com/Rinzii/ccmath.git
        GIT_TAG 26a0e1845eebc775121964b17a3fc5bd9c330831
)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(djson)
FetchContent_MakeAvailable(ccmath)
target_link_libraries(${PROJECT_NAME} PRIVATE djson::djson)
target_link_libraries(${PROJECT_NAME} PRIVATE ccmath::ccmath)

set(BUILD_SHARED_LIBS ON)
add_subdirectory(dependencies)

target_link_libraries(${PROJECT_NAME} PRIVATE ImGui-SFML::ImGui-SFML)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-audio)

file(GLOB_RECURSE sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "src/*.?pp")

target_sources(${PROJECT_NAME} PRIVATE
main.cpp
${sources}
)

# Copy DLLs needed for runtime on Windows
if(WIN32)
  if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_FILE:ImGui-SFML::ImGui-SFML>
          $<TARGET_FILE:sfml-graphics>
	      $<TARGET_FILE:sfml-audio>
          $<TARGET_FILE:sfml-window>
          $<TARGET_FILE:sfml-system>
          $<TARGET_FILE_DIR:${PROJECT_NAME}>)
  endif()
endif()
