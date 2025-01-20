
cmake_minimum_required(VERSION 3.15)
project(fornani-game
        LANGUAGES CXX
        VERSION 1.0
)

add_library(${PROJECT_NAME})
add_library(fornani::game ALIAS fornani-game)

include(${FORNANI_ROOT_DIR}/cmake/func/AddTargetSource.cmake)


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

add_subdirectory(include/fornani)
add_subdirectory(src)

if (WIN32)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            COMMENT "Copy Steam API DLL"
            PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${steamworks_dll_file} $<TARGET_FILE_DIR:${PROJECT_NAME}>/steam_api64.dll
            VERBATIM)

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