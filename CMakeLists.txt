
cmake_minimum_required(VERSION 3.15)
project(fornani
  LANGUAGES CXX
  VERSION 1.0
)

set(CMAKE_CXX_STANDARD 20)

set(FORNANI_STEAM_APP_ID 3218760 CACHE STRING "The Steam app ID assigned to the game, leave to 480 for testing")
option(FORNANI_DEV_BUILD "If enabled, writes a steam_appid.txt file next to the executable, that will avoid the game from relaunching itself from Steam. DISABLE IN PRODUCTION" ON)

add_executable(${PROJECT_NAME})

include(cmake/func/AddTargetSource.cmake)
include(cmake/GenerateDataTemplates.cmake)


add_subdirectory(dependencies)
target_link_directories(${PROJECT_NAME}
        PRIVATE
        fornani::deps
)

# Setup our include structure
target_include_directories(${PROJECT_NAME}
        PUBLIC include "${CMAKE_CURRENT_BINARY_DIR}/include"
        PRIVATE "src"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE FORNANI_STEAM_APP_ID=${FORNANI_STEAM_APP_ID})

target_sources(${PROJECT_NAME} PRIVATE
main.cpp
)
add_subdirectory(include/fornani)
add_subdirectory(src)

if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(${PROJECT_NAME} INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(${PROJECT_NAME} INTERFACE -Wno-sign-conversion)
endif()

if (WIN32)
  add_custom_command(
          TARGET ${PROJECT_NAME}
          COMMENT "Copy Steam API DLL"
          PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${steamworks_dll_file} $<TARGET_FILE_DIR:${PROJECT_NAME}>/steam_api64.dll
          VERBATIM)

  #app icon
  #target_sources(${PROJECT_NAME} PRIVATE src/app/app.rc) #TODO: MOVE
  if(MSVC)
    set(subsystem $<IF:$<BOOL:$<CONFIG:Debug>>,CONSOLE,WINDOWS>)
    target_link_options(${PROJECT_NAME} PRIVATE
      /SUBSYSTEM:${subsystem}
      /ENTRY:mainCRTStartup
    )
  endif()
endif()

if(FORNANI_DEV_BUILD)
add_custom_command(
        TARGET ${PROJECT_NAME}
        COMMENT "Write steam_appid.txt"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E echo ${FORNANI_STEAM_APP_ID} > $<TARGET_FILE_DIR:${PROJECT_NAME}>/steam_appid.txt
        VERBATIM)
endif()