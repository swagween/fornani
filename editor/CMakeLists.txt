project(fornani-editor)

if (NOT DEFINED FORNANI_ROOT_DIR)
  message(FATAL_ERROR "FORNANI_ROOT_DIR has not been specified! Did you some how call the editor cmake only?")
endif ()

add_executable(fornani-editor)
set_target_properties(fornani-editor PROPERTIES OUTPUT_NAME "fornani-editor")
target_link_libraries(fornani-editor PRIVATE
        steamworks
        fornani::deps
        fornani::game
)

target_include_directories(fornani-editor
        PUBLIC include "${CMAKE_CURRENT_BINARY_DIR}/include"
        PRIVATE "src"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(fornani-editor INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(fornani-editor INTERFACE -Wno-sign-conversion)
endif()

if (FORNANI_STEAM_APP_ID OR FORNANI_ENABLE_DEV_ID)
  if (NOT DEFINED FORNANI_STEAM_APP_ID)
    set(FORNANI_STEAM_APP_ID 480 CACHE STRING "Developer Steam App ID: 480")
  endif ()
  target_compile_definitions(${PROJECT_NAME} PUBLIC FORNANI_STEAM_APP_ID=${FORNANI_STEAM_APP_ID})
else ()
  set(FORNANI_STEAM_APP_ID ${FORNANI_STEAM_APP_ID_DEFAULT} CACHE STRING "Default Steam App ID: 3218760")
  target_compile_definitions(${PROJECT_NAME} PUBLIC FORNANI_STEAM_APP_ID=${FORNANI_STEAM_APP_ID})
endif ()
message(STATUS "FORNANI: Building with steam app ID: ${FORNANI_STEAM_APP_ID}")


add_subdirectory(include/editor)
add_subdirectory(src)

if (WIN32)
    add_custom_command(
            TARGET fornani-editor
            COMMENT "Copy Steam API DLL"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${steamworks_dll_file} $<TARGET_FILE_DIR:fornani-editor>/steam_api64.dll
            VERBATIM
    )

    if(MSVC)
        set(subsystem $<IF:$<BOOL:$<CONFIG:Debug>>,CONSOLE,WINDOWS>)
        target_link_options(fornani-editor PRIVATE
                /SUBSYSTEM:${subsystem}
                /ENTRY:mainCRTStartup
        )
    endif()
endif()

if(FORNANI_DEV_BUILD)
    add_custom_command(
            TARGET fornani-editor
            COMMENT "Write steam_appid.txt"
            PRE_LINK COMMAND ${CMAKE_COMMAND} -E echo ${FORNANI_STEAM_APP_ID} > $<TARGET_FILE_DIR:${PROJECT_NAME}>/steam_appid.txt
            VERBATIM
    )
endif()
