#ifndef TSR_PLATFORM
#define TSR_PLATFORM

#if (defined(_WIN64) && _WIN64)
//TODO(Fran): defines to prevent windows etc
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#endif

