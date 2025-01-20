project(fornani-launcher)

add_executable(fornani-launcher)
target_link_libraries(fornani-launcher PRIVATE
        fornani::deps
        fornani::game
)

target_include_directories(fornani-launcher
        PRIVATE src
)

if(CMAKE_GENERATOR MATCHES "^(Visual Studio)")
    target_compile_options(fornani-launcher INTERFACE /MP)
endif()


if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    target_compile_options(fornani-launcher PRIVATE
            -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type
    )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    target_compile_options(fornani-launcher PRIVATE
            /W4 /WX
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(${PROJECT_NAME} INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(${PROJECT_NAME} INTERFACE -Wno-sign-conversion)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE FORNANI_STEAM_APP_ID=${FORNANI_STEAM_APP_ID})

add_subdirectory(src)
