project(fornani-launcher)

add_executable(fornani-launcher)
target_link_libraries(fornani-launcher PRIVATE
        steamworks
        fornani::deps
        fornani::game
)

target_include_directories(fornani-launcher
        PRIVATE src
)


if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(fornani-launcher INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(fornani-launcher INTERFACE -Wno-sign-conversion)
endif()

if (FORNANI_STEAM_APP_ID)
    target_compile_definitions(fornani-launcher PRIVATE FORNANI_STEAM_APP_ID=${FORNANI_STEAM_APP_ID})
    message(STATUS "Building ${PROJECT_NAME} with Steam App ID: ${FORNANI_STEAM_APP_ID}")
else()
    target_compile_definitions(fornani-launcher PRIVATE FORNANI_STEAM_APP_ID=${FORNANI_STEAM_APP_ID_DEFAULT})
    message(STATUS "Building ${PROJECT_NAME} with Steam App ID: ${FORNANI_STEAM_APP_ID_DEFAULT}")
endif()




add_subdirectory(src)

if (WIN32)
    add_custom_command(
            TARGET fornani-launcher
            COMMENT "Copy Steam API DLL"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${steamworks_dll_file} $<TARGET_FILE_DIR:fornani-launcher>/steam_api64.dll
            VERBATIM)

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
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E echo ${FORNANI_STEAM_APP_ID} > $<TARGET_FILE_DIR:fornani-launcher>/steam_appid.txt
            VERBATIM
    )
endif()

# Move our template files into the resource directory
