include(CMakeDependentOption)

add_library(${PROJECT_NAME})
add_library(fornani::game ALIAS fornani)

include(${FORNANI_ROOT_DIR}/cmake/func/AddTargetSource.cmake)


target_link_libraries(${PROJECT_NAME}
        PUBLIC
        # TODO: Start removing all deps that are not used in public headers
        djson::djson
        ccmath::ccmath
        ImGui-SFML::ImGui-SFML
        SFML::System
        SFML::Graphics
        SFML::Window
        SFML::Audio
        steamworks

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


if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(${PROJECT_NAME} INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(${PROJECT_NAME} INTERFACE -Wno-sign-conversion)
endif()
