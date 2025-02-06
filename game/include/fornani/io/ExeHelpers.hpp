
#pragma once

/*
 * Cross-platform helpers to get location of calling executable.
 * Supported platforms:
 * - Windows
 * - Linux
 * - macOS (Not tested but should work)
 */

#include <filesystem>

namespace fornani::io {
/**
 * @brief Gets a path to the calling executable
 */
auto abs_exe_path() -> std::filesystem::path;

/**
 * @brief Gets a path to the directory of the calling executable
 */
auto abs_exe_directory() -> std::filesystem::path;
}
