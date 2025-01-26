include(CMakeDependentOption)

add_library(fornani)
add_library(fornani::game ALIAS fornani)

if (NOT DEFINED FORNANI_ROOT_DIR)
  message(FATAL_ERROR "FORNANI_ROOT_DIR has not been specified! Did you some how call the game cmake only?")
endif ()

include(${FORNANI_ROOT_DIR}/cmake/func/AddTargetSource.cmake)

target_link_libraries(fornani
        PUBLIC
        # TODO: Start removing all deps that are not used in public headers
        # The currently disabled targets should be fine.
        #Though disabling them means they are only accessible in .cpp files which is desired behavior.
        #djson::djson
        #ccmath::ccmath
        #ImGui-SFML::ImGui-SFML
        #SFML::System
        SFML::Graphics
        SFML::Window
        #SFML::Audio
        #steamworks

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

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang|IntelLLVM|AppleClang")
  if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC") # clang-cl
    add_compile_options(fornani INTERFACE /W4056 /W4146 /W4018 /W4056)
  endif ()
  add_compile_options(fornani INTERFACE -Wno-sign-conversion -Wno-format-security)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  add_compile_options(fornani INTERFACE /W4056 /W4146 /W4018 /W4056)
endif ()

