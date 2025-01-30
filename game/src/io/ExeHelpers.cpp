#include "fornani/io/ExeHelpers.hpp"

#include <filesystem>
#include <array>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
	#include "fornani/core/platform/win32/Windows.hpp"
#elif defined(__APPLE__)
	#include <mach-o/dyld.h>
#elif defined(__linux__)
	#include <string>
	#include <unistd.h>
#else
	#error "Unsupported platform."
#endif

namespace fornani::io {

auto abs_exe_path() -> std::filesystem::path
{
#if defined(_WIN32) || defined(_WIN64)
	std::array<wchar_t, FILENAME_MAX> path = { 0 };
	if (GetModuleFileNameW(nullptr, path.data(), FILENAME_MAX) == 0) {
		throw std::runtime_error("Failed to get executable path on Windows.");
	}
	return {path.data()};

#elif defined(__APPLE__)
	std::array<char, FILENAME_MAX> path;
	uint32_t size = static_cast<uint32_t>(path.size());
	if (_NSGetExecutablePath(path.data(), &size) != 0) {
		throw std::runtime_error("Executable path is too long on macOS.");
	}
	return std::filesystem::canonical(std::filesystem::path(path.data()));

#elif defined(__linux__)
	std::array<char, FILENAME_MAX> path;
	ssize_t count = readlink("/proc/self/exe", path.data(), FILENAME_MAX);
	if (count == -1) {
		throw std::runtime_error("Failed to get executable path on Linux.");
	}
	return std::filesystem::canonical(std::filesystem::path(std::string(path.data(), count)));

#else
	throw std::runtime_error("Unsupported platform.");
#endif
}

auto abs_exe_directory() -> std::filesystem::path
{
	return abs_exe_path().parent_path();
}

} // namespace imp::core::platform
