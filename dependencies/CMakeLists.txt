include(FetchContent)
set(BUILD_SHARED_LIBS OFF)

# SFML
set(BUILD_SHARED_LIBS ON)
FetchContent_Declare(sfml
  GIT_REPOSITORY https://github.com/SFML/SFML
  #GIT_TAG 2f11710abc5aa478503a7ff3f9e654bd2078ebab
  GIT_TAG 69ea0cd863aed1d4092b970b676924a716ff718b # 2.6.1
)
FetchContent_MakeAvailable(sfml)
set(BUILD_SHARED_LIBS OFF)

# Dear ImGui
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG 35b1148efb839381b84de9290d9caf0b66ad7d03 # 1.82
)

FetchContent_MakeAvailable(imgui)

# ImGui-SFML
FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml
  GIT_TAG 82dc2033e51b8323857c3ae1cf1f458b3a933c35 # 2.3
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
FetchContent_Declare(
        ccmath
        GIT_REPOSITORY https://github.com/Rinzii/ccmath.git
        GIT_TAG 26a0e1845eebc775121964b17a3fc5bd9c330831
)
FetchContent_MakeAvailable(ccmath)

# steamworks API
FetchContent_Declare(
  steamworks_archive
  URL https://partner.steamgames.com/downloads/steamworks_sdk_160.zip
)
FetchContent_MakeAvailable(steamworks_archive)

add_library(steamworks INTERFACE)
target_include_directories(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/public)
if (WIN32)
  target_link_libraries(steamworks INTERFACE ${steamworks_archive_SOURCE_DIR}/redistributable_bin/win64/steam_api64.lib)
endif()

set(steamworks_dll_file ${steamworks_archive_SOURCE_DIR}/redistributable_bin/win64/steam_api64.dll PARENT_SCOPE)