include(CMakeDependentOption)

add_library(fornani)
add_library(fornani::game ALIAS fornani)

include(${FORNANI_ROOT_DIR}/cmake/func/AddTargetSource.cmake)


target_link_libraries(fornani
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
target_include_directories(fornani
        PUBLIC include "${CMAKE_CURRENT_BINARY_DIR}/include"
        PRIVATE "src"
)

add_subdirectory(include/fornani)
add_subdirectory(src)


if(CMAKE_CXX_COMPILER_ID STREQUAL MSVC)
    add_compile_options(fornani INTERFACE /W4056 /W4146 /W4018 /W4056)
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(fornani INTERFACE -Wno-sign-conversion)
endif()
