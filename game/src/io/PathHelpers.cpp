#include "fornani/io/PathHelpers.hpp"

#include <array>
#include <cstdio>  // For FILENAME_MAX
#include <cstdlib> // For std::getenv
#include <filesystem>
#include <mutex>
#include <string>

#if defined(_WIN32) || defined(_WIN64)
#include <shlobj.h> // For SHGetFolderPathA
#include "fornani/sys/platform/win32/Windows.hpp"
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/sysctl.h>
#include <sys/types.h>
#else
#error "Unsupported platform."
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <pwd.h>
#endif

namespace fornani::io {

namespace {
// A helper function to safely retrieve environment variables.
std::string thread_safe_getenv(char const* var) {
	static std::mutex env_mutex;
	std::lock_guard lock(env_mutex);
	char const* value = std::getenv(var);
	return value ? std::string(value) : std::string();
}
} // namespace

auto abs_exe_path() -> std::filesystem::path {
#if defined(_WIN32) || defined(_WIN64)
	std::array<wchar_t, FILENAME_MAX> path = {};
	if (GetModuleFileNameW(nullptr, path.data(), FILENAME_MAX) == 0) {
		return {}; // Failed to get executable path
	}
	try {
		return canonical(std::filesystem::path(path.data()));
	} catch (...) { return std::filesystem::path(path.data()); }

#elif defined(__APPLE__)
	std::vector<char> path(FILENAME_MAX);
	uint32_t size = static_cast<uint32_t>(path.size());
	if (_NSGetExecutablePath(path.data(), &size) != 0) {
		path.resize(size); // Resize buffer and retry
		if (_NSGetExecutablePath(path.data(), &size) != 0) {
			return {}; // Executable path is too long or cannot be obtained
		}
	}
	try {
		return std::filesystem::canonical(std::filesystem::path(path.data()));
	} catch (...) { return std::filesystem::path(path.data()); }

#elif defined(__linux__)
	std::array<char, FILENAME_MAX> path;
	ssize_t count = readlink("/proc/self/exe", path.data(), FILENAME_MAX);
	if (count == -1) {
		return {}; // Failed to get executable path
	}
	try {
		return std::filesystem::canonical(std::filesystem::path(std::string(path.data(), count)));
	} catch (...) { return std::filesystem::path(std::string(path.data(), count)); }

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	std::array<char, FILENAME_MAX> path;
	size_t size = path.size();
	int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};

	if (sysctl(mib, 4, path.data(), &size, nullptr, 0) == -1) {
		return {}; // Failed to get executable path on BSD
	}
	// Ensure null termination if there's room.
	if (size < path.size()) { path[size] = '\0'; }
	try {
		return std::filesystem::canonical(std::filesystem::path(path.data()));
	} catch (...) { return std::filesystem::path(path.data()); }
#else
	return {};
#endif
}

auto abs_exe_directory() -> std::filesystem::path {
	std::filesystem::path const exe_path = abs_exe_path();
	if (exe_path.empty()) { return {}; }
	return exe_path.parent_path();
}

auto get_resource_directory() -> std::filesystem::path {
#ifndef NANI_PRODUCTION
	namespace fs = std::filesystem;
	// Start at the directory containing the executable.
	fs::path currentDir = abs_exe_directory();
	if (currentDir.empty()) { return {}; }

	// Move up the directory tree until we reach the root.
	while (!currentDir.empty()) {
		fs::path dependenciesDir = currentDir / "dependencies";
		fs::path gameDir = currentDir / "game";
		fs::path launcherDir = currentDir / "launcher";

		if (fs::path resourcesDir = currentDir / "resources"; is_directory(dependenciesDir) && is_directory(gameDir) && is_directory(launcherDir) && is_directory(resourcesDir)) {
			try {
				return canonical(resourcesDir);
			} catch (...) { return resourcesDir; }
		}

		fs::path parent = currentDir.parent_path();
		if (parent == currentDir) { // Reached the filesystem root.
			break;
		}
		currentDir = parent;
	}
#endif
	// If not found, assume the resources directory is in the same directory as the executable.
	std::filesystem::path const exeDir = abs_exe_directory();
	if (exeDir.empty()) { return {}; }
	return exeDir / "resources";
}

/**
 * @brief Gets the current user's home directory.
 *
 * On Windows, this function first attempts to retrieve the home directory
 * using SHGetFolderPathA. If that fails, it falls back to the USERPROFILE
 * environment variable.
 *
 * On Unix-like systems (Linux, macOS, BSD), it first attempts to read the HOME
 * environment variable, then falls back to querying the password database.
 *
 * @return A std::filesystem::path representing the home directory, or an empty path if it cannot be determined.
 */
auto get_home_directory() -> std::filesystem::path {
#if defined(_WIN32) || defined(_WIN64)
	std::array<char, MAX_PATH> path = {};
	if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PROFILE, nullptr, 0, path.data()))) { return std::filesystem::path(path.data()); }
	if (std::string const userProfile = thread_safe_getenv("USERPROFILE"); !userProfile.empty()) { return std::filesystem::path(userProfile); }
	return {};

#else
	std::string home = thread_safe_getenv("HOME");
	if (!home.empty()) { return std::filesystem::path(home); }
	struct passwd pwd_storage;
	struct passwd* pwd = nullptr;
	std::vector<char> buf(1024);
	while (true) {
		int result = getpwuid_r(getuid(), &pwd_storage, buf.data(), buf.size(), &pwd);
		if (result == 0) break;
		if (result != ERANGE) return {};
		buf.resize(buf.size() * 2);
	}
	if (pwd != nullptr && pwd->pw_dir != nullptr) { return std::filesystem::path(pwd->pw_dir); }
	return {};
#endif
}

} // namespace fornani::io
