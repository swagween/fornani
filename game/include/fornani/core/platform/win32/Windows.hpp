
#pragma once

/*
 * Wrapper for windows.h that removes all the garbage
 */

// clang-format off

/// STEP ONE: Get ready for windows.h by saving and undefining some macros
#if defined(_WIN32) || defined(WIN32)

// Save these macros for later; Windows redefines them
#pragma push_macro("MAX_uint8")
#pragma push_macro("MAX_uint16")
#pragma push_macro("MAX_uint32")
#pragma push_macro("MAX_int32")
#pragma push_macro("TEXT")
#pragma push_macro("TRUE")
#pragma push_macro("FALSE")

// Undefine the TEXT macro for winnt.h to redefine it, unless it's already been included
#ifndef _WINNT_
#undef TEXT
#endif



/// STEP TWO: Include windows.h

#if defined(_WINDOWS_) && !defined(FORNANI_MINIMAL_WINDOWS_INCLUDE)
    GEN_MESSAGE ( " " )
    GEN_MESSAGE ( "You have included windows.h before MinWindows.h" )
    GEN_MESSAGE ( "All useless stuff from the windows headers won't be excluded !!!" )
    GEN_MESSAGE ( " " )
#endif // _WINDOWS_

#define FORNANI_MINIMAL_WINDOWS_INCLUDE

// WIN32_LEAN_AND_MEAN excludes rarely used services from windows headers.
#define WIN32_LEAN_AND_MEAN

// The below excludes some other unused services from the windows headers -- see windows.h for details.
#define NOGDICAPMASKS            // CC_*, LC_*, PC_*, CP_*, TC_*, RC_
#define NOVIRTUALKEYCODES        // VK_*
#define NOWINMESSAGES            // WM_*, EM_*, LB_*, CB_*
#define NOWINSTYLES              // WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
#define NOSYSMETRICS             // SM_*
#define NOMENUS                  // MF_*
#define NOICONS                  // IDI_*
#define NOKEYSTATES              // MK_*
#define NOSYSCOMMANDS            // SC_*
#define NORASTEROPS              // Binary and Tertiary raster ops
#define NOSHOWWINDOW             // SW_*
#define OEMRESOURCE              // OEM Resource values
#define NOATOM                   // Atom Manager routines
#define NOCLIPBOARD              // Clipboard routines
#define NOCOLOR                  // Screen colors
#define NOCTLMGR                 // Control and Dialog routines
#define NODRAWTEXT               // DrawText() and DT_*
#define NOGDI                    // All GDI #defines and routines
#define NOKERNEL                 // All KERNEL #defines and routines
#define NOUSER                   // All USER #defines and routines
//#define NONLS                  // All NLS #defines and routines // required by crt.h
#define NOMB                     // MB_* and MessageBox()
#define NOMEMMGR                 // GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE               // typedef METAFILEPICT
#ifndef NOMINMAX
#define NOMINMAX                 // Macros min(a,b) and max(a,b)
#endif
#define NOMSG                    // typedef MSG and associated routines
#define NOOPENFILE               // OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL                 // SB_* and scrolling routines
#define NOSERVICE                // All Service Controller routines, SERVICE_ equates, etc.
#define NOSOUND                  // Sound driver routines
#define NOTEXTMETRIC             // typedef TEXTMETRIC and associated routines
#define NOWH                     // SetWindowsHook and WH_*
#define NOWINOFFSETS             // GWL_*, GCL_*, associated routines
#define NOCOMM                   // COMM driver routines
#define NOKANJI                  // Kanji support stuff.
#define NOHELP                   // Help engine interface.
#define NOPROFILER               // Profiler interface.
#define NODEFERWINDOWPOS         // DeferWindowPos routines
#define NOMCX                    // Modem Configuration Extensions
#define NOCRYPT                  // Encryption
#define NOTAPE
#define NOIMAGE
#define NOPROXYSTUB
#define NORPC

// https://learn.microsoft.com/en-us/windows/win32/api/unknwn/nn-unknwn-iunknown
struct IUnknown;

// Finally, now we can include windows.h
#include <Windows.h>


