message(STATUS "Fetching SFML...")

# No need to build network module
set(SFML_BUILD_AUDIO TRUE)
set(SFML_BUILD_NETWORK FALSE)

FetchContent_MakeAvailable(sfml)
