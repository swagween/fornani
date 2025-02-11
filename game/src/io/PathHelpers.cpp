#include "fornani/io/PathHelpers.hpp"

#include <array>
#include <filesystem>

#if defined(_WIN32) || defined(_WIN64)
#include "fornani/core/platform/win32/Windows.hpp"
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#include <string>
#else
#error "Unsupported platform."
#endif

namespace fornani::io {

auto abs_exe_path() -> std::filesystem::path {
#if defined(_WIN32) || defined(_WIN64)
	std::array<wchar_t, FILENAME_MAX> path = {0};
	if (GetModuleFileNameW(nullptr, path.data(), FILENAME_MAX) == 0) { throw std::runtime_error("Failed to get executable path on Windows."); }
	return {path.data()};

#elif defined(__APPLE__)
	std::array<char, FILENAME_MAX> path;
	uint32_t size = static_cast<uint32_t>(path.size());
	if (_NSGetExecutablePath(path.data(), &size) != 0) { throw std::runtime_error("Executable path is too long on macOS."); }
	return std::filesystem::canonical(std::filesystem::path(path.data()));

#elif defined(__linux__)
	std::array<char, FILENAME_MAX> path;
	ssize_t count = readlink("/proc/self/exe", path.data(), FILENAME_MAX);
	if (count == -1) { throw std::runtime_error("Failed to get executable path on Linux."); }
	return std::filesystem::canonical(std::filesystem::path(std::string(path.data(), count)));

#else
#error "Unsupported platform."
#endif
}

auto abs_exe_directory() -> std::filesystem::path { return abs_exe_path().parent_path(); }

auto get_resource_directory() -> std::filesystem::path {
#ifndef NANI_PRODUCTION
	namespace fs = std::filesystem;
	// Start at the directory containing the executable.
	fs::path currentDir = abs_exe_directory();

	// Continue moving up the directory tree until we reach the root.
	while (!currentDir.empty()) {
		// Use these paths to determine if we are in the root directory
		fs::path const dependenciesDir = currentDir / "dependencies";
		fs::path const gameDir = currentDir / "game";
		fs::path const launcherDir = currentDir / "launcher";
		fs::path const resourcesDir = currentDir / "resources";

		// Check if all required directories exist and are directories.
		if (fs::is_directory(dependenciesDir) && fs::is_directory(gameDir) && fs::is_directory(launcherDir) && fs::is_directory(resourcesDir)) {
			// Found the correct root; return the absolute (canonical) resources directory.
			return fs::canonical(resourcesDir);
		}

		// Move up one directory. If we have reached the top (or cannot go higher), break.
		fs::path parent = currentDir.parent_path();
		if (parent == currentDir) // Reached the filesystem root.
		{
			break;
		}
		currentDir = parent;
	}
#endif
	// If not found, return assume the resources directory is in the same directory as the executable.
	return abs_exe_directory() / "resources";
}

} // namespace fornani::io
