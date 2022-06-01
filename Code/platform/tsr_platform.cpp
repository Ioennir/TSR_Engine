
// These are supossed to be functions that are present on each platform
void FetchPerformanceCounter(long long* performanceCounter);
void FetchPerformanceFreq(long long* performanceFreq);

// TODO(Fran): HWND is a windows only type.
struct WindowData
{
	void* handle;//HWND handle;
	ui32 width;
	ui32 height;
};

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
// NOTE(Fran): Imgui only has win32 bindings, it shouldnt be a problem
#include "../imgui/imgui_impl_win32.h"

// WIN64 Platform implementation
#include "tsr_win32.cpp"

#endif

namespace Platform
{
	WindowData windowData{};
}