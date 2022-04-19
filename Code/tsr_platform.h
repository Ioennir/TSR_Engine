#ifndef TSR_PLATFORM
#define TSR_PLATFORM

void FetchPerformanceCounter(long long* performanceCounter);
void FetchPerformanceFreq(long long* performanceFreq);

#if (defined(_WIN64) && _WIN64)
// target Windows 7 or later
#define _WIN32_WINNT 0x0601
#include <sdkddkver.h>

#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
#define NOMINMAX
#define STRICT

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Imgui bindings
#include "imgui/imgui_impl_win32.h"

HWND CreateAndSpawnWindow(LPCWSTR winName, RECT wRect, HINSTANCE hInstance, int nCmdShow);

#endif

#endif

