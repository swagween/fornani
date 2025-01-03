include(FetchContent)
set(BUILD_SHARED_LIBS OFF)

# SFML
FetchContent_Declare(sfml
  GIT_REPOSITORY https://github.com/SFML/SFML
  GIT_TAG 7f1162dfea4969bc17417563ac55d93b72e84c1e # 3.0.0
)
FetchContent_MakeAvailable(sfml)

# Dear ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG 993fa347495860ed44b83574254ef2a317d0c14f # 1.91.6
)

FetchContent_MakeAvailable(imgui)

# ImGui-SFML
FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml
  GIT_TAG 5e8f8fb857c8f3cff1096bf3e4402c64e3bcf6af
)
add_subdirectory(imgui-sfml)

# djson
FetchContent_Declare(
    djson
    GIT_REPOSITORY https://github.com/karnkaul/djson
    GIT_TAG v2.1.1
)
FetchContent_MakeAvailable(djson)

# ccmath
set(CCMATH_ENABLE_EXTENSIONS ON)
set(CCMATH_DISABLE_ERRNO ON)
FetchContent_Declare(
        ccmath
        GIT_REPOSITORY https://github.com/Rinzii/ccmath
        GIT_TAG d921f12ef27f92ad08251a0f40dfb1af557c5fe6
)
FetchContent_MakeAvailable(ccmath)

# steamworks API
FetchContent_Declare(
  steamworks_archive
  URL https://github.com/aleokdev/fornani/releases/download/deck-test-1/steamworks_sdk_160.zip
)
FetchContent_MakeAvailable(steamworks_archive)

add_library(steamworks INTERFACE)
target_include_directories(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/public)
if (WIN32)
  target_link_libraries(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/redistributable_bin/win64/steam_api64.lib)
  if (MSVC)
    # Avoid hundreds of strncpy deprecation messages
    target_compile_definitions(steamworks INTERFACE _CRT_SECURE_NO_WARNINGS)
  endif()
  
  set(steamworks_dll_file ${steamworks_archive_SOURCE_DIR}/redistributable_bin/win64/steam_api64.dll PARENT_SCOPE)
elseif (UNIX)
  target_link_libraries(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/redistributable_bin/linux64/libsteam_api.so)
endif()
