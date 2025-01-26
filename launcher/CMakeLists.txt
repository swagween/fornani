project(fornani-launcher)

if (NOT DEFINED FORNANI_ROOT_DIR)
  message(FATAL_ERROR "FORNANI_ROOT_DIR has not been specified! Did you some how call the launcher cmake only?")
endif ()

add_executable(fornani-launcher)
set_target_properties(fornani-launcher PROPERTIES OUTPUT_NAME "fornani")
target_link_libraries(fornani-launcher PRIVATE
        steamworks
        fornani::deps
        fornani::game
)

target_include_directories(fornani-launcher
        PUBLIC include "${CMAKE_CURRENT_BINARY_DIR}/include"
        PRIVATE "src"
)

if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(fornani-launcher INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(fornani-launcher INTERFACE -Wno-sign-conversion)
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

add_subdirectory(include/launcher)
add_subdirectory(src)

if (WIN32)
    add_custom_command(
            TARGET fornani-launcher
            COMMENT "Copy Steam API DLL"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${steamworks_dll_file} $<TARGET_FILE_DIR:fornani-launcher>/steam_api64.dll
            VERBATIM
    )

    if(MSVC)
        set(subsystem $<IF:$<BOOL:$<CONFIG:Debug>>,CONSOLE,WINDOWS>)
        target_link_options(fornani-launcher PRIVATE
                /SUBSYSTEM:${subsystem}
                /ENTRY:mainCRTStartup
        )
    endif()
endif()

if(FORNANI_DEV_BUILD)
    add_custom_command(
            TARGET fornani-launcher
            COMMENT "Write steam_appid.txt"
            PRE_LINK COMMAND ${CMAKE_COMMAND} -E echo ${FORNANI_STEAM_APP_ID} > $<TARGET_FILE_DIR:${PROJECT_NAME}>/steam_appid.txt
            VERBATIM
    )
endif()
