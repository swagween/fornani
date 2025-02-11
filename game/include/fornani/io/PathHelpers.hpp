#pragma once

/*
 * Cross-system helpers for getting path information
 * Supported platforms:
 * - Windows
 * - Linux
 * - macOS (Not tested but should work)
 * - FreeBSD, OpenBSD, NetBSD (Not tested but should work)
 */

#include <filesystem>

namespace fornani::io {

/**
 * @brief Gets a path to the calling executable.
 */
auto abs_exe_path() -> std::filesystem::path;

/**
 * @brief Gets a path to the directory of the calling executable.
 */
auto abs_exe_directory() -> std::filesystem::path;

/**
 * @brief Gets a path to the resources directory.
 */
auto get_resource_directory() -> std::filesystem::path;

/**
 * @brief Gets the current user's home directory.
 */
auto get_home_directory() -> std::filesystem::path;

} // namespace fornani::io
