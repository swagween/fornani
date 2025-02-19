#include "fornani/core/CrashHandler.hpp"

#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

#include "fornani/io/Logger.hpp"

#if defined(_WIN32)
#include "fornani/core/platform/win32/Windows.hpp"
#endif

#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
// Include cpptrace only in non-production builds.
#if defined(_WIN32)
#include <dbghelp.h>
#endif
#include <cpptrace/cpptrace.hpp>

#endif

namespace fornani::core {

#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
namespace {
// Container for registered callbacks (only used in non-production builds).
std::vector<CrashCallback> g_callbacks; // NOLINT
} // namespace
#endif

#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
namespace {
std::string getStackTrace() {
	// Generate a resolved stack trace
	return cpptrace::generate_trace().to_string(false);
}
} // namespace
#endif

inline io::Logger const crash_logger{"CrashHandler"};

//
// POSIX (Linux/macOS) signal handler
//
#if defined(__APPLE__) || defined(__linux__)
void signalHandler(int signum, siginfo_t* /*info*/, void* /*context*/) {
	CrashInfo crashInfo;
	crashInfo.code = signum;
	crashInfo.description = std::string(strsignal(signum));
#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
	crashInfo.stackTrace = getStackTrace();
	NANI_LOG_ERROR(crash_logger,
				   "CrashHandler caught signal {} ({})\n"
				   "Stack Trace:\n {}",
				   signum, crashInfo.description, crashInfo.stackTrace);
	CrashHandler::invokeCallbacks(crashInfo);
#else
	NANI_LOG_ERROR(crash_logger, "Crash: {} ({})", signum, crashInfo.description);
#endif
	// Restore the default handler and re-raise the signal so that the process terminates.
	signal(signum, SIG_DFL);
	raise(signum);
}
#endif // __APPLE__ || __linux__

//
// Windows exception handler
//
#if defined(_WIN32)
namespace {
std::string getExceptionTypeDescription(DWORD code) {
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION: return "Access Violation: Invalid memory access.";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "Array Bounds Exceeded: Array index out of range.";
	case EXCEPTION_BREAKPOINT: return "Breakpoint: Debug breakpoint encountered.";
	case EXCEPTION_DATATYPE_MISALIGNMENT: return "Data Misalignment: Misaligned memory access.";
	case EXCEPTION_FLT_DENORMAL_OPERAND: return "Floating-Point Denormal Operand: Operand is denormalized.";
	case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "Floating-Point Divide By Zero: Division by zero in floating-point arithmetic.";
	case EXCEPTION_FLT_INEXACT_RESULT: return "Floating-Point Inexact Result: Inexact floating-point calculation.";
	case EXCEPTION_FLT_INVALID_OPERATION: return "Floating-Point Invalid Operation: Invalid floating-point operation.";
	case EXCEPTION_FLT_OVERFLOW: return "Floating-Point Overflow: Floating-point value overflow.";
	case EXCEPTION_FLT_STACK_CHECK: return "Floating-Point Stack Check: Floating-point stack error.";
	case EXCEPTION_FLT_UNDERFLOW: return "Floating-Point Underflow: Floating-point value underflow.";
	case EXCEPTION_ILLEGAL_INSTRUCTION: return "Illegal Instruction: Invalid or undefined CPU instruction.";
	case EXCEPTION_IN_PAGE_ERROR: return "In-Page Error: Page fault during memory access.";
	case EXCEPTION_INT_DIVIDE_BY_ZERO: return "Integer Divide By Zero: Division by zero in integer arithmetic.";
	case EXCEPTION_INT_OVERFLOW: return "Integer Overflow: Arithmetic overflow in integer operation.";
	case EXCEPTION_INVALID_DISPOSITION: return "Invalid Disposition: Exception handling error.";
	case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "Non-continuable Exception: Exception cannot be resumed.";
	case EXCEPTION_PRIV_INSTRUCTION: return "Privileged Instruction: Restricted instruction execution attempted.";
	case EXCEPTION_SINGLE_STEP: return "Single Step: Single-step (trace) event occurred.";
	case EXCEPTION_STACK_OVERFLOW: return "Stack Overflow: Thread stack capacity exceeded.";
#ifdef EXCEPTION_GUARD_PAGE
	case EXCEPTION_GUARD_PAGE: return "Guard Page Violation: Memory access triggered a guard page.";
#endif
#ifdef EXCEPTION_FLT_MULTIPLE_TRAPS
	case EXCEPTION_FLT_MULTIPLE_TRAPS: return "Floating-Point Multiple Traps: Multiple floating-point exceptions occurred.";
#endif
	default: return "Unknown Exception: Unrecognized exception code.";
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
LONG WINAPI windowsExceptionHandler(EXCEPTION_POINTERS* pExceptionInfo) {
	CrashInfo crashInfo;
	crashInfo.code = static_cast<int>(pExceptionInfo->ExceptionRecord->ExceptionCode);

	// Build a primary description.
	std::string exceptionTypeDesc = getExceptionTypeDescription(pExceptionInfo->ExceptionRecord->ExceptionCode);
	crashInfo.description = std::format("Exception Code   : 0x{:08x}\n"
										"Meaning          : {}\n"
										"Fault Address    : {}\n"
										"Exception Flags  : 0x{:08x}\n",
										crashInfo.code, exceptionTypeDesc, pExceptionInfo->ExceptionRecord->ExceptionAddress, pExceptionInfo->ExceptionRecord->ExceptionFlags);

	// Append any exception parameters, if available.
	if (pExceptionInfo->ExceptionRecord->NumberParameters > 0) {
		crashInfo.extraInfo += "Exception Parameters:\n";
		for (ULONG i = 0; i < pExceptionInfo->ExceptionRecord->NumberParameters; ++i) { crashInfo.extraInfo += std::format("  [{}]: 0x{:08x}\n", i, pExceptionInfo->ExceptionRecord->ExceptionInformation[i]); }
	}

	// For access violations, populate extraInfo with additional details.
	if (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && pExceptionInfo->ExceptionRecord->NumberParameters >= 2) {
		DWORD violationType = pExceptionInfo->ExceptionRecord->ExceptionInformation[0];
		ULONG_PTR targetAddress = pExceptionInfo->ExceptionRecord->ExceptionInformation[1];
		std::string operation;
		switch (violationType) {
		case 0: operation = "read"; break;
		case 1: operation = "write"; break;
		case 8: operation = "execute"; break;
		default: operation = "unknown"; break;
		}
		crashInfo.extraInfo = std::format("Access Violation Details:\n"
										  "  Operation    : {}\n"
										  "  Target Addr  : 0x{:08x}\n",
										  operation, targetAddress);
	}

	// If a nested exception record exists, include its information.
	if (pExceptionInfo->ExceptionRecord->ExceptionRecord != nullptr) {
		crashInfo.extraInfo += "Nested Exception Record:\n";
		crashInfo.extraInfo +=
			std::format("  Nested Exception Code   : 0x{:08x}\n"
						"  Nested Fault Address    : {}\n"
						"  Nested Exception Flags  : 0x{:08x}\n",
						pExceptionInfo->ExceptionRecord->ExceptionRecord->ExceptionCode, pExceptionInfo->ExceptionRecord->ExceptionRecord->ExceptionAddress, pExceptionInfo->ExceptionRecord->ExceptionRecord->ExceptionFlags);
	}

	crashInfo.crashContext += std::format("Thread ID   : {}\nProcess ID  : {}\n", GetCurrentThreadId(), GetCurrentProcessId());

	// Optionally, capture register context information into crashContext.
#if defined(_M_X64) || defined(_M_IX86)
	if (pExceptionInfo->ContextRecord) {
#if defined(_M_X64)
		crashInfo.crashContext = std::format("Register Context (x64):\n"
											 "  RAX: 0x{:016x}\n"
											 "  RBX: 0x{:016x}\n"
											 "  RCX: 0x{:016x}\n"
											 "  RDX: 0x{:016x}\n"
											 "  RSI: 0x{:016x}\n"
											 "  RDI: 0x{:016x}\n"
											 "  RBP: 0x{:016x}\n"
											 "  RSP: 0x{:016x}\n"
											 "  RIP: 0x{:016x}\n",
											 pExceptionInfo->ContextRecord->Rax, pExceptionInfo->ContextRecord->Rbx, pExceptionInfo->ContextRecord->Rcx, pExceptionInfo->ContextRecord->Rdx, pExceptionInfo->ContextRecord->Rsi,
											 pExceptionInfo->ContextRecord->Rdi, pExceptionInfo->ContextRecord->Rbp, pExceptionInfo->ContextRecord->Rsp, pExceptionInfo->ContextRecord->Rip);
#elif defined(_M_IX86)
		crashInfo.crashContext = std::format("Register Context (x86):\n"
											 "  EAX: 0x{:08x}\n"
											 "  EBX: 0x{:08x}\n"
											 "  ECX: 0x{:08x}\n"
											 "  EDX: 0x{:08x}\n"
											 "  ESI: 0x{:08x}\n"
											 "  EDI: 0x{:08x}\n"
											 "  EBP: 0x{:08x}\n"
											 "  ESP: 0x{:08x}\n"
											 "  EIP: 0x{:08x}\n",
											 pExceptionInfo->ContextRecord->Eax, pExceptionInfo->ContextRecord->Ebx, pExceptionInfo->ContextRecord->Ecx, pExceptionInfo->ContextRecord->Edx, pExceptionInfo->ContextRecord->Esi,
											 pExceptionInfo->ContextRecord->Edi, pExceptionInfo->ContextRecord->Ebp, pExceptionInfo->ContextRecord->Esp, pExceptionInfo->ContextRecord->Eip);
#endif
	}
#endif

#if !defined(NANI_PRODUCTION)
	// Optionally collect a stack trace.
	crashInfo.stackTrace = getStackTrace();
	NANI_LOG_ERROR(crash_logger, "CrashHandler caught exception:\n{}\n\nExtra Info:\n{}\n\nCrash Context:\n{}\n\nStack Trace:\n{}", crashInfo.description, crashInfo.extraInfo, crashInfo.crashContext, crashInfo.stackTrace);
	CrashHandler::invokeCallbacks(crashInfo);
#else
	NANI_LOG_ERROR(crash_logger, "Crash: 0x{:08x}", crashInfo.code);
#endif

	// Return EXCEPTION_EXECUTE_HANDLER to let Windows terminate the process.
	return EXCEPTION_EXECUTE_HANDLER;
}
} // namespace
#endif // _WIN32

//
// CrashHandler method implementations
//
void CrashHandler::setupPlatformHandlers() {
#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
#if defined(__APPLE__) || defined(__linux__)
	struct sigaction action;
	std::memset(&action, 0, sizeof(action));
	action.sa_sigaction = signalHandler;
	action.sa_flags = SA_SIGINFO | SA_RESETHAND; // Reset to default after handling

	// Register handlers for common fatal signals.
	sigaction(SIGSEGV, &action, nullptr);
	sigaction(SIGABRT, &action, nullptr);
	sigaction(SIGFPE, &action, nullptr);
	sigaction(SIGILL, &action, nullptr);
#ifdef SIGBUS
	sigaction(SIGBUS, &action, nullptr);
#endif

#elif defined(_WIN32)
	SetUnhandledExceptionFilter(windowsExceptionHandler);
#endif
#endif
}

void CrashHandler::initialize() { setupPlatformHandlers(); }

void CrashHandler::registerCallback(CrashCallback const& callback) {
#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
	getCallbacks().push_back(callback);
#else
	// In production builds, callback registration is disabled.
	(void)callback;
#endif
}

void CrashHandler::invokeCallbacks(CrashInfo const& info) {
#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
	for (auto const& callback : getCallbacks()) { callback(info); }
#endif
}

#if !defined(NANI_PRODUCTION) || !defined(NANI_DISABLE_ADVANCED_CRASH_REPORTING)
std::vector<CrashCallback>& CrashHandler::getCallbacks() { return g_callbacks; }
#endif

} // namespace fornani::core