// HACK: Define macros that some windows.h variants don't
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef WM_DWMCOMPOSITIONCHANGED
#define WM_DWMCOMPOSITIONCHANGED 0x031E
#endif
#ifndef WM_DWMCOLORIZATIONCOLORCHANGED
#define WM_DWMCOLORIZATIONCOLORCHANGED 0x0320
#endif
#ifndef WM_COPYGLOBALDATA
#define WM_COPYGLOBALDATA 0x0049
#endif
#ifndef WM_UNICHAR
#define WM_UNICHAR 0x0109
#endif
#ifndef UNICODE_NOCHAR
#define UNICODE_NOCHAR 0xFFFF
#endif
#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0
#endif
#ifndef GET_XBUTTON_WPARAM
#define GET_XBUTTON_WPARAM(w) (HIWORD(w))
#endif
#ifndef EDS_ROTATEDMODE
#define EDS_ROTATEDMODE 0x00000004
#endif
#ifndef DISPLAY_DEVICE_ACTIVE
#define DISPLAY_DEVICE_ACTIVE 0x00000001
#endif
#ifndef _WIN32_WINNT_WINBLUE
#define _WIN32_WINNT_WINBLUE 0x0603
#endif
#ifndef _WIN32_WINNT_WIN8
#define _WIN32_WINNT_WIN8 0x0602
#endif
#ifndef WM_GETDPISCALEDSIZE
#define WM_GETDPISCALEDSIZE 0x02e4
#endif
#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI 96
#endif
#ifndef OCR_HAND
#define OCR_HAND 32649
#endif



/// STEP THREE: Clean up windows.h

// Hide Windows-only types (same as HideWindowsPlatformTypes.h)
#undef INT
#undef UINT
#undef DWORD
#undef FLOAT

// Undo any Windows defines.
#undef uint8
#undef uint16
#undef uint32
#undef int32
#undef float
#undef CDECL
#undef PF_MAX
#undef CaptureStackBackTrace
#undef CopyFile
#undef CreateDesktop
#undef CreateDirectory
#undef CreateFont
#undef DeleteFile
#undef DrawText
#undef FindWindow
#undef GetClassInfo
#undef GetClassName
#undef GetCommandLine
#undef GetCurrentTime
#undef GetEnvironmentVariable
#undef GetFileAttributes
#undef GetFreeSpace
#undef GetMessage
#undef GetNextSibling
#undef GetObject
#undef GetProp
#undef GetTempFileName
#undef IMediaEventSink
#undef IsMaximized
#undef IsMinimized
#undef LoadString
#undef MemoryBarrier
#undef MoveFile
#undef PlaySound
#undef PostMessage
#undef ReportEvent
#undef SendMessage
#undef SetPort
#undef SetProp
#undef UpdateResource
#undef Yield

// Undefine all the atomics. AllowWindowsPlatformAtomics/HideWindowsPlatformAtomics temporarily defining these macros.
#undef InterlockedIncrement
#undef InterlockedDecrement
#undef InterlockedAdd
#undef InterlockedExchange
#undef InterlockedExchangeAdd
#undef InterlockedCompareExchange
#undef InterlockedCompareExchangePointer
#undef InterlockedExchange64
#undef InterlockedExchangeAdd64
#undef InterlockedCompareExchange64
#undef InterlockedIncrement64
#undef InterlockedDecrement64
#undef InterlockedAnd
#undef InterlockedOr
#undef InterlockedXor

// Restore any previously defined macros
#pragma pop_macro("MAX_uint8")
#pragma pop_macro("MAX_uint16")
#pragma pop_macro("MAX_uint32")
#pragma pop_macro("MAX_int32")
#pragma pop_macro("TEXT")
#pragma pop_macro("TRUE")
#pragma pop_macro("FALSE")


// Make sure the version is high enough for API to be defined. For CRITICAL_SECTION
#if !defined(_XTL_) && (_WIN32_WINNT < 0x0403)
#error SetCriticalSectionSpinCount requires _WIN32_WINNT >= 0x0403
#endif



/// STEP FOUR: Perform some minor changes to the Windows API

// In RELEASE disable OutputDebugString
    #if defined(FORNANI_PRODUCTION) && !defined(FORNANI_PRODUCTION_LOGGING)
        #undef OutputDebugString
        #define OutputDebugString(...) (void) 0
    #endif

#endif // defined(_WIN32) || defined(WIN32)
