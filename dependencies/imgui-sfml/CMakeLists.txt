message(STATUS "Fetching ImGui-SFML...")

set(IMGUI_DIR ${IMGUI_INCLUDE_DIR})
set(IMGUI_SFML_FIND_SFML OFF)
set(IMGUI_SFML_IMGUI_DEMO ON)

# ImGui-SFML
FetchContent_Declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml
        GIT_TAG v3.0
)

FetchContent_MakeAvailable(imgui-sfml)
