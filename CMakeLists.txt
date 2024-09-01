
cmake_minimum_required(VERSION 3.11)
project(fornani
  LANGUAGES CXX
  VERSION 1.0
)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME})

add_subdirectory(dependencies)
target_link_libraries(${PROJECT_NAME} PRIVATE djson::djson)
target_link_libraries(${PROJECT_NAME} PRIVATE ccmath::ccmath)

target_link_libraries(${PROJECT_NAME} PRIVATE ImGui-SFML::ImGui-SFML)
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-audio sfml-system sfml-window sfml-graphics)
target_link_libraries(${PROJECT_NAME} PRIVATE steamworks)

file(GLOB_RECURSE sources LIST_DIRECTORIES false CONFIGURE_DEPENDS "src/*.?pp")

target_sources(${PROJECT_NAME} PRIVATE
main.cpp
${sources}
)

if (WIN32)
  add_custom_command(
          TARGET ${PROJECT_NAME}
          COMMENT "Copy Steam API DLL"
          PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${steamworks_dll_file} $<TARGET_FILE_DIR:${PROJECT_NAME}>/steam_api64.dll
          VERBATIM)
endif()