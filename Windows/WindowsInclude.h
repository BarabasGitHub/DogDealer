#pragma once
// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.
#define _WIN32_WINNT 0x0601
#include <winsdkver.h>
#include <SDKDDKVer.h>
#ifndef UNICODE
#define UNICODE
#endif

// We will use some defines to turn off a stack of windows APIs that we wont need
// this is not strictly neccessary but helps compilation speed.

// These two defines turn off some of the more rarely used APIs, we wont be needing them.
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

// In addition the below defines turn off specific APIs, lets just jettison the lot.
#define NOGDICAPMASKS //     - CC_*, LC_*, PC_*, CP_*, TC_*, RC_
//#define NOWINMESSAGES //     - WM_*, EM_*, LB_*, CB_*
//#define NOWINSTYLES //       - WS_*, CS_*, ES_*, LBS_*, SBS_*, CBS_*
//#define NOSYSMETRICS //      - SM_*
//#define NOMENUS //           - MF_*
//#define NOICONS //           - IDI_*
//#define NOKEYSTATES //       - MK_*
#define NOSYSCOMMANDS //     - SC_*
#define NORASTEROPS //       - Binary and Tertiary raster ops
//#define NOSHOWWINDOW //      - SW_*
#define OEMRESOURCE //       - OEM Resource values
#define NOATOM //            - Atom Manager routines
#define NOCLIPBOARD //       - Clipboard routines
//#define NOCOLOR //           - Screen colors
//#define NOCTLMGR //          - Control and Dialog routines
#define NODRAWTEXT //        - DrawText() and DT_*
#define NOGDI //             - All GDI defines and routines
#define NOKERNEL //          - All KERNEL defines and routines
#define NONLS //             - All NLS defines and routines
#define NOMEMMGR //          - GMEM_*, LMEM_*, GHND, LHND, associated routines
#define NOMETAFILE //        - typedef METAFILEPICT
#define NOMINMAX //          - Macros min(a,b) and max(a,b)
//#define NOMSG //             - typedef MSG and associated routines
//#define NOOPENFILE //        - OpenFile(), OemToAnsi, AnsiToOem, and OF_*
#define NOSCROLL //          - SB_* and scrolling routines
#define NOSERVICE //         - All Service Controller routines, SERVICE_ equates, etc.
//#define NOSOUND //           - Sound driver routines
#define NOTEXTMETRIC //      - typedef TEXTMETRIC and associated routines
#define NOWH //              - SetWindowsHook and WH_*
//#define NOWINOFFSETS //      - GWL_*, GCL_*, associated routines
#define NOCOMM //            - COMM driver routines
#define NOKANJI //           - Kanji support stuff.
#define NOHELP //            - Help engine interface.
#define NOPROFILER //        - Profiler interface.
#define NODEFERWINDOWPOS //  - DeferWindowPos routines
#define NOMCX //             - Modem Configuration Extensions
//
// we use GetAsyncKeyState & VK for input
// #define NOUSER //            - All USER defines and routines
// #define NOVIRTUALKEYCODES // - VK_*
// #define NOMB //              - MB_* and MessageBox()

#include <Windows.h>

#include "WindowsErrorsToException.h"