#ifndef TSR_PROFILING
#define TSR_PROFILING
#include "tsr_platform.h"
#include "tsr_types.h"

struct TimeData
{
	i64 baseTime{ 0 };
	i64 currTime{ 0 };
	i64 prevTime{ 0 };
	i64 countsPerSec{ 0 };
	r64 secondsPerCount{ 0.0 };
	r64 deltaTime{ 0.0 };
	r64 totalTime{ 0.0 };
};

struct FrameStats
{
	r64 fps;
	r64 ms_per_frame;

	r64 avgfps;
	r64 avgmspf;
};

void CalculateFrameStats(TimeData& tData, FrameStats* fStats);
void UpdateTimeInformation(TimeData* tData);
void ResetTimeInformation(TimeData* tData);

#endif