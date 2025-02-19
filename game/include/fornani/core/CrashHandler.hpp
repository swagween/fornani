#pragma once

#include <functional>
#include <string>
#include <vector>

namespace fornani::core {

// TODO: This needs a lot of work, but it's a start.

/**
 * @brief Structure holding crash details.
 *
 * In production mode (when NANI_PRODUCTION is defined) only the crash code
 * and description are available.
 */
// CrashInfo.h (platform independent)
struct CrashInfo {
	int code;				  // Signal number (POSIX) or exception code (Windows)
	std::string description;  // Human–readable description of the crash
	std::string extraInfo;	  // Additional details (e.g. access violation specifics)
	std::string crashContext; // Extra context (e.g. CPU registers, thread id, etc.)
#if !defined(NANI_PRODUCTION)
	std::string stackTrace; // Captured stack trace (only available in non-production builds)
#endif
};

/// Callback type for handling a crash event.
using CrashCallback = std::function<void(CrashInfo const& info)>;

/**
 * @brief CrashHandler provides a cross–platform crash handling facility.
 *
 * In production builds (with NANI_PRODUCTION defined) the handler outputs only
 * minimal crash info. In non–production builds it captures a full stack trace and
 * invokes any registered callbacks.
 */
class CrashHandler {
  public:
	/// Installs the OS–specific crash handlers.
	static void initialize();

	/**
	 * @brief Registers a callback to be invoked on a crash.
	 *
	 * In production mode this function is a no–op.
	 *
	 * @param callback A function taking a CrashInfo parameter.
	 */
	static void registerCallback(CrashCallback const& callback);

	/// Invokes all registered callbacks.
	///	Does nothing in production builds.
	static void invokeCallbacks(CrashInfo const& info);

  private:
#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
	/// Returns the container holding the callbacks.
	static std::vector<CrashCallback>& getCallbacks();
#endif

	/// Installs the OS–specific signal/exception handlers.
	static void setupPlatformHandlers();
};

} // namespace fornani::core
